//
//  programcomboboxcontroller.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-01-30.
//  Copyright © 2016 Daniel Cliche. All rights reserved.
//

#include "programcomboboxcontroller.h"

#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>

// ---------------------------------------------------------------------------------------------------------------------
ProgramComboBoxController::ProgramComboBoxController(MDStudio::ComboBox *comboBox, MDStudio::Studio *studio, MDStudio::MultiDPIFont *font) : _comboBox(comboBox), _studio(studio), _font(font)
{
    _didSelectPresetFn = nullptr;
    _didConfirmPresetSelectionFn = nullptr;

    updatePresets();

    using namespace std::placeholders;
    
    comboBox->setNbRowsFn(std::bind(&ProgramComboBoxController::nbRows, this, _1));
    comboBox->setViewForRowFn(std::bind(&ProgramComboBoxController::viewForRow, this, _1, _2));
    comboBox->setDidSelectRowFn(std::bind(&ProgramComboBoxController::didSelectRow, this, _1, _2));
    comboBox->setDidHoverRowFn(std::bind(&ProgramComboBoxController::didHoverRow, this, _1, _2));
    comboBox->setDidConfirmRowSelectionFn(std::bind(&ProgramComboBoxController::didConfirmRowSelection, this, _1, _2));
    comboBox->setDidSetFocusStateFn(std::bind(&ProgramComboBoxController::didSetFocusState, this, _1, _2));
    comboBox->reload();
    
    comboBox->setSearchFieldTextChangingFn(std::bind(&ProgramComboBoxController::searchFieldTextChanging, this, _1, _2));
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int ProgramComboBoxController::nbRows(MDStudio::ComboBox *sender)
{
    return static_cast<unsigned int>(_presetNames.size());
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<MDStudio::View> ProgramComboBoxController::viewForRow(MDStudio::ComboBox* sender, int row)
{
    std::string title = _presetNames[row];
    
    std::shared_ptr<MDStudio::ListItemView> listItemView = std::shared_ptr<MDStudio::ListItemView>(new MDStudio::ListItemView("listItemView" + std::to_string(row), nullptr, title));
    listItemView->setFont(_font);
    return listItemView;
}

// ---------------------------------------------------------------------------------------------------------------------
void ProgramComboBoxController::didSelectRow(MDStudio::ComboBox* sender, int row)
{
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setIsHighlighted(i == row);
    }
    
    if (_didSelectPresetFn)
        _didSelectPresetFn(this, _presets[row]);

    
    if (row >= 0 && row < _presets.size()) {
        MDStudio::Preset preset = _presets[row];

        std::shared_ptr<MDStudio::View> view = sender->rowViews()[row];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        _comboBox->scrollToVisibleRectV(listItemView->rect());
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ProgramComboBoxController::didHoverRow(MDStudio::ComboBox* sender, int row)
{
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setIsHovering(i == row);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ProgramComboBoxController::didConfirmRowSelection(MDStudio::ComboBox* sender, int row)
{
    _comboBox->close();
    
    if (row >= 0 && _didConfirmPresetSelectionFn)
        _didConfirmPresetSelectionFn(this, _presets[row]);
}

// ---------------------------------------------------------------------------------------------------------------------
void ProgramComboBoxController::didSetFocusState(MDStudio::ComboBox* sender, bool state)
{
    for (unsigned int i = 0; i < sender->rowViews().size(); ++i) {
        std::shared_ptr<MDStudio::View> view = sender->rowViews()[i];
        std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
        listItemView->setFocusState(state);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
std::string ProgramComboBoxController::presetName(const MDStudio::Preset &preset)
{
    std::stringstream s;
    if (preset._bank == 0)
        s << std::setfill('0') << std::setw(3) << preset._number << std::string(" - ");
    s << preset._name;
    return s.str();
}

// ---------------------------------------------------------------------------------------------------------------------
void ProgramComboBoxController::setPreset(const MDStudio::Preset *preset)
{
    if (preset != nullptr) {

        // Check if the preset is already selected by comparing the name
        if (_comboBox->title() == presetName(*preset))
            return;
        
        _comboBox->setTitle(presetName(*preset));
        
        _comboBox->setSelectedRow(-1, false);
        
        for (unsigned int i = 0; i < _comboBox->rowViews().size(); ++i) {
            std::shared_ptr<MDStudio::View> view = _comboBox->rowViews()[i];
            std::shared_ptr<MDStudio::ListItemView> listItemView = (std::static_pointer_cast<MDStudio::ListItemView>)(view);
            if (listItemView->title() == presetName(*preset)) {
                _comboBox->setSelectedRow(i, false);
                listItemView->setIsHighlighted(true);
                _comboBox->scrollToVisibleRectV(listItemView->rect());
            } else {
                listItemView->setIsHighlighted(false);
            }
        }
        
    } else {
        _comboBox->setTitle("---");
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ProgramComboBoxController::updatePresets()
{
    _presetNames.clear();
    _presets.clear();
    
    // Create a list of preset names
    std::vector<MDStudio::Preset> presets = _studio->presets();
    for (auto preset : presets) {
        if (preset._bank == 0) {
            std::string name = preset._name;
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            std::string searchText = _searchText;
            std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::tolower);
            if (searchText.empty() || (name.find(searchText) != std::string::npos)) {
                _presetNames.push_back(presetName(preset));
                _presets.push_back(preset);
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ProgramComboBoxController::searchFieldTextChanging(MDStudio::ComboBox *sender, std::string text)
{
    if (text != _searchText) {
        _searchText = text;
        _comboBox->close();
        updatePresets();
        _comboBox->reload();
    
        if (_comboBox->rowViews().size() > 0)
            _comboBox->setSelectedRow(0);
        
        _comboBox->open();
    }
}

