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
activeWindowProcessName = GetWindowProcess()  - Note that if you're running 32bit pegleg on 64bit version of windows, you can't retreive 64bit process names.
x,y = GetCursorPos()

Reload() - reload configuration files
Shutdown() - exit programm

AddScript(filename) - Load lua script

RegisterHotKey("modifiers","key",callback)
    modifiers:
        list of separated modifiers. Example: "SHIFT-CTRL-ALT"

RegisterEvent(event, callback)
    events:
        KeyDown
        KeyUp
        OnCreate
    
    if callback function returns true then keypress isn't passed for further processing.

MouseInput("type", x, y, absolute) - Emulate mouse input. absolute parameter used only for MOVE event
    type:
        "MOVE"
        "LEFTUP"
        "LEFTDOWN"
        "RIGHTUP"
        "RIGHTDOWN"
        "MIDDLEDOWN"
        "MIDDLEUP"
    x:
        screen X-coordinate if absolute is true or pixels relative to current position
    y:
        screen Y-coordinate if absolute is true or pixels relative to current position
        
KeyboardInput( input ) - Emulate keyboard input
    input:
        > prefix = keydown
        < prefix = keyup
        without prefix = keypress
        A-B, 1-9 - for corresponding keys
        (KEYNAME) - for other keys (full list in pegleg.conf.lua)
    Example:
        KeyboardInput(">(CTRL)C<(CTRL)") means down Ctrl, press C, release Ctrl
]]

MOD = {
["LALT"] = false,
["LCTRL"] = false,
["LSHIFT"] = false,
}

timeout = {}

console.SetBackgroundColor(0.8, 0.8, 0.8)

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
--~     if key == "LSHIFT" or key == "LALT" or key == "LCTRL" then
--~         MOD[key] = false
--~     end
--~ end)

function log(msg)
    io.output(io.open("log.log","a"))
    io.write(msg)
    io.close()
end

AddScript("config.lua")
-- AddScript("xbmc_binds.lua")