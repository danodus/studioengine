-- StudioSlider.lua

require "Common"

---------------------------------

StudioSlider = {}
StudioSlider.__index = StudioSlider

function StudioSlider:layout(sender)
    local r = sender:bounds()
    r.size.width = r.size.width - 20
    r.origin.y = r.origin.y + 10
    r.size.height = r.size.height - 10
    self._slider:setFrame(r)
    local margin = r.size.height / 2
    r = makeRect(margin - 10, 0, r.size.width - 2 * margin + 20, 15)
    self._rulerView:setFrame(r)
    self._labelView:setFrame(makeRect(r.origin.x + r.size.width, r.origin.y, 20, 8))
end

function StudioSlider:drawRuler(sender)
    local dc = sender:drawContext()
    local r = sender:bounds()
    local deltaTick = (r.size.width - 20) / (#self._tickLabels - 1)
        
    dc:pushStates()
    dc:setStrokeColor(lightGrayColor)
    for i = 1, #self._tickLabels do
        local x = 10 + (i - 1) * deltaTick
        dc:drawLine(makePoint(x, 10), makePoint(x, 13))
        dc:drawCenteredText(makeRect(x - deltaTick / 2, 0, deltaTick, 8), self._tickLabels[i])
    end
    dc:popStates()
end

function StudioSlider:drawLabel(sender)
    local dc = sender:drawContext()
    local r = sender:bounds()
    dc:pushStates()
    dc:setStrokeColor(lightGrayColor)
    dc:drawCenteredText(r, self._title)
    dc:popStates()
end

function StudioSlider:dispose(sender)
    sender:removeAllSubviews()
    sender:setOwner(nil)
end

function StudioSlider.new(name, title, min, max, pos, tickLabels)
    local self = setmetatable({}, StudioSlider)
    self._view = View.new(name)
    self._view:setOwner(self)
    self._view:setLayoutFn(self.layout)
    self._view:setDisposeFn(self.dispose)

    self._title = title
    self._tickLabels = tickLabels

    self._slider = Slider.new("studioSliderSlider", min, max, pos)
    self._rulerView = View.new("studioSliderRulerView")
    self._rulerView:setOwner(self)
    self._rulerView:setDrawFn(self.drawRuler)
    self._labelView = View.new("studioSliderLabelView")
    self._labelView:setOwner(self)
    self._labelView:setDrawFn(self.drawLabel)

    self._view:addSubview(self._rulerView)
    self._view:addSubview(self._labelView)
    self._view:addSubview(self._slider)

    self._thumbImage = Image.new("SliderHThumb@2x.png")
    self._slider:setThumbImage(self._thumbImage)

    self._minRailImage = Image.new("SliderHRailLeft@2x.png")
    self._slider:setMinRailImage(self._minRailImage)

    self._middleRailImage = Image.new("SliderHRailCenter@2x.png")
    self._slider:setMiddleRailImage(self._middleRailImage)

    self._maxRailImage = Image.new("SliderHRailRight@2x.png")
    self._slider:setMaxRailImage(self._maxRailImage)

    return self
end

function StudioSlider:view()
    return self._view
end

function StudioSlider:slider()
    return self._slider
end

--------------
--[[
function layout(sender)
   local r = sender:bounds()
   r = centeredRectInRect(r, 230, 30)
   slider:view():setFrame(r)
end

local topView = getTopView()
slider = StudioSlider.new("testSlider", "dB", -1, 1, 0, {"-∞", "-36", "", "", "-27", "", "", "-18", "", "-12", "-9", "-6", "-3", " 0"})

topView:addSubview(slider:view())
topView:setLayoutFn(layout)
]]--
