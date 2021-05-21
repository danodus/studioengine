--
-- AboutView.lua
--

require "Common"

if language() == "fr" then

aboutText="Studio Engine"
copyright= "Copyright © 2021 Daniel Cliche. Tous droits réservés."
acknowledgementsButtonTitle="Remerciements..."
licensedToStr="Licencié(e): "

else

aboutText = "Studio Engine"
copyright= "Copyright © 2021 Daniel Cliche. All rights reserved."
acknowledgementsButtonTitle="Acknowledgements..."
licensedToStr="Licensed to: "

end

------------------------------------------------------------------------------------------------------------------------
function layout(sender)
local r = topView:bounds()
boxView:setFrame(r)
r = makeRect(128, 48, 400, 130)
r = inset(r, 10, 10)
labelView1:setFrame(r)

r = makeRect(20, 20, 300, 20)
licensedToLabel:setFrame(r)

r = makeRect(20, 60, 100, 100)
iconImageView:setFrame(r)
r = makeRect(600 - 20 - 100, 20, 100, 20)
okButton:setFrame(r)
r = leftOfRectWithMargin(r, 150, 8)
acknowledgementsButton:setFrame(r)
end

------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------

topView = getTopView()

description = aboutText .. "\nv. " .. appVersion() .. "\n" .. copyright

description = description .. "\n\n" .. "MelobaseCore v. " .. appCoreVersion()
description = description .. ", " .. "MDStudio v. " .. studioVersion()

boxView = BoxView.new("boxView")
boxView:setCornerRadius(5)
labelView1 = LabelView.new("labelView1", description)
okButton = Button.new("okButton", "OK")
acknowledgementsButton = Button.new("acknowledgementsButton", acknowledgementsButtonTitle)
iconImage = Image.new("Icon512@2x.png")
iconImageView = ImageView.new("iconImageView", iconImage)

licensedToLabel = LabelView.new("licensedToLabel", "");

topView:addSubview(boxView)
topView:addSubview(labelView1)
topView:addSubview(okButton)
topView:addSubview(acknowledgementsButton)
topView:addSubview(iconImageView)
topView:addSubview(licensedToLabel)

topView:setLayoutFn(layout)

setContentSize(makeSize(600, 180))