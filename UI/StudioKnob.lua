-- StudioKnob.lua

require "Common"

TAU = math.pi * 2.0

function drawSpokes(dc, pt, innerRadius, outerRadius, minAngle, maxAngle, nbDivisions)
    dc:pushStates()
    dc:setStrokeColor(lightGrayColor)

    for angle=minAngle, maxAngle, (maxAngle - minAngle) / nbDivisions do
        local p1 = makePoint(pt.x + innerRadius * math.cos(angle), pt.y + innerRadius * math.sin(angle))
        local p2 = makePoint(pt.x + outerRadius * math.cos(angle), pt.y + outerRadius * math.sin(angle))
        dc:drawLine(p1, p2) 
    end
    dc:popStates()
end

function drawPan(dc, pt, radius, isRight)
    dc:pushStates()
    dc:setStrokeColor(lightGrayColor)
    dc:setStrokeWidth(1.5)
    dc:drawCircle(pt, radius)
    dc:popStates()
    dc:pushStates()
    dc:setFillColor(lightGrayColor)
    local startAngle = 1.0 / 4.0 * TAU
    if isRight then
        startAngle = -startAngle
    end
    dc:drawArc(pt, radius - 1, startAngle, 1.0 / 2.0 * TAU)
    dc:popStates()
end

---------------------------------

StandardStudioKnobType = 0
PanStudioKnobType = 1

StudioKnob = {}
StudioKnob.__index = StudioKnob

function StudioKnob:layout(sender)
    local r = sender:bounds()
    local r2 = centeredRectInRect(r, 30, 30)
    r2.origin.y = r2.origin.y + 1
    self._slider:setFrame(r2)
end

function StudioKnob:draw(sender)
    local dc = sender:drawContext()
    local r = sender:bounds()
    local r2 = centeredRectInRect(r, 30, 30)
    r2.origin.y = r2.origin.y + 1
    dc:pushStates()
    dc:setStrokeColor(lightGrayColor)
    --dc:drawRect(r)
    dc:drawCenteredText(makeRect(r2.origin.x, r2.origin.y - 10, r2.size.width, 15), self._title)
    local minAngle = -1.0 / 8.0 * TAU
    local maxAngle = minAngle + 3.0 / 4.0 * TAU
    local nbDivisions = 20
    drawSpokes(dc, makePoint(r2.origin.x + r2.size.width / 2, r2.origin.y + r2.size.height / 2), 16, 18, minAngle, maxAngle, nbDivisions)
    if self._type == PanStudioKnobType then
        drawPan(dc, makePoint(r2.origin.x - 3, r2.origin.y - 2), 3, false)
        drawPan(dc, makePoint(r2.origin.x + r2.size.width + 3, r2.origin.y - 2), 3, true)
    end
    dc:popStates()
end

function StudioKnob:dispose(sender)
    sender:removeAllSubviews()
    sender:setOwner(nil)
end

function StudioKnob.new(name, title, min, max, pos, type)
    local self = setmetatable({}, StudioKnob)
    self._view = View.new(name)
    self._view:setOwner(self)
    self._view:setDrawFn(self.draw)
    self._view:setLayoutFn(self.layout)
    self._view:setDisposeFn(self.dispose)

    self._title = title
    self._type = type

    self._slider = Slider.new("studioKnobSlider", min, max, pos)
    self._slider:setType(RadialSliderType)
    self._view:addSubview(self._slider)

    self._thumbImage = Image.new("SliderRThumb@2x.png")
    self._slider:setThumbImage(self._thumbImage)

    return self
end

function StudioKnob:view()
    return self._view
end

function StudioKnob:slider()
    return self._slider
end

--------------
--[[
function layout(sender)
   local r = sender:bounds()
   r = centeredRectInRect(r, 60, 55)
   knob:view():setFrame(r)
end

local topView = getTopView()
knob = StudioKnob.new("testKnob", "KNOB", -1, 1, 0, PanStudioKnobType)

topView:addSubview(knob:view())
topView:setLayoutFn(layout)
]]--
