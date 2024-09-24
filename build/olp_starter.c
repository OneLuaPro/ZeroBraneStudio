/* ZeroBrane Studio sources are released under the MIT License */

/* Copyright (c) 2011-2023 Paul Kulchenko (paul@kulchenko.com) */

/* Permission is hereby granted, free of charge, to any person obtaining a copy */
/* of this software and associated documentation files (the "Software"), to deal */
/* in the Software without restriction, including without limitation the rights */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell */
/* copies of the Software, and to permit persons to whom the Software is */
/* furnished to do so, subject to the following conditions: */

/* The above copyright notice and this permission notice shall be included in */
/* all copies or substantial portions of the Software. */

/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN */
/* THE SOFTWARE. */

// this is an extremly ugly quick and dirty hack...
// maybe it could be refactored to do some error catching and
// other things, but right now it does what it should...
// (providing a single exe file in our main directory without
// polluting it with all these dlls located in the /bin folder)

// Based on win32_starter.c, ported for Lua 5.4 and linked with static Lua lib

#ifdef __MINGW32__
#define _WIN32_WINNT 0x0502
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winbase.h>
#include <stdlib.h>
#include <stdio.h>
#include <shlwapi.h>
#include <pathcch.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

static int luafunc_mbox (lua_State *L)
{
  const char *title = (const char*)lua_tolstring(L,1,NULL);
  const char *msg = (const char*)lua_tolstring(L,2,NULL);
  MessageBox(NULL,msg,title,MB_OK|MB_ICONERROR);
  return 0;
}

static const char *luacode =
  "local msg = _ERRMSG; _ERRMSG = nil "
  "local arg = _ARG or {}; _ARG = nil "
  "xpcall("
  "function() "
  "(loadfile 'src/main.lua')(table.unpack(arg)) end,"
  "function(err) msg('Uncaught lua script exception',debug.traceback(err)) end)"
  ;

static const char *setOneLuaProPaths =
  "function setOneLuaProPaths(basePath,path,cpath) "
  "package.path=basePath..path "
  "package.cpath=basePath..cpath "
  "end "
  ;

PCHAR* CommandLineToArgv(PCHAR CmdLine,int* _argc)
{
  PCHAR* argv;
  PCHAR  _argv;
  size_t  len;
  ULONG   argc;
  CHAR   a;
  size_t   i, j;

  BOOLEAN  in_QM;
  BOOLEAN  in_TEXT;
  BOOLEAN  in_SPACE;

  len = strlen(CmdLine);
  i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

  argv = (PCHAR*)GlobalAlloc(GMEM_FIXED, i + (len+2)*sizeof(CHAR));

  _argv = (PCHAR)(((PUCHAR)argv)+i);

  argc = 0;
  argv[argc] = _argv;
  in_QM = 0;
  in_TEXT = 0;
  in_SPACE = 1;
  i = 0;
  j = 0;

  while( a = CmdLine[i] ) {
    if(in_QM) {
      if(a == '\"') {
        in_QM = 0;
      } else {
        _argv[j] = a;
        j++;
      }
    } else {
      switch(a) {
      case '\"':
	in_QM = 1;
	in_TEXT = 1;
	if(in_SPACE) {
	  argv[argc] = _argv+j;
	  argc++;
	}
	in_SPACE = 0;
	break;
      case ' ':
      case '\t':
      case '\n':
      case '\r':
	if(in_TEXT) {
	  _argv[j] = '\0';
	  j++;
	}
	in_TEXT = 0;
	in_SPACE = 1;
	break;
      default:
	in_TEXT = 1;
	if(in_SPACE) {
	  argv[argc] = _argv+j;
	  argc++;
	}
	_argv[j] = a;
	j++;
	in_SPACE = 0;
	break;
      }
    }
    i++;
  }
  _argv[j] = '\0';
  argv[argc] = NULL;

  (*_argc) = argc;
  return argv;
}

PCHAR WideCharToUTF8(LPCWSTR text) {
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);
  PCHAR buffer = (PCHAR)GlobalAlloc(GMEM_FIXED, size_needed);
  WideCharToMultiByte(CP_UTF8, 0, text, -1, buffer, size_needed, NULL, NULL);
  return buffer;
}

int WINAPI WinMain(HINSTANCE hInstance,  HINSTANCE hPrevInstance,  LPSTR lpCmdLine, int nCmdShow)
{
  int argc;
  char ** argv = CommandLineToArgv(WideCharToUTF8(GetCommandLineW()),&argc);

  WCHAR buffer[MAX_PATH];
  WCHAR buf2[MAX_PATH];
  LPTSTR path2 = (LPTSTR)GlobalAlloc(GMEM_FIXED, (MAX_PATH+1)*sizeof(WCHAR));

  LPWSTR path = (LPWSTR)GlobalAlloc(GMEM_FIXED, (MAX_PATH+1)*sizeof(WCHAR));

  // Enable console output although in /SUBSYSTEM:WINDOWS and not in /SUBSYSTEM:CONSOLE
  // https://speedyleion.github.io/c/c++/windows/2021/07/11/WinMain-and-stdout.html
  if(!GetStdHandle(STD_OUTPUT_HANDLE)){
    if(AttachConsole(ATTACH_PARENT_PROCESS)){
      freopen("CONOUT$","wb",stdout);
      freopen("CONOUT$","wb",stderr);
    }
  }

  if (GetCurrentDirectoryW(MAX_PATH, path) == 0) {
    MessageBox(NULL,
	       TEXT("Couldn't find the current working directory"),
	       TEXT("Failed to start editor"),
	       MB_OK|MB_ICONERROR);
    return 0;
  }

  if (!GetModuleFileNameW(NULL, buffer, MAX_PATH)) {
    MessageBox(NULL,
	       TEXT("Couldn't find the executable path"),
	       TEXT("Failed to start editor"),
	       MB_OK|MB_ICONERROR);
    return 0;
  }
  PathCchRemoveFileSpec(buffer,MAX_PATH);
  SetCurrentDirectoryW(buffer);
  // remove /opt/ZeroBraneStudio from path in buffer...
  PathCchRemoveFileSpec(buffer,MAX_PATH);
  PathCchRemoveFileSpec(buffer,MAX_PATH);
  // ... to yield OneLuaPro (variable) base install directory
  // printf("Base path in buffer = %ls\n",buffer);
  
  // OK, I don't do any error checking here, which COULD
  // lead to bugs that are hard to find, but considered the simplicity
  // of the whole process, it SHOULD be pretty unlikely to fail here
  // but don't come back on me if it does...
  lua_State *L = luaL_newstate();

  if (L!=NULL) {
    // First set new package.path and package.cpath according to OneLuaPro installation
    // https://www.codingwiththomas.com/blog/a-lua-c-api-cheat-sheet
    luaL_openlibs(L);
    if (luaL_dostring(L,setOneLuaProPaths) != 0)
      MessageBox(NULL,
		 TEXT("An unexpected error occured while running setOneLuaProPaths."),
		 TEXT("Failed to start editor"),
		 MB_OK|MB_ICONERROR);    
    lua_getglobal(L, "setOneLuaProPaths");  // Push the function name onto the stack
    // Push 1st arg base-dir on stack
    lua_pushstring(L, WideCharToUTF8(buffer));
    // Push 2nd arg path-extension on stack
    lua_pushstring(L, "\\share\\lua\\"LUA_VERSION_MAJOR"."LUA_VERSION_MINOR"\\?.lua");
    // Push 3rd arg cpath-extension on stack
    lua_pushstring(L, "\\lib\\lua\\"LUA_VERSION_MAJOR"."LUA_VERSION_MINOR"\\?.dll");
    // call lua function setOneLuaProPaths() with 3 input args and no outputs args
    lua_pcall(L, 3, 0, 0); 

    // (Almost) original code
    int i;
    lua_createtable(L,argc+2,0);
    lua_pushstring(L,argv[0]); // executable name goes first
    lua_rawseti(L,-2,1);
    lua_pushstring(L,"-cwd");
    lua_rawseti(L,-2,2);
    lua_pushstring(L,WideCharToUTF8(path));
    lua_rawseti(L,-2,3);
    for (i=1;i<argc;i++) {
      lua_pushstring(L,argv[i]);
      lua_rawseti(L,-2,i+3);
    }
    //lua_setfield(L,LUA_GLOBALSINDEX,"_ARG");	// Lua 5.1
    lua_setglobal(L,"_ARG");	// Lua 5.4
    // luaL_openlibs(L);
    lua_pushcclosure(L,luafunc_mbox,0);
    //lua_setfield(L,LUA_GLOBALSINDEX,"_ERRMSG");	// Lua 5.1
    lua_setglobal(L,"_ERRMSG");	// Lua 5.4
    if (luaL_loadbuffer(L,luacode,strlen(luacode),"Initializer") == 0)
      lua_pcall(L,0,0,0);
    else
      MessageBox(NULL,
		 TEXT("An unexpected error occured while loading the lua chunk."),
		 TEXT("Failed to start editor"),
		 MB_OK|MB_ICONERROR);
    lua_close(L);
  }
  else {
    MessageBox(NULL,
	       TEXT("Couldn't initialize a luastate"),
	       TEXT("Failed to start editor"),
	       MB_OK|MB_ICONERROR);
  }
  return 0;
}
