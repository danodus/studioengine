--
-- ConfirmSetAsPlayedAllView.lua
--

require "Common"

if language() == "fr" then

confirmSetAsPlayedAllStr="Mettre comme jouées toutes les séquences?\n\nCette opération ne peut être défaite."
cancelStr = "Annuler"

else

confirmSetAsPlayedAllStr="Set as played all the sequences?\n\nThis operation cannot be undone."
cancelStr = "Cancel"

end

------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------

topView = getTopView()

boxView = BoxView.new("boxView")
boxView:setCornerRadius(5)
messageLabelView = LabelView.new("messageLabelView", confirmSetAsPlayedAllStr)
okButton = Button.new("okButton", "OK")
cancelButton = Button.new("cancelButton", cancelStr)

topView:addSubview(boxView)
topView:addSubview(messageLabelView)
topView:addSubview(okButton)
topView:addSubview(cancelButton)

topView:setLayoutFn(layoutConfirmationView)

setContentSize(makeSize(500, 200))
