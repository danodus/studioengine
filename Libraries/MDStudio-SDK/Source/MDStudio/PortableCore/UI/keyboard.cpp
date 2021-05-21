//
//  keyboard.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-26.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "keyboard.h"

#include <platform.h>

#include "responderchain.h"

using namespace MDStudio;

// Mapping white/black key index -> pitch in octave
const int Keyboard::whiteKeyToPitchMapping[7] = {0, 2, 4, 5, 7, 9, 11};
const int Keyboard::blackKeyToPitchMapping[5] = {1, 3, 6, 8, 10};

// Drum kit labels (GM)
const std::map<int, std::string> Keyboard::drumKitLabels = {
    {35, "Bass Drum 2"},    {36, "Bass Drum 1"},     {37, "Rimshot"},        {38, "Snare Drum 1"},
    {39, "Hand Clap"},      {40, "Snare Drum 2"},    {41, "Low Tom 2"},      {42, "Closed Hi-hat"},
    {43, "Low Tom 1"},      {44, "Pedal Hi-hat"},    {45, "Mid Tom 2"},      {46, "Open Hi-hat"},
    {47, "Mid Tom 1"},      {48, "High Tom 2"},      {49, "Crash Cymbal 1"}, {50, "High Tom 1"},
    {51, "Ride Cymbal 1"},  {52, "Chinese Cymbal"},  {53, "Ride Bell"},      {54, "Tambourine"},
    {55, "Splash Cymbal"},  {56, "Cowbell"},         {57, "Crash Cymbal 2"}, {58, "Vibra Slap"},
    {59, "Ride Cymbal 2"},  {60, "High Bongo"},      {61, "Low Bongo"},      {62, "Mute High Conga"},
    {63, "Opn High Conga"}, {64, "Low Conga"},       {65, "High Timbale"},   {66, "Low Timbale"},
    {67, "High Agogô"},     {68, "Low Agogô"},       {69, "Cabasa"},         {70, "Maracas"},
    {71, "Short Whistle"},  {72, "Long Whistle"},    {73, "Short Güiro"},    {74, "Long Güiro"},
    {75, "Claves"},         {76, "High Wood Block"}, {77, "Low Wood Block"}, {78, "Mute Cuíca"},
    {79, "Open Cuíca"},     {80, "Mute Triangle"},   {81, "Open Triangle"}};

const float Keyboard::drumKitWhiteKeyLabelOffsets[7] = {-4.0f, 0.0f, 4.0f, -4.0f, -2.0f, 2.0f, 4.0f};

// ---------------------------------------------------------------------------------------------------------------------
Keyboard::Keyboard(std::string name, void* owner) : Control(name, owner) {
    _keyPressedFn = nullptr;
    _keyReleasedFn = nullptr;
    _scrollFn = nullptr;
    _highlightedKey = -1;
    _isCaptured = false;
    _isDrumKit = false;
    _currentChannel = 0;

    memset(_keyStates, 0, sizeof(_keyStates));

    std::string path = Platform::sharedInstance()->resourcesPath() + "/OpenSans-Semibold.ttf";
    _font = new MultiDPIFont(7, path);
}

// ---------------------------------------------------------------------------------------------------------------------
Keyboard::~Keyboard() { delete _font; }

// ---------------------------------------------------------------------------------------------------------------------
void Keyboard::setKeyPressedState(int channel, int pitch, bool state) {
    if (pitch >= 12 && pitch < (96 + 12)) {
        _keyStates[channel][pitch - 12] = state;
        setDirty();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Keyboard::resetKeyPressedStates() {
    memset(_keyStates, 0, sizeof(_keyStates));
    setDirty();
}
