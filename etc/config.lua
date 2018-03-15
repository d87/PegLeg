-- see API description in pegleg.init.lua
RegisterEvent("OnCreate",function ()
    print("PegLeg started.")
--~     console.Show()
--~     print ("Edit config.lua to change configuration and disable console on startup")
    ListWindows()
end)

local timeout = {}
timeout["ALTS"] = os.time()
RegisterHotKey("ALT","S",function ()
    if GetWindowTitle() == "Starcraft 2" then return end
    timeout["ALTS"] = os.time()+3
end)

local photoshop_hooks_enabled = false
local t_ps_opacity = { 97 }
local t_ps_flow = { 97 }
local ps_hudcp_clickblock_event
local ps_hudcp_clickblock_func = function(btn)
    if btn == 1 then return true end
end
local ps_hudcp_onkeyup_event
local ps_hudcp_onkeyup_func = function(key)
    if key == "F1" then
        UnregisterEvent(ps_hudcp_clickblock_event)
        DisableMouseHooks()
        MouseInput("RIGHTUP")
        KeyboardInput("<(ALT)<(SHIFT)")
        UnregisterEvent(ps_hudcp_onkeyup_event)
        ps_hudcp_onkeyup_event = nil
    end
end
local ps_brushsize_onkeyup_event
local ps_brushsize_onkeyup_func = function(key)
    if key == "F8" then
        MouseInput("RIGHTUP")
        KeyboardInput("<(ALT)")
        UnregisterEvent(ps_brushsize_onkeyup_event)
        ps_brushsize_onkeyup_event = nil
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
            local isFlow = IsPressed("SHIFT")
            local param = isFlow and t_ps_flow or t_ps_opacity
            if param[1] > 3 then param[1] = param[1] - 3 end
            local vStr = string.format("%02d",param[1])
            vStr:gsub(".", KeyboardInput);
            local fmt = isFlow and "Flow: %s       " or "Opacity: %s  "
            OSDTextLong(string.format(fmt,vStr), 800,0)
    end
    
    if key == "F3" then
            local isFlow = IsPressed("SHIFT")
            local param = isFlow and t_ps_flow or t_ps_opacity
            if param[1] <= 97 then param[1] = param[1] + 3 end
            local vStr = string.format("%02d",param[1])
            vStr:gsub(".", KeyboardInput);
            local fmt = isFlow and "Flow: %s       " or "Opacity: %s  "
            OSDTextLong(string.format(fmt,vStr), 800,0)
            --if ps_opacity <= 97 then ps_opacity = ps_opacity + 3 end
            --KeyboardInput(">(SHIFT)");
            --string.format("%02d",ps_opacity):gsub(".", KeyboardInput);
            --KeyboardInput("<(SHIFT)");
    end
    if key == "F1" then
        -- HUD Color picker toggle
        if ps_hudcp_onkeyup_event then return end
        KeyboardInput(">(SHIFT)>(ALT)");
        MouseInput("RIGHTDOWN");
        --This mouse block disables left click while we're holding color picker key.
        --Because Photoshop is switching to color sampler tool if you accidently left-click, and this happens quite often with a tablet.
        --Actually if we click color picker will disappear, seems it's using low level input polling or something.
        --But anyway it still serves the purpose of protecting from color sampler
        EnableMouseHooks()
        ps_hudcp_clickblock_event = RegisterEvent("MouseDown",ps_hudcp_clickblock_func);
        
        ps_hudcp_onkeyup_event = RegisterEvent("KeyUp",ps_hudcp_onkeyup_func)
        return true -- eat keypress
    end
    --if key == "F8" then
    --    -- ALT+RMB Brush size tool
    --    if ps_brushsize_onkeyup_event then return end
    --    KeyboardInput(">(ALT)");
    --    MouseInput("RIGHTDOWN");
    --    ps_brushsize_onkeyup_event = RegisterEvent("KeyUp",ps_brushsize_onkeyup_func)
    --    return true -- eat keypress
    --end
end

local painter_opacity = 10
local painter_hudcp_onkeyup_event
local painter_hudcp_onkeyup_func = function(key)
    if key == "F1" then
        UnregisterEvent(painter_hudcp_onkeyup_event)
        KeyboardInput(">(CTRL)>(ALT)1<(ALT)<(CTRL)");
        painter_hudcp_onkeyup_event = nil
    end
end
local function painter_hooks(key)
    --Toggle Temportal Color Palette (Painter 12)
    if key == "F1" then
        if painter_hudcp_onkeyup_event then return true end
        painter_hudcp_onkeyup_event = RegisterEvent("KeyUp",painter_hudcp_onkeyup_func)
        KeyboardInput(">(CTRL)>(ALT)1<(ALT)<(CTRL)");
        return true
    end    
    if key == "F2" then
            if painter_opacity > 1 then painter_opacity = painter_opacity - 1 end
            --string.format("%02d",math.fmod(painter_opacity,10)):gsub(".", KeyboardInput);
            KeyboardInput(string.format("%d",math.fmod(painter_opacity,10)))
    end
    if key == "F3" then
            if painter_opacity < 10 then painter_opacity = painter_opacity + 1 end
            KeyboardInput(string.format("%d",math.fmod(painter_opacity,10)))
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
        photoshop_hooks_enabled = not photoshop_hooks_enabled  -- also toggles painter functions
    end,
    ['F9'] = Reload,
    ['F10'] = function() SetAlwaysOnTop(not IsAlwaysOnTop()) end,
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
        local procname = GetWindowProcess()
        if procname == "Photoshop.exe" then
            return photoshop_hooks(key)
        end
        if procname == "Painter 12 x64.exe" then
            return painter_hooks(key)
        end
    end
end)


AddScript("gamepad.lua")


function echo()
    print("echo")
end
function MoveWin(num)
    return function()
        MoveWindowToDesktop(num)
    end
end

function After(ms, func)
    local id = "Timer"..math.random(20)
    CreateTimer(id, ms, function()
        func()
        KillTimer(id)
    end)
end

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


