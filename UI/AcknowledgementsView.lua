--
-- AcknowledgementsView.lua
--

require "Common"

if language() == "fr" then

thanks="Des parties de ce logiciel peuvent utiliser le matériel suivant protégé par le droit d'auteur:"

else

thanks="Portions of this software may utilize the following copyrighted material:"

end

------------------------------------------------------------------------------------------------------------------------
function layout(sender)
local r = topView:bounds()
boxView:setFrame(centeredRectInRect(r, 800, 500))
r = makeRect(20, 500 - 40, 800, 20)
labelView:setFrame(r)
r = makeRect(0, 48, 800, 500 - 40 - 48 - 8)
r = inset(r, 20, 0)
scrollView:setFrame(r)
r = makeRect(0, 20, 800, 20)
okButton:setFrame(centeredRectInRect(r, 100, 20))
end

------------------------------------------------------------------------------------------------------------------------

topView = getTopView();

f = assert(io.open("Acknowledgements.txt", "r"))
acknowledgments = f:read("*all")
f:close(f)

boxView = BoxView.new("boxView")
boxView:setCornerRadius(5)
labelView = LabelView.new("labelView", thanks)
textView = TextView.new("textView")
textView:setText(acknowledgments)
textView:setIsEnabled(false)
scrollView = ScrollView.new("scrollView", textView)
scrollView:setContentSize(textView:contentSize())
okButton = Button.new("okButton", "OK")

topView:addSubview(boxView)
topView:addSubview(labelView)
topView:addSubview(scrollView)
topView:addSubview(okButton)

topView:setLayoutFn(layout)

setContentSize(makeSize(800, 500))
