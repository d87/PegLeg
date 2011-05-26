--[[
PEGLEG FUNCTIONS LIST:
console.Show() - Show console
console.Hide() - Hide console
console.Clear() - Clear console
console.SetBackgroundColor(r,g,b) - Set console bg color. r,g,b - values between 0 and 1
console.SetColor(r,g,b) - Set console text color. r,g,b - values between 0 and 1

Shell ("cmdline" [,"workingdir"]) - execute string in shell
Start ("cmdline" [,"workingdir"]) - start new process

timerID = CreateTimer(interval, callback) - Create new timer. Timers repeatedly execute callback function after a specified interval, in milliseconds.
KillTimer(timerID)
activeWindowTitle = GetWindowTitle()
activeWindowProcessName = GetWindowProcess()
x,y = GetCursorPos()

Reload() - reload configuration files
Shutdown() - exit programm

AddScript(filename) - Execute Lua script

RegisterHotKey("modifiers","key",callback)
    modifiers:
        list of separated modifiers. Example: "SHIFT-CTRL-ALT"

        
UnregisterEvent(eventID)
eventID = RegisterEvent(event, callback)
    events:
        KeyDown
        KeyUp
        MouseDown
        MouseUp
        OnCreate
        
    if callback function returns true then keypress isn't passed for further processing.
    

MouseInput("type", x, y, absolute) - Emulate mouse input. absolute parameter used only for MOVE event
    type:
        MOVE
        LEFTUP
        LEFTDOWN
        RIGHTUP
        RIGHTDOWN
        MIDDLEDOWN
        MIDDLEUP
    x:
        Screen X-coordinate, if absolute is true, or pixels relative to current position.
    y:
        Screen Y-coordinate, if absolute is true, or pixels relative to current position.
        
KeyboardInput( input ) - Emulate keyboard input
    input:
        > prefix = keydown
        < prefix = keyup
        without prefix = keypress
        A-B, 1-9 - for corresponding keys
        (KEYNAME) - for other keys (full list in pegleg.conf.lua)
    Example:
        KeyboardInput(">(CTRL)C<(CTRL)") means down Ctrl, press C, release Ctrl
        
SetAlwaysOnTop( state )
    state:
        boolean - enable/disable always on top mode for current window
IsAlwaysOnTop()
    returns true if always on top flag is set for current window
    
EnableMouseHooks( state )
    state:
        boolean - enable/disable mouse hooks. if ommited defaults to true
DisableMouseHooks() - disable mouse hooks
    
]]

--~ MOD = {
--~     ["LALT"] = false,
--~     ["LCTRL"] = false,
--~     ["LSHIFT"] = false,
--~ }

console.SetBackgroundColor(0.6,0,0)
console.SetColor(1,1,1)

--~ RegisterEvent("OnCreate",function()
--~     print "PegLeg started."
--~     console.Show()
--~ end)

--~ RegisterEvent("KeyDown",function(key)
--~     if MOD[key] ~= nil then
--~         MOD[key] = true
--~     end
--~     log(string.format("%d KDOWN: %s",os.time(),key))
--~ end)
--~ RegisterEvent("KeyUp",function(key)
--~     if MOD[key] ~= nil then
--~         MOD[key] = false
--~     end
--~ end)

--~ function log(msg)
--~     io.output(io.open("log.log","a"))
--~     io.write(msg)
--~     io.close()
--~ end

AddScript("config.lua")