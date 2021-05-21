--
-- StudioChannelView.lua
--

require "Common"
require "StudioKnob"
require "StudioSlider"

if language() == "fr" then

level="Niveau"
balance="Balance"

else

level="Level"
balance="Balance"

end

------------------------------------------------------------------------------------------------------------------------
function layout(sender)
    local r = topView:bounds()

    backgroundImageView:setFrame(r)
    backgroundGlossImageView:setFrame(r)

    r = centeredRectInRect(r, 245, 20)
    r.origin.y = r.origin.y + 35
    instrumentComboBox:setFrame(r)

    r = belowRectWithMargin(r, 245 + 20, 26, 6)
    levelStudioSlider:view():setFrame(r)

    r = belowRectWithMargin(r, 60, 50, 0)
    r.origin.x = r.origin.x + 33
    balanceStudioKnob:view():setFrame(r)

    r = rightOfRectWithMargin(r, 60, 0)
    reverbStudioKnob:view():setFrame(r)

    r = rightOfRectWithMargin(r, 60, 0)
    chorusStudioKnob:view():setFrame(r)
end

------------------------------------------------------------------------------------------------------------------------

topView = getTopView()

backgroundImage = Image.new("BackgroundGraySmall@2x.png")
backgroundImageView = ImageView.new("backgroundImageView", backgroundImage)

backgroundGlossImage = Image.new("BackgroundGlossSmall@2x.png")
backgroundGlossImageView = ImageView.new("BackgroundGlossImageView", backgroundGlossImage)

instrumentComboBox = ComboBox.new("instrumentComboBox")
instrumentComboBox:setMaxHeight(350)

levelStudioSlider = StudioSlider.new("levelStudioSlider", "dB", -54 - 3, 0, 0, {"-∞", "  -54", "", "", "", "", "", "-36", "", "", "", "", "", "-18", "", "-12  ", "-9", "-6", "-3", " 0"})
balanceStudioKnob = StudioKnob.new("balanceStudioKnob", "PAN", -1, 1, 0, PanStudioKnobType)
reverbStudioKnob = StudioKnob.new("reverbStudioKnob", "REVERB", 0, 1, 0, StandardStudioKnobType)
chorusStudioKnob = StudioKnob.new("chorusStudioKnob", "CHORUS", 0, 1, 0, StandardStudioKnobType)

levelSlider = levelStudioSlider:slider()
balanceSlider = balanceStudioKnob:slider()
reverbSlider = reverbStudioKnob:slider()
chorusSlider = chorusStudioKnob:slider()

topView:addSubview(backgroundImageView)
topView:addSubview(backgroundGlossImageView)
topView:addSubview(instrumentComboBox)
topView:addSubview(levelStudioSlider:view())
topView:addSubview(balanceStudioKnob:view())
topView:addSubview(reverbStudioKnob:view())
topView:addSubview(chorusStudioKnob:view())

topView:setLayoutFn(layout)
