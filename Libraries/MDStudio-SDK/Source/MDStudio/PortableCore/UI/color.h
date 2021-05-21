//
//  color.h
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

// Ref.: https://en.wikipedia.org/wiki/Web_colors

#ifndef COLOR_H
#define COLOR_H

namespace MDStudio {

struct Color {
    float red, green, blue, alpha;
};

// Transparent
extern Color zeroColor;

// Gray/black colors
extern Color gainsboroColor;
extern Color lightGrayColor;
extern Color silverColor;
extern Color darkGrayColor;
extern Color grayColor;
extern Color dimGrayColor;
extern Color lightSlateGrayColor;
extern Color slateGrayColor;
extern Color darkSlateGrayColor;
extern Color blackColor;

// Pink colors
extern Color pinkColor;
extern Color lightPinkColor;
extern Color hotPinkColor;
extern Color deepPinkColor;
extern Color paleVioletRedColor;
extern Color mediumVioletRedColor;

// Red colors
extern Color lightSalmonColor;
extern Color salmonColor;
extern Color darkSalmonColor;
extern Color lightCoralColor;
extern Color indianRedColor;
extern Color crimsonColor;
extern Color fireBrickColor;
extern Color darkRedColor;
extern Color redColor;

// Orange colors
extern Color orangeRedColor;
extern Color tomatoColor;
extern Color coralColor;
extern Color darkOrangeColor;
extern Color orangeColor;

// Yellow colors
extern Color yellowColor;
extern Color lightYellowColor;
extern Color lemonChiffon;
extern Color lightGoldenrodYellow;
extern Color papayaWhip;
extern Color moccasinColor;
extern Color peachPuffColor;
extern Color paleGoldenrodColor;
extern Color khakiColor;
extern Color darkKhakiColor;
extern Color goldColor;

// Brown colors
extern Color cornsilkColor;
extern Color blanchedAlmondColor;
extern Color bisqueColor;
extern Color navajoWhiteColor;
extern Color wheatColor;
extern Color burlyWoodColor;
extern Color tanColor;
extern Color rosyBrownColor;
extern Color sandyBrownColor;
extern Color goldenrodColor;
extern Color darkGoldenrodColor;
extern Color peruColor;
extern Color chocolateColor;
extern Color saddleBrownColor;
extern Color siennaColor;
extern Color brownColor;
extern Color maroonColor;

// Green colors
extern Color darkOliveGreenColor;
extern Color oliveColor;
extern Color oliveDrabColor;
extern Color yellowGreenColor;
extern Color limeGreenColor;
extern Color limeColor;
extern Color lawnGreenColor;
extern Color chartreuseColor;
extern Color greenYellowColor;
extern Color springGreenColor;
extern Color mediumSpringGreenColor;
extern Color lightGreenColor;
extern Color paleGreenColor;
extern Color darkSeaGreenColor;
extern Color mediumAquamarineColor;
extern Color mediumSeaGreenColor;
extern Color seaGreenColor;
extern Color forestGreenColor;
extern Color greenColor;
extern Color darkGreenColor;

// Cyan colors
extern Color aquaColor;
extern Color cyanColor;
extern Color lightCyanColor;
extern Color paleTurquoiseColor;
extern Color aquamarineColor;
extern Color turquoiseColor;
extern Color mediumTurquoiseColor;
extern Color darkTurquoiseColor;
extern Color lightSeaGreenColor;
extern Color cadetBlueColor;
extern Color darkCyanColor;
extern Color tealColor;

// Blue colors
extern Color lightSteelBlueColor;
extern Color powderBlueColor;
extern Color lightBlueColor;
extern Color skyBlueColor;
extern Color lightSkyBlueColor;
extern Color deepSkyBlueColor;
extern Color dodgerBlueColor;
extern Color cornflowerBlueColor;
extern Color steelBlueColor;
extern Color royalBlueColor;
extern Color blueColor;
extern Color mediumBlueColor;
extern Color darkBlueColor;
extern Color navyColor;
extern Color midnightBlueColor;

// Purple/violet/magenta colors
extern Color lavenderColor;
extern Color thistleColor;
extern Color plumColor;
extern Color violetColor;
extern Color orchidColor;
extern Color fuchsiaColor;
extern Color magentaColor;
extern Color mediumOrchidColor;
extern Color mediumPurpleColor;
extern Color blueVioletColor;
extern Color darkVioletColor;
extern Color darkOrchidColor;
extern Color darkMagentaColor;
extern Color purpleColor;
extern Color indigoColor;
extern Color darkSlateBlueColor;
extern Color slateBlueColor;
extern Color mediumSlateBlueColor;

// White colors
extern Color whiteColor;
extern Color snowColor;
extern Color honeydewColor;
extern Color mintCreamColor;
extern Color azureColor;
extern Color aliceBlueColor;
extern Color ghostWhiteColor;
extern Color whiteSmokeColor;
extern Color seashellColor;
extern Color beigeColor;
extern Color oldLaceColor;
extern Color floralWhiteColor;
extern Color ivoryColor;
extern Color antiqueWhiteColor;
extern Color linenColor;
extern Color lavenderBlushColor;
extern Color mistyRoseColor;

// Non-standard colors
extern Color veryDimGrayColor;

// System colors
extern Color systemButtonBorderColor;

Color makeColor(float red, float green, float blue, float alpha);

}  // namespace MDStudio

#endif  // COLOR_H
