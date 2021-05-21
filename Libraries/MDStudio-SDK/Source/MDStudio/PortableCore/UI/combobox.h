//
//  combobox.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-15.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <memory>

#include "boxview.h"
#include "button.h"
#include "control.h"
#include "listview.h"
#include "scrollview.h"
#include "searchfield.h"
#include "window.h"

namespace MDStudio {

class ComboBox : public Control {
   public:
    typedef std::function<unsigned int(ComboBox* sender)> nbRowsFnType;
    typedef std::function<std::shared_ptr<View>(ComboBox* sender, int row)> viewForRowFnType;
    typedef std::function<void(ComboBox* sender, int row)> didSelectRowFnType;
    typedef std::function<void(ComboBox* sender, int row)> didDeselectRowFnType;
    typedef std::function<void(ComboBox* sender, int row)> didHoverRowFnType;
    typedef std::function<void(ComboBox* sender, int row)> didConfirmRowSelectionFnType;
    typedef std::function<void(ComboBox* sender, bool state)> didSetFocusStateFnType;
    typedef std::function<void(ComboBox* sender, std::string text)> SearchFieldTextChangingFnType;

    typedef enum { AbovePosition, BelowPosition } ListPositionEnumType;

   private:
    nbRowsFnType _nbRowsFn;
    viewForRowFnType _viewForRowFn;
    didSelectRowFnType _didSelectRowFn;
    didDeselectRowFnType _didDeselectRowFn;
    didHoverRowFnType _didHoverRowFn;
    didConfirmRowSelectionFnType _didConfirmRowSelectionFn;
    didSetFocusStateFnType _didSetFocusStateFn;
    SearchFieldTextChangingFnType _searchFieldTextChangingFn;

    std::shared_ptr<Button> _button;
    std::shared_ptr<BoxView> _boxView;
    std::shared_ptr<ListView> _listView;
    std::shared_ptr<ScrollView> _listScrollView;
    std::shared_ptr<SearchField> _searchField;

    ListPositionEnumType _listPosition;

    View *_listContainerView, *_resolvedListContainerView;
    float _maxHeight;

    Window* _window;

    Rect _windowFrame;

    void buttonClicked(View* sender);
    void updateListScrollViewFrame();

    unsigned int listViewNbRows(ListView* sender);
    std::shared_ptr<View> listViewViewForRow(ListView* sender, int row);
    void listViewDidSelectRow(ListView* sender, int row);
    void listViewDidDeselectRow(ListView* sender, int row);
    void listViewDidHoverRow(ListView* sender, int row);
    void listViewDidConfirmRowSelection(ListView* sender, int row);
    void listViewDidSetFocusState(ListView* sender, bool state);
    void searchFieldTextChanging(SearchField* sender, std::string text);
    void willRemoveFromSuperview() override;

   public:
    ComboBox(std::string name, void* owner, float rowHeight = 18.0f);
    ~ComboBox();

    void setFrame(Rect rect) override;

    void setListPosition(ListPositionEnumType listPosition);
    ListPositionEnumType listPosition() { return _listPosition; }

    void setListContainerView(View* listContainerView) { _listContainerView = listContainerView; }
    View* listContainerView() { return _listContainerView; }
    void setMaxHeight(float maxHeight) { _maxHeight = maxHeight; }
    float maxHeight() { return _maxHeight; }

    void open();
    void close();
    bool isOpen();

    void setFont(MultiDPIFont* font) { _button->setFont(font); }
    void setSearchFieldFont(MultiDPIFont* font) { _searchField->setFont(font); }

    void setSearchFieldIsVisible(bool isVisible) { _searchField->setIsVisible(isVisible); }

    void setIsHorizScrollBarVisible(bool isVisible) { _listScrollView->setIsHorizScrollBarVisible(isVisible); }
    void setIsVertScrollBarVisible(bool isVisible) { _listScrollView->setIsVertScrollBarVisible(isVisible); }

    void setTitle(std::string title) { _button->setTitle(title); }
    std::string title() { return _button->title(); }

    void setSelectedRow(int row, bool isDelegateNotified = true) { _listView->setSelectedRow(row, isDelegateNotified); }
    int selectedRow() { return _listView->selectedRow(); }

    void reload() { _listView->reload(); }

    void setIsEnabled(bool isEnabled) { _button->setIsEnabled(isEnabled); }

    void scrollToVisibleRectV(Rect rect) { _listScrollView->scrollToVisibleRectV(rect); }

    const std::vector<std::shared_ptr<View>>& rowViews() { return _listView->subviews(); }

    void setIsCapturing(bool isCapturing) { _listView->setIsCapturing(isCapturing); }
    void setIsScrollingWithDrag(bool isScrollingWithDrag) {
        _listScrollView->setIsScrollingWithDrag(isScrollingWithDrag);
    }

    void setNbRowsFn(nbRowsFnType nbRowsFn) { _nbRowsFn = nbRowsFn; }
    void setViewForRowFn(viewForRowFnType viewForRowFn) { _viewForRowFn = viewForRowFn; }
    void setDidSelectRowFn(didSelectRowFnType didSelectRowFn) { _didSelectRowFn = didSelectRowFn; }
    void setDidDeselectRowFn(didDeselectRowFnType didDeselectRowFn) { _didDeselectRowFn = didDeselectRowFn; }
    void setDidHoverRowFn(didHoverRowFnType didHoverRowFn) { _didHoverRowFn = didHoverRowFn; }
    void setDidConfirmRowSelectionFn(didConfirmRowSelectionFnType didConfirmRowSelectionFn) {
        _didConfirmRowSelectionFn = didConfirmRowSelectionFn;
    }
    void setDidSetFocusStateFn(didSetFocusStateFnType didSetFocusStateFn) { _didSetFocusStateFn = didSetFocusStateFn; }
    void setSearchFieldTextChangingFn(SearchFieldTextChangingFnType searchFieldTextChangingFn) {
        _searchFieldTextChangingFn = searchFieldTextChangingFn;
    }
};

}  // namespace MDStudio

#endif  // COMBOBOX_H
