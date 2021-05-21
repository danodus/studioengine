//
//  programcomboboxcontroller.h
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-01-30.
//  Copyright © 2016 Daniel Cliche. All rights reserved.
//

#ifndef PROGRAMCOMBOBOXCONTROLLER_H
#define PROGRAMCOMBOBOXCONTROLLER_H

#include <vector>
#include <string>

#include <studio.h>
#include <combobox.h>
#include <listitemview.h>

class ProgramComboBoxController {
    
public:
    
    typedef std::function<void(ProgramComboBoxController *sender,  MDStudio::Preset preset)> DidSelectPresetFnType;
    typedef std::function<void(ProgramComboBoxController *sender,  MDStudio::Preset preset)> DidConfirmPresetSelectionFnType;
    
private:
    
    MDStudio::Studio *_studio;
    MDStudio::ComboBox *_comboBox;
    
    MDStudio::MultiDPIFont *_font;
    
    std::vector<std::string> _presetNames;
    std::vector<MDStudio::Preset> _presets;
    
    std::string _searchText;
    
    std::string presetName(const MDStudio::Preset &preset);
    
    unsigned int nbRows(MDStudio::ComboBox *sender);
    std::shared_ptr<MDStudio::View> viewForRow(MDStudio::ComboBox* sender, int row);
    void didSelectRow(MDStudio::ComboBox* sender, int row);
    void didHoverRow(MDStudio::ComboBox* sender, int row);
    void didConfirmRowSelection(MDStudio::ComboBox* sender, int row);
    void didSetFocusState(MDStudio::ComboBox* sender, bool state);
    void searchFieldTextChanging(MDStudio::ComboBox *sender, std::string text);
    
    void updatePresets();

    DidSelectPresetFnType _didSelectPresetFn;
    DidConfirmPresetSelectionFnType _didConfirmPresetSelectionFn;
    
public:
    
    ProgramComboBoxController(MDStudio::ComboBox *comboBox, MDStudio::Studio *studio, MDStudio::MultiDPIFont *font);
    
    void setPreset(const MDStudio::Preset *preset);
    
    void setDidSelectPresetFn(DidSelectPresetFnType didSelectPresetFn) { _didSelectPresetFn = didSelectPresetFn; }
    void setDidConfirmPresetSelectionFn(DidConfirmPresetSelectionFnType didConfirmPresetSelectionFn) { _didConfirmPresetSelectionFn = didConfirmPresetSelectionFn; }

};


#endif // PROGRAMCOMBOBOXCONTROLLER_H
