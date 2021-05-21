--
-- ConfirmCleanupView.lua
--

require "Common"

if language() == "fr" then

confirmCleanupStr="Déplacer les séquences avec cote négative dans la corbeille?\n\nCette opération ne peut être défaite."
cancelStr = "Annuler"

else

confirmCleanupStr="Move the sequences with negative ratings in the Trash?\n\nThis operation cannot be undone."
cancelStr = "Cancel"

end


------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------

topView = getTopView()

boxView = BoxView.new("boxView")
boxView:setCornerRadius(5)
messageLabelView = LabelView.new("messageLabelView", confirmCleanupStr)
okButton = Button.new("okButton", "OK")
cancelButton = Button.new("cancelButton", cancelStr)

topView:addSubview(boxView)
topView:addSubview(messageLabelView)
topView:addSubview(okButton)
topView:addSubview(cancelButton)

topView:setLayoutFn(layoutConfirmationView)

setContentSize(makeSize(500, 200))