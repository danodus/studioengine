-- ControlsView.lua

require "Common"

if language() == "fr" then

noneStr="Aucun"
tapTempoStr="Tap Tempo"
specifiedStr="Spécifiés"

noneTooltipStr="Enregistrement sans indicatif de mesure"
tapTempoTooltipStr="Enregistrement avec Tap Tempo"
specifiedTooltipStr="Enregistement avec indicatif de mesure et tempo spécifiés"

tempoLabelStr="T"
timeSignatureLabelStr="IM"

else

noneStr="None"
tapTempoStr="Tap Tempo"
specifiedStr="Specified"

noneTooltipStr="Record without time signature"
tapTempoTooltipStr="Record with Tap Tempo"
specifiedTooltipStr="Record with specified time signature and tempo"

tempoLabelStr="T"
timeSignatureLabelStr="TS"

end

function metronomeLayout(sender)
    local r = makeRect(0.0, sender:bounds().size.height - 20.0, sender:bounds().size.width, 20.0)
    metronomeNoneButton:setFrame(r)
    r.origin.y = r.origin.y - r.size.height
    metronomeTapTempoButton:setFrame(r)
    r.origin.y = r.origin.y - r.size.height
    metronomeSpecifiedButton:setFrame(r)
end

function metronomeSettingsLayout(sender)

    r = sender:bounds()
    r = makeRect(0, r.size.height - 20, 20, 16)

    timeSigLabelView:setFrame(r)

    local r2 = rightOfRectWithMargin(r, 150, 8)
    timeSigNumSegmentedControl:setFrame(r2)

    r.origin.y = r.origin.y - r.size.height - 4
    r2.origin.y = r2.origin.y - r2.size.height - 4
    timeSigDenumSegmentedControl:setFrame(r2)

    r.origin.y = r.origin.y - r.size.height - 4
    tempoLabelView:setFrame(r)

    r2.origin.y = r2.origin.y - r2.size.height - 4
    r2.size.width = 100
    tempoSlider:setFrame(r2)

    r2 = rightOfRectWithMargin(r2, 40, 8)
    tempoValueTextField:setFrame(r2)

end


function layout(sender)

    local r = makeRect(0.0, 0.0, sender:bounds().size.width, 80);
    controlsBoxView:setFrame(r)

    r = makeRect(10, 10, 60, 60)
    rewindButton:setFrame(r)
    r.origin.x = r.origin.x + 70
    playButton:setFrame(r);
    r.origin.x = r.origin.x + 100
    recordButton:setFrame(r)
    r.origin.x = r.origin.x + r.size.width + 20

    r.size.width = 120
    metronomeView:setFrame(r)

    r.origin.x = r.origin.x + r.size.width
    r.size.width = 200
    metronomeSettingsView:setFrame(r)

    r.origin.x = r.origin.x + r.size.width + 30
    r.size.width = 60
    convertSequenceButton:setFrame(r)

    r.origin.x = sender:bounds().size.width - 100 - 10
    r.size.width = 100

    statusBackgroundImageView:setFrame(r)
    statusImageView:setFrame(r)

end

function draw(sender)
    local dc = sender:drawContext()
    dc:pushStates()
    dc:setStrokeColor(blackColor)
    dc:setStrokeWidth(1.5)
    dc:drawRoundRect(makeRect(370, 5, 200, 70), 10)
    dc:popStates()
end

topView = getTopView()

-- Properties

metronomeModeProperty = Property.new("metronomeMode")
metronomeModeProperty:setValueWillChangeFn(function(sender)
    metronomeNoneButton:setState(false)
    metronomeTapTempoButton:setState(false)
    metronomeSpecifiedButton:setState(false)
end)

metronomeModeProperty:setValueDidChangeFn(function(sender)
    local value = sender:value()
    if value == 0 then
        metronomeNoneButton:setState(true)
    elseif value == 1 then
        metronomeTapTempoButton:setState(true)
    elseif value == 2 then
        metronomeSpecifiedButton:setState(true)
    end
end)

timeSigNumProperty = Property.new("timeSigNumProperty")
timeSigNumProperty:setValueDidChangeFn(function(sender)
    local value = sender:value()
    local conv = {[2]=1,[3]=2,[4]=3,[5]=4,[7]=5,[11]=6}
    timeSigNumSegmentedControl:setSelectedSegment(conv[value] or -1, false)
end)

timeSigDenumProperty = Property.new("timeSigDenumProperty")
timeSigDenumProperty:setValueDidChangeFn(function(sender)
    local value = sender:value()
    local conv = {[1]=1,[2]=2,[4]=3,[8]=4,[16]=5,[32]=6,[64]=7}
    timeSigDenumSegmentedControl:setSelectedSegment(conv[value] or -1, false)
end)

tempoProperty = Property.new("tempoProperty")
tempoProperty:setValueDidChangeFn(function(sender)
    local value = sender:value()
    tempoSlider:setPos(value, false)
    tempoValueTextField:setText(tostring(value), false)
end)

-- Load images
previousImage = Image.new("Previous@2x.png")
playImage = Image.new("Play@2x.png")
pauseImage = Image.new("Pause@2x.png")
recordImage = Image.new("Record@2x.png")
convertToMTImage = Image.new("ConvertToMT@2x.png")
convertToSTImage = Image.new("ConvertToST@2x.png")
statusBackgroundImage = Image.new("StudioStatusBackground@2x.png")

-- Add controls view
controlsBoxView = BoxView.new("controlsBoxView")
controlsBoxView:setFillColors(makeColor(0.6, 0.6, 0.6, 1.0), lightGrayColor)
controlsBoxView:setBorderColor(lightGrayColor)
rewindButton = Button.new("rewindButton", "", previousImage)
rewindButton:setBorderColor(zeroColor)
playButton = Button.new("playButton", "", playImage)
playButton:setBorderColor(zeroColor)
recordButton = Button.new("recordButton", "", recordImage)
recordButton:setBorderColor(zeroColor)

metronomeView = View.new("metronomeView")
metronomeView:setLayoutFn(metronomeLayout)

metronomeSettingsView = View.new("metronomeSettingsView")
metronomeSettingsView:setLayoutFn(metronomeSettingsLayout)

metronomeNoneButton = Button.new("metronomeNoneButton", noneStr)
metronomeNoneButton:setType(RadioButtonType)
metronomeNoneButton:setTextColor(blackColor)
metronomeNoneButton:setBorderColor(blackColor)
metronomeNoneButton:setClickedFn(function(sender)
    metronomeModeProperty:setValue(0)
end)
metronomeNoneButton:setTooltipText(noneTooltipStr)
metronomeTapTempoButton = Button.new("metronomeTapTempoButton", tapTempoStr)
metronomeTapTempoButton:setType(RadioButtonType)
metronomeTapTempoButton:setTextColor(blackColor)
metronomeTapTempoButton:setBorderColor(blackColor)
metronomeTapTempoButton:setClickedFn(function(sender)
    metronomeModeProperty:setValue(1)
end)
metronomeTapTempoButton:setTooltipText(tapTempoTooltipStr)
metronomeSpecifiedButton = Button.new("metronomeSpecifiedButton", specifiedStr)
metronomeSpecifiedButton:setType(RadioButtonType)
metronomeSpecifiedButton:setTextColor(blackColor)
metronomeSpecifiedButton:setBorderColor(blackColor)
metronomeSpecifiedButton:setClickedFn(function(sender)
    metronomeModeProperty:setValue(2)
end)
metronomeSpecifiedButton:setTooltipText(specifiedTooltipStr)

metronomeView:addSubview(metronomeNoneButton)
metronomeView:addSubview(metronomeTapTempoButton)
metronomeView:addSubview(metronomeSpecifiedButton)

timeSigLabelView = LabelView.new("timeSigLabelView", timeSignatureLabelStr)
timeSigLabelView:setTextAlign(RightTextAlign)
timeSigLabelView:setTextColor(blackColor)
timeSigNumSegmentedControl = SegmentedControl.new("timeSigNumSegmentedControl", {"2", "3", "4", "5", "7", "11"})
timeSigNumSegmentedControl:setBorderColor(blackColor)
timeSigNumSegmentedControl:setTextColor(blackColor)
timeSigNumSegmentedControl:setHighlightColor(makeColor(0,1,1,1))
timeSigNumSegmentedControl:setDidSelectSegmentFn(function(sender, selectedSegment)
    local conv={2,3,4,5,7,11}
    timeSigNumProperty:setValue(conv[selectedSegment])
end)
timeSigDenumSegmentedControl = SegmentedControl.new("timeSigDenumSegmentedControl", {"1", "2", "4", "8", "16", "32", "64"})
timeSigDenumSegmentedControl:setBorderColor(blackColor)
timeSigDenumSegmentedControl:setTextColor(blackColor)
timeSigDenumSegmentedControl:setHighlightColor(makeColor(0,1,1,1))
timeSigDenumSegmentedControl:setDidSelectSegmentFn(function(sender, selectedSegment)
    local conv={1,2,4,8,16,32,64}
    timeSigDenumProperty:setValue(conv[selectedSegment])
end)

tempoLabelView = LabelView.new("tempoLabelView", tempoLabelStr)
tempoLabelView:setTextAlign(RightTextAlign)
tempoLabelView:setTextColor(blackColor)
tempoSlider = Slider.new("tempoSlider", 30, 300, 120)
tempoSlider:setPosChangedFn(function(sender, pos)
    tempoProperty:setValue(math.floor((pos + 0.5) / 5) * 5)
end)
tempoValueTextField = TextField.new("tempoValueTextField")
--tempoValueTextField:setTextColor(blackColor)
tempoValueTextField:setTextDidChangeFn(function(sender, text)
    local value = tonumber(text)
   
    if value then
        value = math.floor(value)
        if value < 30 then
            value = 30
        elseif value > 300 then
            value = 300
        end
    else
        value = tempoProperty:value()
    end
    tempoProperty:setValue(value)
end)

metronomeSettingsView:addSubview(timeSigLabelView)
metronomeSettingsView:addSubview(timeSigNumSegmentedControl)
metronomeSettingsView:addSubview(timeSigDenumSegmentedControl)
metronomeSettingsView:addSubview(tempoLabelView)
metronomeSettingsView:addSubview(tempoSlider)
metronomeSettingsView:addSubview(tempoValueTextField)

convertSequenceButton = Button.new("convertSequenceButton", "", convertToMTImage)
convertSequenceButton:setBorderColor(zeroColor)
statusBackgroundImageView = ImageView.new("statusBackgroundImageView", statusBackgroundImage, false)
statusImageView = ImageView.new("statusImageView", nil, false)
topView:addSubview(controlsBoxView)
topView:addSubview(rewindButton)
topView:addSubview(playButton)
topView:addSubview(recordButton)
topView:addSubview(metronomeView)
topView:addSubview(metronomeSettingsView)
topView:addSubview(convertSequenceButton)
topView:addSubview(statusBackgroundImageView)
topView:addSubview(statusImageView)

topView:setLayoutFn(layout)
topView:setDrawFn(draw)

-- Initial property values
tempoProperty:setValue(120)
metronomeModeProperty:setValue(2)
timeSigNumProperty:setValue(4)
timeSigDenumProperty:setValue(4)

