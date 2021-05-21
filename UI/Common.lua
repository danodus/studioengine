--
-- Common.lua
--

if language() == "fr" then

trashStr = "Corbeille"
sequencesStr = "Séquences"

else

trashStr = "Trash"
sequencesStr = "Sequences"

end


LeftTextAlign = 0
CenterTextAlign = 1
RightTextAlign = 2

LinearSliderType = 0
RadialSliderType = 1


------------------------------------------------------------------------------------------------------------------------
-- Colors

if makeColor then

function RGBA(red, green, blue, alpha)
    return makeColor(red / 255, green / 255, blue / 255, alpha / 255)
end

function RGB(red, green, blue)
    return RGBA(red, green, blue, 255)
end

-- Transparent
zeroColor = RGBA(0, 0, 0, 0)

-- Gray/black colors
gainsboroColor = RGB(220, 220, 220)
lightGrayColor = RGB(211, 211, 211)
silverColor = RGB(192, 192, 192)
darkGrayColor = RGB(169, 169, 169)
grayColor = RGB(128, 128, 128)
dimGrayColor = RGB(105, 105, 105)
lightSlateGrayColor = RGB(119, 136, 153)
slateGrayColor = RGB(112, 128, 144)
darkSlateGrayColor = RGB(47, 79, 79)
blackColor = RGB(0, 0, 0)

-- Pink colors
pinkColor = RGB(255, 192, 203)
lightPinkColor = RGB(255, 182, 193)
hotPinkColor = RGB(255, 105, 180)
deepPinkColor = RGB(255, 20, 147)
paleVioletRedColor = RGB(219, 112, 147)
mediumVioletRedColor = RGB(199, 21, 133)

-- Red colors
lightSalmonColor = RGB(255, 160, 122)
salmonColor = RGB(250, 128, 114)
darkSalmonColor = RGB(233, 150, 122)
lightCoralColor = RGB(240, 128, 128)
indianRedColor = RGB(205, 92, 92)
crimsonColor = RGB(220, 20, 60)
fireBrickColor = RGB(178, 34, 34)
darkRedColor = RGB(139, 0, 0)
redColor = RGB(255, 0, 0)

-- Orange colors
orangeRedColor = RGB(255, 69, 0)
tomatoColor = RGB(255, 99, 71)
coralColor = RGB(255, 127, 80)
darkOrangeColor = RGB(255, 140, 0)
orangeColor = RGB(255, 165, 0)

-- Yellow colors
yellowColor = RGB(255, 255, 0)
lightYellowColor = RGB(255, 255, 224)
lemonChiffon = RGB(255, 250, 205)
lightGoldenrodYellow = RGB(250, 250, 210)
papayaWhip = RGB(255, 239, 213)
moccasinColor = RGB(225, 228, 181)
peachPuffColor = RGB(255, 218, 185)
paleGoldenrodColor = RGB(238, 232, 170)
khakiColor = RGB(240, 230, 140)
darkKhakiColor = RGB(189, 183, 107)
goldColor = RGB(255, 215, 0)

-- Brown colors
cornsilkColor = RGB(255, 248, 220)
blanchedAlmondColor = RGB(255, 235, 205)
bisqueColor = RGB(255, 228, 196)
navajoWhiteColor = RGB(255, 222, 173)
wheatColor = RGB(245, 222, 179)
burlyWoodColor = RGB(222, 184, 135)
tanColor = RGB(210, 180, 140)
rosyBrownColor = RGB(188, 143, 143)
sandyBrownColor = RGB(244, 164, 96)
goldenrodColor = RGB(218, 165, 32)
darkGoldenrodColor = RGB(184, 134, 11)
peruColor = RGB(205, 133, 63)
chocolateColor = RGB(210, 105, 30)
saddleBrownColor = RGB(139, 69, 19)
siennaColor = RGB(160, 82, 45)
brownColor = RGB(165, 42, 42)
maroonColor = RGB(128, 0, 0)

-- Green colors
darkOliveGreenColor = RGB(85, 107, 47)
oliveColor = RGB(128, 128, 0)
oliveDrabColor = RGB(107, 142, 35)
yellowGreenColor = RGB(154, 205, 50)
limeGreenColor = RGB(50, 205, 50)
limeColor = RGB(0, 255, 0)
lawnGreenColor = RGB(124, 252, 0)
chartreuseColor = RGB(127, 255, 0)
greenYellowColor = RGB(173, 255, 47)
springGreenColor = RGB(0, 255, 127)
mediumSpringGreenColor = RGB(0, 250, 154)
lightGreenColor = RGB(144, 238, 144)
paleGreenColor = RGB(152, 251, 152)
darkSeaGreenColor = RGB(143, 188, 143)
mediumAquamarineColor = RGB(102, 205, 170)
mediumSeaGreenColor = RGB(60, 179, 113)
seaGreenColor = RGB(46, 139, 87)
forestGreenColor = RGB(34, 139, 34)
greenColor = RGB(0, 128, 0)
darkGreenColor = RGB(0, 100, 0)

-- Cyan colors
aquaColor = RGB(0, 255, 255)
cyanColor = RGB(0, 255, 255)
lightCyanColor = RGB(224, 255, 255)
paleTurquoiseColor = RGB(175, 238, 238)
aquamarineColor = RGB(127, 255, 212)
turquoiseColor = RGB(64, 224, 208)
mediumTurquoiseColor = RGB(72, 209, 204)
darkTurquoiseColor = RGB(0, 206, 209)
lightSeaGreenColor = RGB(32, 178, 170)
cadetBlueColor = RGB(95, 158, 160)
darkCyanColor = RGB(0, 139, 139)
tealColor = RGB(0, 128, 128)

-- Blue colors
lightSteelBlueColor = RGB(176, 196, 222)
powderBlueColor = RGB(176, 224, 230)
lightBlueColor = RGB(173, 216, 230)
skyBlueColor = RGB(135, 206, 235)
lightSkyBlueColor = RGB(135, 206, 250)
deepSkyBlueColor = RGB(0, 191, 255)
dodgerBlueColor = RGB(30, 144, 255)
cornflowerBlueColor = RGB(100, 149, 237)
steelBlueColor = RGB(70, 130, 180)
royalBlueColor = RGB(65, 105, 225)
blueColor = RGB(0, 0, 255)
mediumBlueColor = RGB(0, 0, 205)
darkBlueColor = RGB(0, 0, 139)
navyColor = RGB(0, 0, 128)
midnightBlueColor = RGB(25, 25, 112)

-- Purple/violet/magenta colors
lavenderColor = RGB(230, 230, 250)
thistleColor = RGB(216, 191, 216)
plumColor = RGB(221, 160, 221)
violetColor = RGB(238, 130, 238)
orchidColor = RGB(218, 112, 214)
fuchsiaColor = RGB(255, 0, 255)
magentaColor = RGB(255, 0, 255)
mediumOrchidColor = RGB(186, 85, 211)
mediumPurpleColor = RGB(147, 112, 219)
blueVioletColor = RGB(138, 43, 226)
darkVioletColor = RGB(148, 0, 211)
darkOrchidColor = RGB(153, 50, 204)
darkMagentaColor = RGB(139, 0, 139)
purpleColor = RGB(128, 0, 128)
indigoColor = RGB(75, 0, 130)
darkSlateBlueColor = RGB(72, 61, 139)
slateBlueColor = RGB(106, 90, 205)
mediumSlateBlueColor = RGB(123, 104, 238)

-- White colors
whiteColor = RGB(255, 255, 255)
snowColor = RGB(255, 250, 250)
honeydewColor = RGB(240, 255, 240)
mintCreamColor = RGB(245, 255, 250)
azureColor = RGB(240, 255, 255)
aliceBlueColor = RGB(240, 248, 255)
ghostWhiteColor = RGB(248, 248, 255)
whiteSmokeColor = RGB(245, 245, 245)
seashellColor = RGB(255, 245, 238)
beigeColor = RGB(245, 245, 220)
oldLaceColor = RGB(253, 245, 230)
floralWhiteColor = RGB(255, 250, 240)
ivoryColor = RGB(255, 255, 240)
antiqueWhiteColor = RGB(250, 235, 215)
linenColor = RGB(250, 240, 230)
lavenderBlushColor = RGB(255, 240, 245)
mistyRoseColor = RGB(255, 228, 225)

-- Non-standard colors
veryDimGrayColor = RGB(64, 64, 64)


-- System colors
systemButtonBorderColor = lightGrayColor

end -- if makeColor

------------------------------------------------------------------------------------------------------------------------
-- Button

StandardButtonType = 0
CheckBoxButtonType = 1
CustomCheckBoxButtonType = 2
RadioButtonType = 3
OKButtonType = 4
CancelButtonType = 5
ComboBoxUpButtonType = 6
ComboBoxDownButtonType = 7
SegmentedControlButtonType = 8
DisclosureButtonType = 9
SortButtonType = 10

------------------------------------------------------------------------------------------------------------------------
function centeredRectInRect(rect, width, height)
cx = rect.origin.x + rect.size.width / 2.0
cy = rect.origin.y + rect.size.height / 2.0
return makeRect(math.floor(cx - width / 2.0), math.floor(cy - height / 2.0), width, height)
end

------------------------------------------------------------------------------------------------------------------------
function leftOfRect(rect, width)
return makeRect(rect.origin.x - width, rect.origin.y, width, rect.size.height)
end

------------------------------------------------------------------------------------------------------------------------
function leftOfRectWithMargin(rect, width, margin)
return makeRect(rect.origin.x - width - margin, rect.origin.y, width, rect.size.height)
end

------------------------------------------------------------------------------------------------------------------------
function rightOfRect(rect, width)
return makeRect(rect.origin.x + rect.size.width, rect.origin.y, width, rect.size.height)
end

------------------------------------------------------------------------------------------------------------------------
function rightOfRectWithMargin(rect, width, margin)
return makeRect(rect.origin.x + rect.size.width + margin, rect.origin.y, width, rect.size.height)
end

------------------------------------------------------------------------------------------------------------------------
function belowRectWithMargin(rect, width, height, margin)
return makeRect(rect.origin.x, rect.origin.y - height - margin, width, height)
end

------------------------------------------------------------------------------------------------------------------------
function inset(rect, dx, dy)
return makeRect(rect.origin.x + dx, rect.origin.y + dy, rect.size.width - dx * 2, rect.size.height - dy * 2)
end

------------------------------------------------------------------------------------------------------------------------
function layoutConfirmationView(sender)
local r = topView:bounds()
boxView:setFrame(r)

r.origin.y = r.origin.y + 100
r.size.height = r.size.height - 100
r = inset(r, 20, 20)
messageLabelView:setFrame(r)

if operatingSystem() == "windows" then
    leftButton = okButton
    rightButton = cancelButton
else
    leftButton = cancelButton
    rightButton = okButton
end

r = topView:bounds()
r = makeRect(r.size.width - 20 - 100 - 12 - 100, 20, 100, 20)
leftButton:setFrame(r)

r = rightOfRectWithMargin(r, 100, 12)
rightButton:setFrame(r)
end
