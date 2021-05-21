--
-- NewSequenceView.lua
--

require "Common"

if language() == "fr" then
cancelStr = "Annuler"
nameStr = "Nom:"
timeSigStr = "Ind. mesure:"
tempoStr = "Tempo:"
else
cancelStr = "Cancel"
nameStr = "Name:"
timeSigStr = "Time Signature:"
tempoStr = "Tempo:"
end

------------------------------------------------------------------------------------------------------------------------
function layout(sender)
local r = topView:bounds()
boxView:setFrame(r)

if operatingSystem() == "windows" then
leftButton = okButton
rightButton = cancelButton
else
leftButton = cancelButton
rightButton = okButton
end

r = makeRect(r.size.width - 20 - 100 - 12 - 100, 20, 100, 20)
leftButton:setFrame(r)

r = rightOfRectWithMargin(r, 100, 12)
rightButton:setFrame(r)

r = topView:bounds()
r = makeRect(20, r.size.height - 20 - 20, 100, 20)
nameLabelView:setFrame(r)

local r2 = rightOfRectWithMargin(r, 250, 8)
-- Adjustments for text field
local r3 = r2
r3.origin.y = r3.origin.y - 1
r3.size.height = 22
nameTextField:setFrame(r3)

r.origin.y = r.origin.y - r.size.height - 48
timeSigLabelView:setFrame(r)

r2.origin.y = r2.origin.y - r2.size.height - 48
timeSigNumSegmentedControl:setFrame(r2)

r.origin.y = r.origin.y - r.size.height - 8
r2.origin.y = r2.origin.y - r2.size.height - 8
timeSigDenumSegmentedControl:setFrame(r2)

r.origin.y = r.origin.y - r.size.height - 48
tempoLabelView:setFrame(r)

r2.origin.y = r2.origin.y - r2.size.height - 48
tempoSlider:setFrame(r2)

r2 = rightOfRectWithMargin(r2, 60, 8)
tempoValueLabelView:setFrame(r2)

end

------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------

topView = getTopView()

boxView = BoxView.new("boxView")
boxView:setCornerRadius(5)

okButton = Button.new("okButton", "OK")
cancelButton = Button.new("cancelButton", cancelStr)

nameLabelView = LabelView.new("nameLabelView", nameStr)
nameLabelView:setTextAlign(RightTextAlign);
nameTextField = TextField.new("nameTextField")
nameTextField:setBorderColor(systemButtonBorderColor)
nameTextField:setBorderSize(2)


timeSigLabelView = LabelView.new("timeSigLabelView", timeSigStr)
timeSigLabelView:setTextAlign(RightTextAlign);
timeSigNumSegmentedControl = SegmentedControl.new("timeSigNumSegmentedControl", {"2", "3", "4", "5", "7", "11"})
timeSigDenumSegmentedControl = SegmentedControl.new("timeSigDenumSegmentedControl", {"1", "2", "4", "8", "16", "32", "64"})

tempoLabelView = LabelView.new("tempoLabelView", tempoStr)
tempoLabelView:setTextAlign(RightTextAlign);
tempoSlider = Slider.new("tempoSlider", 30, 300, 120)
tempoValueLabelView = LabelView.new("tempoValueLabelView", "")

topView:addSubview(boxView)
topView:addSubview(okButton)
topView:addSubview(cancelButton)
topView:addSubview(nameLabelView)
topView:addSubview(nameTextField)
topView:addSubview(timeSigLabelView)
topView:addSubview(timeSigNumSegmentedControl)
topView:addSubview(timeSigDenumSegmentedControl)
topView:addSubview(tempoLabelView)
topView:addSubview(tempoSlider)
topView:addSubview(tempoValueLabelView)

topView:setLayoutFn(layout)

setContentSize(makeSize(480, 280))
