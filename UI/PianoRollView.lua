--
-- PianoRollView.lua
--

require "Common"

if language() == "fr" then

quantizeStr = "Quantifier"
addFlagTooltipStr = "Ajouter un drapeau"
removeFlagTooltipStr = "Supprimer un drapeau"
removeAllFlagsTooltipStr = "Supprimer tous les drapeaux"
goToPreviousFlagTooltipStr = "Aller au drapeau précédent"
goToNextFlagTooltipStr = "Aller au drapeau suivant"

else

quantizeStr = "Quantize"
addFlagTooltipStr = "Add flag"
removeFlagTooltipStr = "Remove flag"
removeAllFlagsTooltipStr = "Remove all flags"
goToPreviousFlagTooltipStr = "Go to previous flag"
goToNextFlagTooltipStr = "Go to next flag"

end
