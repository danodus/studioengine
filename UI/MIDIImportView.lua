--
-- MIDIImportView.lua
--

require "Common"

if language() == "fr" then

progressStr="Importation MIDI en cours..."

else

progressStr="MIDI import in progress..."

end

------------------------------------------------------------------------------------------------------------------------
function layout(sender)
local r = topView:bounds()
boxView:setFrame(r)
r = inset(r, 20, 20)
r2 = r
r2.origin.y = 48
r2.size.height = 20
labelView1:setFrame(r2)
r2 = belowRectWithMargin(r2, r2.size.width, 20, 8)
progressIndicator:setFrame(r2)
end

------------------------------------------------------------------------------------------------------------------------

topView = getTopView()

boxView = BoxView.new("boxView")
boxView:setCornerRadius(5)
labelView1 = LabelView.new("labelView1", progressStr)
progressIndicator = ProgressIndicator.new("progressIndicator", 1)

topView:addSubview(boxView)
topView:addSubview(labelView1)
topView:addSubview(progressIndicator)

topView:setLayoutFn(layout)

setContentSize(makeSize(600, 88))
