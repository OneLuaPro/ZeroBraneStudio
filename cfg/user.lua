--[[--
  Use this file to specify **System** preferences.
  Review (cfg\user-sample.lua) or check [online documentation](http://studio.zerobrane.com/documentation.html) for details.
  https://github.com/pkulchenko/ZeroBraneStudio/blob/master/cfg/user-sample.lua
--]]--
-- Try to find lua interpreter, works on Windows 10
-- FIXME: This can be done better I guess...
local f=io.popen("where.exe lua.exe")
local s = f:read("*a")
if #s > 0 then
  -- remove trailing \n and set path.lua variable
  path.lua = s:sub(1,#s-1)
end
