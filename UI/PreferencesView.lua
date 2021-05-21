--
-- PreferencesView.lua
--

require "Common"

if language() == "fr" then

midiInputStr = "Entrée MIDI:"
midiOutputStr = "Sortie MIDI:"
audioOutputStr = "Sortie audio:"
audioOutputLatencyStr = "Latence audio:"
midiDestStr = "Destination MIDI:"
midiDestInternalSynthStr = "Synthétiseur interne"
midiDestMIDIOutputStr = "Sortie MIDI"
nbVoicesStr = "Nombre de voix:"
autoStopStr = "Arrêt automatique de l'enregistrement"
autoStopPeriodStr = "Temps d'inactivité:"

else

midiInputStr = "MIDI Input:"
midiOutputStr = "MIDI Output:"
audioOutputStr = "Audio Output:"
midiDestStr = "MIDI Destination:"
midiDestInternalSynthStr = "Internal Synthesizer"
midiDestMIDIOutputStr = "MIDI Output"
nbVoicesStr = "Number of Voices:"
audioOutputLatencyStr = "Audio Output Latency:"
autoStopStr = "Stop Recording Automatically"
autoStopPeriodStr = "Inactivity Period:"

end

------------------------------------------------------------------------------------------------------------------------
function layout(sender)
local r = topView:bounds()
boxView:setFrame(r)

-- MIDI settings

r = makeRect(20, r.size.height - 20 - 20, 150, 20)
midiInputSelectionLabelView:setFrame(r)

local r2 = rightOfRectWithMargin(r, 300, 8)
midiInputComboBox:setFrame(r2)

r.origin.y = r.origin.y - r.size.height - 8
midiOutputSelectionLabelView:setFrame(r)

r2.origin.y = r2.origin.y - r2.size.height - 8
midiOutputComboBox:setFrame(r2)

-- Audio settings

r.origin.y = r.origin.y - r.size.height - 48
audioOutputSelectionLabelView:setFrame(r)

r2 = rightOfRectWithMargin(r, 300, 8)
audioOutputComboBox:setFrame(r2)

r.origin.y = r.origin.y - r.size.height - 8
audioOutputLatencyLabelView:setFrame(r)

r2 = rightOfRectWithMargin(r, 500, 8)
audioOutputLatencySlider:setFrame(r2)

r2 = rightOfRectWithMargin(r2, 50, 8)
audioOutputLatencyValueLabelView:setFrame(r2)

-- MIDI destination

r = belowRectWithMargin(r, 150, 20, 48)
midiDestLabelView:setFrame(r)

r2 = rightOfRectWithMargin(r, 150, 8)
midiDestInternalSynthButton:setFrame(r2)

r2 = rightOfRectWithMargin(r2, 150, 8)
midiDestMIDIOutputButton:setFrame(r2)

-- Number of voices

r = belowRectWithMargin(r, 150, 20, 48)
nbVoicesLabelView:setFrame(r)

r2 = rightOfRectWithMargin(r, 500, 8)
nbVoicesSlider:setFrame(r2)

r2 = rightOfRectWithMargin(r2, 50, 8)
nbVoicesValueLabelView:setFrame(r2)

-- Auto stop settings

r = belowRectWithMargin(r, 300, 20, 48)
autoStopEnableButton:setFrame(r)

r = belowRectWithMargin(r, 150, 20, 8)
autoStopPeriodLabelView:setFrame(r)

r2 = rightOfRectWithMargin(r, 500, 8)
autoStopPeriodSlider:setFrame(r2)

r2 = rightOfRectWithMargin(r2, 50, 8)
autoStopPeriodValueLabelView:setFrame(r2)

-- OK button

r2 = topView:bounds()
r = makeRect(0, 20, r2.size.width, 20)
okButton:setFrame(centeredRectInRect(r, 100, 20))
end

------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------

topView = getTopView()

boxView = BoxView.new("boxView")
boxView:setCornerRadius(5)
okButton = Button.new("okButton", "OK")

midiInputSelectionLabelView = LabelView.new("midiInputSelectionLabelView", midiInputStr)
midiInputSelectionLabelView:setTextAlign(RightTextAlign);
midiInputComboBox = ComboBox.new("midiInputComboBox")
midiInputComboBox:setListPosition(1)

midiOutputSelectionLabelView = LabelView.new("midiOutputSelectionLabelView", midiOutputStr)
midiOutputSelectionLabelView:setTextAlign(RightTextAlign);
midiOutputComboBox = ComboBox.new("midiOutputComboBox")
midiOutputComboBox:setListPosition(1)

audioOutputSelectionLabelView = LabelView.new("audioOutputSelectionLabelView", audioOutputStr)
audioOutputSelectionLabelView:setTextAlign(RightTextAlign);
audioOutputComboBox = ComboBox.new("audioOutputComboBox")
audioOutputComboBox:setListPosition(1)

audioOutputLatencyLabelView = LabelView.new("audioOutputLatencyLabelView", audioOutputLatencyStr)
audioOutputLatencyLabelView:setTextAlign(RightTextAlign);
audioOutputLatencySlider = Slider.new("audioOutputLatencySlider", 0.001, 0.200, 0.008)
audioOutputLatencyValueLabelView = LabelView.new("audioOutputLatencyValueLabelView", "")


midiDestLabelView = LabelView.new("midiDestLabelView", midiDestStr)
midiDestLabelView:setTextAlign(RightTextAlign);
midiDestInternalSynthButton = Button.new("midiDestInternalSynthButton", midiDestInternalSynthStr)
midiDestInternalSynthButton:setType(3)
midiDestInternalSynthButton:setHighlightColor(blueColor)
midiDestMIDIOutputButton = Button.new("midiDestMIDIOutputButton", midiDestMIDIOutputStr)
midiDestMIDIOutputButton:setType(3)
midiDestMIDIOutputButton:setHighlightColor(blueColor)

nbVoicesLabelView = LabelView.new("nbVoicesLabelView", nbVoicesStr)
nbVoicesLabelView:setTextAlign(RightTextAlign);
nbVoicesSlider = Slider.new("nbVoicesSlider", 24, 64, 64)
nbVoicesValueLabelView = LabelView.new("nbVoicesLabelView", "")

autoStopEnableButton = Button.new("autoStopEnableButton", autoStopStr)
autoStopEnableButton:setType(1)
autoStopPeriodLabelView = LabelView.new("autoStopPeriodLabelView", autoStopPeriodStr)
autoStopPeriodLabelView:setTextAlign(RightTextAlign);
autoStopPeriodSlider = Slider.new("autoStopPeriodSlider", 1, 30, 4)
autoStopPeriodValueLabelView = LabelView.new("autoStopPeriodValueLabelView", "")

topView:addSubview(boxView)
topView:addSubview(okButton)

topView:addSubview(midiInputSelectionLabelView)
topView:addSubview(midiInputComboBox)

topView:addSubview(midiOutputSelectionLabelView)
topView:addSubview(midiOutputComboBox)

topView:addSubview(audioOutputSelectionLabelView)
topView:addSubview(audioOutputComboBox)
topView:addSubview(audioOutputLatencyLabelView)
topView:addSubview(audioOutputLatencySlider)
topView:addSubview(audioOutputLatencyValueLabelView)

topView:addSubview(midiDestLabelView)
topView:addSubview(midiDestInternalSynthButton)
topView:addSubview(midiDestMIDIOutputButton)

topView:addSubview(nbVoicesLabelView)
topView:addSubview(nbVoicesSlider)
topView:addSubview(nbVoicesValueLabelView)

topView:addSubview(autoStopEnableButton)
topView:addSubview(autoStopPeriodLabelView)
topView:addSubview(autoStopPeriodSlider)
topView:addSubview(autoStopPeriodValueLabelView)

topView:setLayoutFn(layout)

setContentSize(makeSize(748, 464))
