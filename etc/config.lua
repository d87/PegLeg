-- see API description in pegleg.init.lua
RegisterEvent("OnCreate",function ()
    print("PegLeg started.")
--~     console.Show()
--~     print ("Edit config.lua to change configuration and disable console on startup")
end)

local timeout = {}
timeout["ALTS"] = os.time()
RegisterHotKey("ALT","S",function ()
    if GetWindowTitle() == "Starcraft 2" then return end
    timeout["ALTS"] = os.time()+3
end)

local photoshop_hooks_enabled = false
local ps_opacity = 70
local ps_hudcp_onkeyup_event
local ps_hudcp_onkeyup_func = function(key)
    if key == "F1" then
        MouseInput("RIGHTUP")
        KeyboardInput("<(ALT)<(SHIFT)")
        UnregisterEvent(ps_hudcp_onkeyup_event)
        ps_hudcp_onkeyup_event = nil
    end
end
local function photoshop_hooks(key)
    -- Toggle color picker
    if key == "F4" then
        if string.find(GetWindowTitle(),"Color Picker") then
            KeyboardInput("(RETURN)")
        else
            --ox, oy = GetCursorPos()
            --MouseInput("MOVE", 18, 638, true)
            --MouseInput("LEFTDOWN")
            --MouseInput("LEFTUP")
            --MouseInput("MOVE", ox, oy, true)
            KeyboardInput("C") -- Keybind for color picker in CS5
        end
    end
    
    if key == "F2" then
            if ps_opacity > 3 then ps_opacity = ps_opacity - 3 end
            string.format("%02d",ps_opacity):gsub(".", KeyboardInput)
    end
    
    if key == "F3" then
            if ps_opacity <= 97 then ps_opacity = ps_opacity + 3 end
            string.format("%02d",ps_opacity):gsub(".", KeyboardInput)
    end
    if key == "F1" then
        -- HUD Color picker toggle
        if ps_hudcp_onkeyup_event then return end
        KeyboardInput(">(SHIFT)>(ALT)");
        MouseInput("RIGHTDOWN");
        ps_hudcp_onkeyup_event = RegisterEvent("KeyUp",ps_hudcp_onkeyup_func)
        return true -- eat keypress
    end
end

local actions = {
    ['3'] = function() sh[[start D:\]] end,
    ['4'] = function() sh[[start H:\games\]] end,
    ['5'] = function() sh[[start H:\games\_StarCraft2\Replays\]] end,
    ['6'] = function() sh[[start H:\soft]] end,
    ['7'] = function() sh[[start D:\Downloads\]] end,
    ['8'] = function() sh[[start N:\]] end,
    ['9'] = function() sh[[start C:\]] end,
    ['R'] = function() sh[[D:\_dump\]] end,
    ['B'] = function() sh[[D:\_dump\__DesktopTrash\_notes.txt]] end,
    ['E'] = function() Start[[H:\soft\putty.exe -load "nevihta"]] end,
    ['R'] = function() Start[[H:\soft\putty.exe -load "nevihta_auth"]] end,
    ['F12'] = function()
        photoshop_hooks_enabled = not photoshop_hooks_enabled 
    end,
    ['F9'] = Reload,
    ['F10'] = Shutdown,
--~     ['F11'] = console.Hide,
    ['F11'] = console.Show,
}
RegisterEvent("KeyDown",function (key,vk,scan)
    if timeout["ALTS"] and os.time() < timeout["ALTS"] then
        timeout["ALTS"] = nil
        if actions[key] then
            actions[key]()
            return true
        end
    end
    
    if photoshop_hooks_enabled then
        if GetWindowProcess() == "Photoshop.exe" then
            return photoshop_hooks(key)
        end
    end
end)

--~ local t1 = 0
--~ local t2 = 0
--~ RegisterHotKey("ALT","G",function ()
--~     t1 = CreateTimer(1000,function()
--~         print("T-1 "..t1)
--~     end)
--~     t2 = CreateTimer(2000,function()
--~         print("T-2 "..t2)
--~     end)
--~ end)

--~ RegisterHotKey("ALT","H",function ()
--~     KillTimer(t1)
--~ end)
--~ RegisterHotKey("ALT","J",function ()
--~     print("xt111")
--~     KillTimer(t2)
--~ end)

--~ RegisterHotKey("ALT","G",function ()
--~     print (GetWindowProcess())
--~ end)


