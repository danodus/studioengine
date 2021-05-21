//
//  color.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "color.h"

using namespace MDStudio;

// Ref.: https://en.wikipedia.org/wiki/Web_colors

#define RGBA(_red_, _green_, _blue_, _alpha_) \
    { (float)_red_ / 255.0f, (float)_green_ / 255.0f, (float)_blue_ / 255.0f, (float)_alpha_ / 255.0f }
#define RGB(_red_, _green_, _blue_) RGBA(_red_, _green_, _blue_, 255)

// Transparent
Color MDStudio::zeroColor = RGBA(0, 0, 0, 0);

// Gray/black colors
Color MDStudio::gainsboroColor = RGB(220, 220, 220);
Color MDStudio::lightGrayColor = RGB(211, 211, 211);
Color MDStudio::silverColor = RGB(192, 192, 192);
Color MDStudio::darkGrayColor = RGB(169, 169, 169);
Color MDStudio::grayColor = RGB(128, 128, 128);
Color MDStudio::dimGrayColor = RGB(105, 105, 105);
Color MDStudio::lightSlateGrayColor = RGB(119, 136, 153);
Color MDStudio::slateGrayColor = RGB(112, 128, 144);
Color MDStudio::darkSlateGrayColor = RGB(47, 79, 79);
Color MDStudio::blackColor = RGB(0, 0, 0);

// Pink colors
Color MDStudio::pinkColor = RGB(255, 192, 203);
Color MDStudio::lightPinkColor = RGB(255, 182, 193);
Color MDStudio::hotPinkColor = RGB(255, 105, 180);
Color MDStudio::deepPinkColor = RGB(255, 20, 147);
Color MDStudio::paleVioletRedColor = RGB(219, 112, 147);
Color MDStudio::mediumVioletRedColor = RGB(199, 21, 133);

// Red colors
Color MDStudio::lightSalmonColor = RGB(255, 160, 122);
Color MDStudio::salmonColor = RGB(250, 128, 114);
Color MDStudio::darkSalmonColor = RGB(233, 150, 122);
Color MDStudio::lightCoralColor = RGB(240, 128, 128);
Color MDStudio::indianRedColor = RGB(205, 92, 92);
Color MDStudio::crimsonColor = RGB(220, 20, 60);
Color MDStudio::fireBrickColor = RGB(178, 34, 34);
Color MDStudio::darkRedColor = RGB(139, 0, 0);
Color MDStudio::redColor = RGB(255, 0, 0);

// Orange colors
Color MDStudio::orangeRedColor = RGB(255, 69, 0);
Color MDStudio::tomatoColor = RGB(255, 99, 71);
Color MDStudio::coralColor = RGB(255, 127, 80);
Color MDStudio::darkOrangeColor = RGB(255, 140, 0);
Color MDStudio::orangeColor = RGB(255, 165, 0);

// Yellow colors
Color MDStudio::yellowColor = RGB(255, 255, 0);
Color MDStudio::lightYellowColor = RGB(255, 255, 224);
Color MDStudio::lemonChiffon = RGB(255, 250, 205);
Color MDStudio::lightGoldenrodYellow = RGB(250, 250, 210);
Color MDStudio::papayaWhip = RGB(255, 239, 213);
Color MDStudio::moccasinColor = RGB(225, 228, 181);
Color MDStudio::peachPuffColor = RGB(255, 218, 185);
Color MDStudio::paleGoldenrodColor = RGB(238, 232, 170);
Color MDStudio::khakiColor = RGB(240, 230, 140);
Color MDStudio::darkKhakiColor = RGB(189, 183, 107);
Color MDStudio::goldColor = RGB(255, 215, 0);

// Brown colors
Color MDStudio::cornsilkColor = RGB(255, 248, 220);
Color MDStudio::blanchedAlmondColor = RGB(255, 235, 205);
Color MDStudio::bisqueColor = RGB(255, 228, 196);
Color MDStudio::navajoWhiteColor = RGB(255, 222, 173);
Color MDStudio::wheatColor = RGB(245, 222, 179);
Color MDStudio::burlyWoodColor = RGB(222, 184, 135);
Color MDStudio::tanColor = RGB(210, 180, 140);
Color MDStudio::rosyBrownColor = RGB(188, 143, 143);
Color MDStudio::sandyBrownColor = RGB(244, 164, 96);
Color MDStudio::goldenrodColor = RGB(218, 165, 32);
Color MDStudio::darkGoldenrodColor = RGB(184, 134, 11);
Color MDStudio::peruColor = RGB(205, 133, 63);
Color MDStudio::chocolateColor = RGB(210, 105, 30);
Color MDStudio::saddleBrownColor = RGB(139, 69, 19);
Color MDStudio::siennaColor = RGB(160, 82, 45);
Color MDStudio::brownColor = RGB(165, 42, 42);
Color MDStudio::maroonColor = RGB(128, 0, 0);

// Green colors
Color MDStudio::darkOliveGreenColor = RGB(85, 107, 47);
Color MDStudio::oliveColor = RGB(128, 128, 0);
Color MDStudio::oliveDrabColor = RGB(107, 142, 35);
Color MDStudio::yellowGreenColor = RGB(154, 205, 50);
Color MDStudio::limeGreenColor = RGB(50, 205, 50);
Color MDStudio::limeColor = RGB(0, 255, 0);
Color MDStudio::lawnGreenColor = RGB(124, 252, 0);
Color MDStudio::chartreuseColor = RGB(127, 255, 0);
Color MDStudio::greenYellowColor = RGB(173, 255, 47);
Color MDStudio::springGreenColor = RGB(0, 255, 127);
Color MDStudio::mediumSpringGreenColor = RGB(0, 250, 154);
Color MDStudio::lightGreenColor = RGB(144, 238, 144);
Color MDStudio::paleGreenColor = RGB(152, 251, 152);
Color MDStudio::darkSeaGreenColor = RGB(143, 188, 143);
Color MDStudio::mediumAquamarineColor = RGB(102, 205, 170);
Color MDStudio::mediumSeaGreenColor = RGB(60, 179, 113);
Color MDStudio::seaGreenColor = RGB(46, 139, 87);
Color MDStudio::forestGreenColor = RGB(34, 139, 34);
Color MDStudio::greenColor = RGB(0, 128, 0);
Color MDStudio::darkGreenColor = RGB(0, 100, 0);

// Cyan colors
Color MDStudio::aquaColor = RGB(0, 255, 255);
Color MDStudio::cyanColor = RGB(0, 255, 255);
Color MDStudio::lightCyanColor = RGB(224, 255, 255);
Color MDStudio::paleTurquoiseColor = RGB(175, 238, 238);
Color MDStudio::aquamarineColor = RGB(127, 255, 212);
Color MDStudio::turquoiseColor = RGB(64, 224, 208);
Color MDStudio::mediumTurquoiseColor = RGB(72, 209, 204);
Color MDStudio::darkTurquoiseColor = RGB(0, 206, 209);
Color MDStudio::lightSeaGreenColor = RGB(32, 178, 170);
Color MDStudio::cadetBlueColor = RGB(95, 158, 160);
Color MDStudio::darkCyanColor = RGB(0, 139, 139);
Color MDStudio::tealColor = RGB(0, 128, 128);

// Blue colors
Color MDStudio::lightSteelBlueColor = RGB(176, 196, 222);
Color MDStudio::powderBlueColor = RGB(176, 224, 230);
Color MDStudio::lightBlueColor = RGB(173, 216, 230);
Color MDStudio::skyBlueColor = RGB(135, 206, 235);
Color MDStudio::lightSkyBlueColor = RGB(135, 206, 250);
Color MDStudio::deepSkyBlueColor = RGB(0, 191, 255);
Color MDStudio::dodgerBlueColor = RGB(30, 144, 255);
Color MDStudio::cornflowerBlueColor = RGB(100, 149, 237);
Color MDStudio::steelBlueColor = RGB(70, 130, 180);
Color MDStudio::royalBlueColor = RGB(65, 105, 225);
Color MDStudio::blueColor = RGB(0, 0, 255);
Color MDStudio::mediumBlueColor = RGB(0, 0, 205);
Color MDStudio::darkBlueColor = RGB(0, 0, 139);
Color MDStudio::navyColor = RGB(0, 0, 128);
Color MDStudio::midnightBlueColor = RGB(25, 25, 112);

// Purple/violet/magenta colors
Color MDStudio::lavenderColor = RGB(230, 230, 250);
Color MDStudio::thistleColor = RGB(216, 191, 216);
Color MDStudio::plumColor = RGB(221, 160, 221);
Color MDStudio::violetColor = RGB(238, 130, 238);
Color MDStudio::orchidColor = RGB(218, 112, 214);
Color MDStudio::fuchsiaColor = RGB(255, 0, 255);
Color MDStudio::magentaColor = RGB(255, 0, 255);
Color MDStudio::mediumOrchidColor = RGB(186, 85, 211);
Color MDStudio::mediumPurpleColor = RGB(147, 112, 219);
Color MDStudio::blueVioletColor = RGB(138, 43, 226);
Color MDStudio::darkVioletColor = RGB(148, 0, 211);
Color MDStudio::darkOrchidColor = RGB(153, 50, 204);
Color MDStudio::darkMagentaColor = RGB(139, 0, 139);
Color MDStudio::purpleColor = RGB(128, 0, 128);
Color MDStudio::indigoColor = RGB(75, 0, 130);
Color MDStudio::darkSlateBlueColor = RGB(72, 61, 139);
Color MDStudio::slateBlueColor = RGB(106, 90, 205);
Color MDStudio::mediumSlateBlueColor = RGB(123, 104, 238);

// White colors
Color MDStudio::whiteColor = RGB(255, 255, 255);
Color MDStudio::snowColor = RGB(255, 250, 250);
Color MDStudio::honeydewColor = RGB(240, 255, 240);
Color MDStudio::mintCreamColor = RGB(245, 255, 250);
Color MDStudio::azureColor = RGB(240, 255, 255);
Color MDStudio::aliceBlueColor = RGB(240, 248, 255);
Color MDStudio::ghostWhiteColor = RGB(248, 248, 255);
Color MDStudio::whiteSmokeColor = RGB(245, 245, 245);
Color MDStudio::seashellColor = RGB(255, 245, 238);
Color MDStudio::beigeColor = RGB(245, 245, 220);
Color MDStudio::oldLaceColor = RGB(253, 245, 230);
Color MDStudio::floralWhiteColor = RGB(255, 250, 240);
Color MDStudio::ivoryColor = RGB(255, 255, 240);
Color MDStudio::antiqueWhiteColor = RGB(250, 235, 215);
Color MDStudio::linenColor = RGB(250, 240, 230);
Color MDStudio::lavenderBlushColor = RGB(255, 240, 245);
Color MDStudio::mistyRoseColor = RGB(255, 228, 225);

// Non-standard colors
Color MDStudio::veryDimGrayColor = RGB(64, 64, 64);

// System colors
Color MDStudio::systemButtonBorderColor = lightGrayColor;

// ---------------------------------------------------------------------------------------------------------------------
Color MDStudio::makeColor(float red, float green, float blue, float alpha) {
    Color color;
    color.red = red;
    color.green = green;
    color.blue = blue;
    color.alpha = alpha;
    return color;
}
