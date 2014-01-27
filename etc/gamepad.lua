
local os_clock = os.clock



local keyrepeat_string
local keyrepeat_skip
local keyrepeat_func = function()
    if keyrepeat_skip > 0 then keyrepeat_skip = keyrepeat_skip - 1; return end
    KeyboardInput(keyrepeat_string)
end
local function KeyRepeat(str)
    KeyboardInput(str)
    if keyreapeat_timer then KillTimer(keyrepeat_timer) end
    keyrepeat_skip = 8
    keyrepeat_string = str
    CreateTimer( "KRTimer", 20, keyrepeat_func)
end
local function KeyRepeatStop()
    KillTimer("KRTimer")
end


-- L1 = 5
-- R2 = 6
-- Cross = 1
-- Circle = 2
-- Square = 3
-- Triangle = 4
-- Start = 8
-- Select = 8

local L2Handler
local R2Handler
local POVHandler
local RSHandler

local dummy = function() end
local JoyDownCallbacksDefault = setmetatable({
    [1] = function() KeyboardInput("(SPACE)") end,
    [2] = function() KeyRepeat("(RIGHT)") end,
    [3] = function() KeyRepeat("(LEFT)") end,
    [4] = function() KeyboardInput("(RETURN)") end,
    [6] = function() MouseInput("LEFTDOWN") end,
    [7] = function()
        if IsJoyButtonPressed(5) then
            Start("H:\\soft\\monitors_off.exe")   
        else
            KeyboardInput("(F8)")
        end
    end,
    [8] = function() KeyboardInput("(NEXT)") end,
    ["UP"] = function()
        if IsJoyButtonPressed(5) then
            KeyRepeat("(F6)")
        else
            KeyRepeat("(UP)")
        end
    end,
    ["DOWN"] = function()
        if IsJoyButtonPressed(5) then
            KeyRepeat("(F5)")
        else
            KeyRepeat("(DOWN)")
        end
    end,
    ["R2"] = function() MouseInput("RIGHTDOWN") end,
}, { __index = function() return dummy end })


local JoyUpCallbacksDefault = setmetatable({
    [2] = function() KeyRepeatStop() end,
    [3] = function() KeyRepeatStop() end,
    [6] = function() MouseInput("LEFTUP") end,
    ["UP"] = function() KeyRepeatStop() end,
    ["DOWN"] = function() KeyRepeatStop() end,
    ["R2"] = function() MouseInput("RIGHTUP") end,
}, { __index = function() return dummy end })

local JoyEmptyCallbacks = setmetatable({
}, { __index = function() return dummy end })


do
    local seqs = {}
    function AddSequence(...)
        local sequence = {...}
        local func = table.remove(sequence)
        local length = #sequence
        if length < 2 then return end
        sequence._step = 1
        sequence._func = func
        table.insert(seqs, sequence)
    end


    local lastTime = os_clock()
    function SequenceProcess(btn)
        local now = os_clock()
        local expired = now > lastTime + .7
        lastTime = now
        for i, sequence in pairs(seqs) do
            if expired then sequence._step = 1 end
            local step = sequence._step
            -- print(step, sequence[step], btn, sequence[step] == btn)
            if sequence[step] == btn then
                if step == #sequence then
                    sequence._func()
                    sequence._step = 1
                else
                    sequence._step = step + 1
                end
            else
                sequence._step = 1
            end
        end
    end
end

local JoyDownCallbacks = JoyDownCallbacksDefault
local JoyUpCallbacks = JoyUpCallbacksDefault


-- local override = setmetatable({
--     [4] = function() print("SUCESS") end,
-- }, { __index = JoyDownCallbacksDefault })


AddSequence(5, "RIGHT", "LEFT", 5, "RIGHT", "LEFT", 5, function()
    if R2Handler == R2HandlerDefault then
        R2Handler = R2HandlerVibration
        L2Handler = L2HandlerVibration
    else
        R2Handler = R2HandlerDefault
        L2Handler = L2HandlerDefault
    end
end)


AddSequence(5, "LEFT", "RIGHT", 5, "LEFT", "RIGHT", 5, function()
    if JoyDownCallbacks ~= JoyEmptyCallbacks then
        -- POVHandler = dummy
        L2Handler = dummy
        R2Handler = dummy
        RSHandler = dummy
        JoyDownCallbacks = JoyEmptyCallbacks
        JoyUpCallbacks = JoyEmptyCallbacks
    else
        -- POVHandler = POVHandlerDefault
        R2Handler = R2HandlerDefault
        L2Handler = L2HandlerDefault
        RSHandler = GamepadMouseMove
        JoyDownCallbacks = JoyDownCallbacksDefault
        JoyUpCallbacks = JoyUpCallbacksDefault
    end
end)

function OnJoyButtonDown(btn)
    JoyDownCallbacks[btn]()
end
function OnJoyButtonUp(btn)
    SequenceProcess(btn)
    JoyUpCallbacks[btn]()
end


RegisterEvent("JoyButtonUp", OnJoyButtonUp)
RegisterEvent("JoyButtonDown", OnJoyButtonDown)







do
    function right_trigger_as_button(downFunc, upFunc)
        local prevZ = 50
        local onR2 = downFunc
        local onR2release = upFunc
        return function(axisZ)
            if axisZ <= 30 and prevZ > 30 then
                onR2()
            end
            if prevZ <= 30 and axisZ > 30 then
                onR2release()
            end
            prevZ = axisZ
        end
    end
end


do
    function left_trigger_as_button(downFunc, upFunc)
        local prevZ = 50
        local onL2 = downFunc
        local onL2release = upFunc
        return function(axisZ)
            if axisZ >= 70 and prevZ < 70 then
                onL2()
            end
            if prevZ >= 70 and axisZ < 70 then
                onL2release()
            end
            prevZ = axisZ
        end
    end
end

L2HandlerDefault = left_trigger_as_button(function() OnJoyButtonDown("L2") end, function() OnJoyButtonUp("L2") end)
R2HandlerDefault = right_trigger_as_button(function() OnJoyButtonDown("R2") end, function() OnJoyButtonUp("R2") end)

do 
    local prevVibration = 0
    R2HandlerVibration = function(axisZ)
        local p = 0
        if axisZ < 49 then
            p = (50 - axisZ) * 2
            if p < 10 then p = 0 end
        end
        if prevVibration ~= p then
            SetGamepadVibration("LEFT", p)
            prevVibration = p
        end
    end
end

do 
    local prevVibration = 0
    L2HandlerVibration = function(axisZ)
        local p = 0
        if axisZ > 51 then
            p = (axisZ - 50) * 2
            if p < 10 then p = 0 end
        end
        if prevVibration ~= p then
            SetGamepadVibration("RIGHT", p)
            prevVibration = p
        end
    end
end


do
    local prev_pov = "CENTERED"
    function POVHandlerDefault(pov)
        if pov ~= prev_pov then
            if prev_pov ~= "CENTERED" then
                -- pov_callbacks[prev_pov][2]()
                OnJoyButtonUp(prev_pov)
            end
            if pov ~= "CENTERED" then
                -- pov_callbacks[pov][1]()
                OnJoyButtonDown(pov)
            end
        end
        prev_pov = pov
    end
end


do
    local threshold = 5
    local msens = 0.3
    function GamepadMouseMove(ar, au)
        local dx,dy = 0,0
        if au > 50+threshold  then
            dx = (au - 50+threshold)*msens
        end
        if au < 50-threshold  then
            dx = ( au - (50-threshold))*msens
        end
        if ar > 50+threshold  then
            dy = (ar - 50+threshold)*msens
        end
        if ar < 50-threshold  then
            dy = ( ar-(50-threshold))*msens
        end
        if dx == 0 and dy == 0 then
            return
        else
            MouseInput("MOVE", dx,dy)
        end
    end
end

L2Handler = L2HandlerDefault
R2Handler = R2HandlerDefault
POVHandler = POVHandlerDefault
RSHandler = GamepadMouseMove

RegisterEvent("JoyUpdate",function()
    local pov, ax,ay,az,ar,au = GetJoyPosInfo()
    L2Handler(az)
    R2Handler(az)
    POVHandler(pov)
    RSHandler(ar, au)
end)
