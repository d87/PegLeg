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


local L2Handler
local R2Handler
local RSHandler

local dummy = function() end
local JoyDownCallbacksDefault = setmetatable({
    ["A"] = function() KeyboardInput("(SPACE)") end,
    ["B"] = function() KeyRepeat("(RIGHT)") end,
    ["X"] = function() KeyRepeat("(LEFT)") end,
    ["Y"] = function() KeyboardInput("(RETURN)") end,
    ["R1"] = function() MouseInput("LEFTDOWN") end,
    ["BACK"] = function()
        if IsJoyButtonPressed(5) then
            Start("H:\\soft\\monitors_off.exe")   
        else
            KeyboardInput("(F8)")
        end
    end,
    ["START"] = function() KeyboardInput("(NEXT)") end,
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
    ["X"] = function() KeyRepeatStop() end,
    ["B"] = function() KeyRepeatStop() end,
    ["R1"] = function() MouseInput("LEFTUP") end,
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


AddSequence("L1", "RIGHT", "LEFT", "L1", "RIGHT", "LEFT", "L1", function()
    if R2Handler == R2HandlerDefault then
        R2Handler = VibrationHandler("LEFT")
        L2Handler = VibrationHandler("RIGHT")
    else
        R2Handler = R2HandlerDefault
        L2Handler = L2HandlerDefault
    end
end)


AddSequence("L1", "LEFT", "RIGHT", "L1", "LEFT", "RIGHT", "L1", function()
    if JoyDownCallbacks ~= JoyEmptyCallbacks then
        L2Handler = dummy
        R2Handler = dummy
        RSHandler = dummy
        JoyDownCallbacks = JoyEmptyCallbacks
        JoyUpCallbacks = JoyEmptyCallbacks
    else
        R2Handler = R2HandlerDefault
        L2Handler = L2HandlerDefault
        RSHandler = GamepadMouseMove
        JoyDownCallbacks = JoyDownCallbacksDefault
        JoyUpCallbacks = JoyUpCallbacksDefault
    end
end)

function OnJoyButtonDown(btn, btnID)
    JoyDownCallbacks[btn]()
end
function OnJoyButtonUp(btn, btnID)
    SequenceProcess(btn)
    JoyUpCallbacks[btn]()
end


RegisterEvent("JoyButtonUp", OnJoyButtonUp)
RegisterEvent("JoyButtonDown", OnJoyButtonDown)





function TriggerAsButton(threshold, downFunc, upFunc)
    local prev = 0
    return function(TriggerValue)
        if TriggerValue >= threshold and prev < threshold then
            downFunc()
        end
        if prev >= threshold and TriggerValue < threshold then
            upFunc()
        end
        prev = TriggerValue
    end
end

function VibrationHandler(motor)
    local prev = 0
    return function(v)
        if prev ~= v then
            SetGamepadVibration(motor, v)
            prev = v
        end
    end
end

function GamepadMouseMove(msens)
    local threshold = 5
    local math_abs = math.abs
    return function (X, Y)
        local dx,dy = 0,0
        if math_abs(X) > threshold then
            dx = X*msens
        end
        if math_abs(Y) > threshold then
            dy = -Y*msens
        end
        if dx == 0 and dy == 0 then
            return
        else
            MouseInput("MOVE", dx,dy)
        end
    end
end


L2HandlerDefault = TriggerAsButton(40, function() OnJoyButtonDown("L2") end, function() OnJoyButtonUp("L2") end)
R2HandlerDefault = TriggerAsButton(40, function() OnJoyButtonDown("R2") end, function() OnJoyButtonUp("R2") end)
RSHandlerDefault = GamepadMouseMove(0.3)

L2Handler = L2HandlerDefault
R2Handler = R2HandlerDefault
RSHandler = RSHandlerDefault

RegisterEvent("JoyUpdate",function()
    local LX,LY, RX,RY, LT,RT = GetJoyPosInfo()
    L2Handler(LT)
    R2Handler(RT)
    RSHandler(RX, RY)
end)
