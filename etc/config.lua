-- see API description in pegleg.init.lua
RegisterEvent("OnCreate",function ()
    print("PegLeg started.")
--~     console.Show()
--~     print ("Edit config.lua to change configuration and disable console on startup")
end)

timeout["ALTS"] = os.time()
RegisterHotKey("ALT","S",function ()
    if GetWindowTitle() == "Starcraft II" then return end
    timeout["ALTS"] = os.time()+3
end)
-- Default Sequence: Alt+S -> <keys from actions table>
local actions = {
    ['3'] = function() sh("start S:\\!ftp\\") end,
    ['4'] = function() sh('start S:\\Games\\') end,
    ['5'] = function() sh('start H:\\_Starcraft2\\Replays\\') end,
    ['6'] = function() sh("start G:\\") end,
    ['7'] = function() sh("start D:\\") end,
    ['8'] = function() sh("start H:\\DUMP\\") end,
    ['9'] = function() sh("start C:\\") end,
    
    ['B'] = function() sh("H:\\DesktopTrash\\!notes.txt") end,
    ['E'] = function() Start('G:\\putty.exe -load "nevihta"') end,
    ['R'] = function() Start('G:\\putty.exe -load "webfaction"') end,
    ['F12'] = function() photoshop_hooks_enabled = not photoshop_hooks_enabled end,
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
            photoshop_hooks(key)
        end
    end
end)


local photoshop_hooks_enabled = false
local ps_opacity = 70
local function photoshop_hooks(key)
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
end



--~ local bot = false
--~ local time = 0
--~     if GetWindowTitle() == "XX" then
--~         if key == "F12" then
--~             --MouseInput("LEFTDOWN")            
--~             --KeyboardInput("(RETURN)")
--~             --MouseInput("LEFTUP")
--~             bot = not bot
--~         end
--~         return
--~     end

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
--~     print ("RAAA")
--~ end)


--~ RegisterEvent("KeyDown",KeyDown)
--~ RegisterEvent("KeyUp",KeyUp)


--- MOUSE HOOKS DISABLED ---
--~ RegisterEvent("MouseDown",MouseDown)
--~ RegisterEvent("MouseUp",MouseUp)
--~ RegisterEvent("MouseMove",mm)


