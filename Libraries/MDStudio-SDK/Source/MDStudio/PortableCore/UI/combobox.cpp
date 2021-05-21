//
//  combobox.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-15.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "combobox.h"

#include <platform.h>

#include <algorithm>

#include "listitemview.h"

using namespace MDStudio;

static std::vector<ComboBox*> openedComboBoxes;

// ---------------------------------------------------------------------------------------------------------------------
ComboBox::ComboBox(std::string name, void* owner, float rowHeight) : Control(name, owner) {
    using namespace std::placeholders;

    _nbRowsFn = nullptr;
    _viewForRowFn = nullptr;
    _didSelectRowFn = nullptr;
    _didDeselectRowFn = nullptr;
    _didHoverRowFn = nullptr;
    _didConfirmRowSelectionFn = nullptr;
    _didSetFocusStateFn = nullptr;
    _searchFieldTextChangingFn = nullptr;

    _listPosition = AbovePosition;
    _listContainerView = nullptr;
    _resolvedListContainerView = nullptr;
    _maxHeight = 400.0f;
    _window = nullptr;

    _button = std::shared_ptr<Button>(new Button("button", this, ""));
    _button->setType(Button::ComboBoxUpButtonType);
    _button->setClickedFn(std::bind(&ComboBox::buttonClicked, this, _1));
    addSubview(_button);

    _boxView = std::shared_ptr<BoxView>(new BoxView("boxView", this));

    _listView = std::shared_ptr<ListView>(new ListView("listView", this, rowHeight));
    _listView->setNbRowsFn(std::bind(&ComboBox::listViewNbRows, this, _1));
    _listView->setViewForRowFn(std::bind(&ComboBox::listViewViewForRow, this, _1, _2));
    _listView->setDidSelectRowFn(std::bind(&ComboBox::listViewDidSelectRow, this, _1, _2));
    _listView->setDidDeselectRowFn(std::bind(&ComboBox::listViewDidDeselectRow, this, _1, _2));
    _listView->setDidHoverRowFn(std::bind(&ComboBox::listViewDidHoverRow, this, _1, _2));
    _listView->setDidConfirmRowSelectionFn(std::bind(&ComboBox::listViewDidConfirmRowSelection, this, _1, _2));
    _listView->setDidSetFocusStateFn(std::bind(&ComboBox::listViewDidSetFocusState, this, _1, _2));
    _listView->setHandleEventFn([=](View* sender, const UIEvent* event) -> bool {
        if (_listView->hasFocus() && event->type == KEY_UIEVENT) {
            if (event->key == KEY_ESCAPE) {
                MDStudio::Platform::sharedInstance()->invoke([=] { close(); });
                return true;
            } else if (event->key != KEY_DOWN && event->key != KEY_UP && event->key != KEY_ENTER) {
                if (_searchField->isVisible()) {
                    _searchField->startEdition();
                    _searchField->textField()->textView()->handleEvent(event);
                    return true;
                }
            }
        }
        return false;
    });

    _searchField = std::make_shared<SearchField>("searchField", this);
    _searchField->setTextChangingFn(std::bind(&ComboBox::searchFieldTextChanging, this, _1, _2));
    _searchField->setCornerRadius(0.0f);

    _searchField->setIsVisible(false);

    _searchField->textField()->textView()->setHandleEventFn([=](View* sender, const UIEvent* event) -> bool {
        if (_searchField->textField()->textView()->hasFocus() && event->type == KEY_UIEVENT)
            if (event->key == KEY_DOWN || event->key == KEY_UP || event->key == KEY_ESCAPE || event->key == KEY_ENTER)
                _listView->captureFocus();
        return false;
    });

    _listScrollView = std::shared_ptr<ScrollView>(new ScrollView("scrollView", this, _listView, true));

    _listScrollView->setIsVisible(false);
}

// ---------------------------------------------------------------------------------------------------------------------
ComboBox::~ComboBox() {
    if (isOpen()) close();

    removeSubview(_button);

    if (_window) {
        close();
        delete _window;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ComboBox::buttonClicked(View* sender) {
    ComboBox* comboBox = (ComboBox*)(sender->owner());

    if (!comboBox->isOpen()) {
        comboBox->open();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ComboBox::updateListScrollViewFrame() {
    Rect r;

    float horizScrollbarThickness =
        _listScrollView->isHorizScrollBarVisible() ? SCROLL_VIEW_SCROLL_BAR_THICKNESS : 0.0f;
    float listHeight = std::min(_maxHeight, _listView->contentHeight() + horizScrollbarThickness + 2.0f);

    View* listContainerView = _listContainerView ? _listContainerView : topView();

    float searchFieldHeight = _searchField->isVisible() ? 20.0f : 0.0f;

    if (_listPosition == AbovePosition) {
        r = makeRect(_button->rect().origin.x, _button->rect().origin.y + rect().size.height, rect().size.width,
                     listHeight + searchFieldHeight);
    } else {
        r = makeRect(_button->rect().origin.x, _button->rect().origin.y - listHeight - searchFieldHeight,
                     rect().size.width, listHeight + searchFieldHeight);
    }

    r.origin.x += _button->resolvedOffset().x;
    r.origin.y += _button->resolvedOffset().y;

    r.origin.x -= listContainerView->rect().origin.x;
    r.origin.y -= listContainerView->rect().origin.y;

    _windowFrame = r;

    r.origin.x = 0.0f;
    r.origin.y = searchFieldHeight;
    r.size.height -= searchFieldHeight;

    _boxView->setFrame(r);
    _listScrollView->setFrame(makeInsetRect(r, 1, 1));

    if (_searchField->isVisible()) {
        r.origin.y = 0.0f;
        r.size.height = searchFieldHeight;
        _searchField->setFrame(r);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ComboBox::setFrame(Rect aRect) {
    Control::setFrame(aRect);

    _button->setFrame(bounds());

    if (_window) updateListScrollViewFrame();
}

// ---------------------------------------------------------------------------------------------------------------------
void ComboBox::open() {
    _resolvedListContainerView = _listContainerView ? _listContainerView : topView();

    if (!_window) {
        // Disable the button
        _button->setIsEnabled(false);

        // Close the opened combo boxes
        for (auto comboBox : openedComboBoxes) comboBox->close();

        float horizScrollbarThickness =
            _listScrollView->isHorizScrollBarVisible() ? SCROLL_VIEW_SCROLL_BAR_THICKNESS : 0.0f;

        _listScrollView->setContentSize(
            makeSize(rect().size.width - horizScrollbarThickness - 2.0f, _listView->contentHeight()));

        // We update the frame in order to keep the list view at the proper position
        updateListScrollViewFrame();

        auto windowRect = _windowFrame;
        if (_listContainerView) {
            auto containerViewOrigin = _listContainerView->rect().origin;
            windowRect.origin.x += containerViewOrigin.x;
            windowRect.origin.y += containerViewOrigin.y;
        }

        _window = new Window();
        _window->setDidResignKeyWindowFn([&](Window* sender) { close(); });

        _window->contentView()->addSubview(_boxView);
        _window->contentView()->addSubview(_listScrollView);
        if (_searchField->isVisible()) _window->contentView()->addSubview(_searchField);
        _listScrollView->setIsVisible(true);

        _window->open(windowRect, true);

        // We update again the frame in order to adjust with the opened window
        updateListScrollViewFrame();

        if (_searchField->isVisible()) {
            _searchField->startEdition();
        } else {
            _listView->captureFocus();
        }

        openedComboBoxes.push_back(this);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ComboBox::close() {
    if (_window) {
        // Make sure that we do not close twice
        _window->setDidResignKeyWindowFn(nullptr);

        _listView->releaseFocus();
        _window->contentView()->removeSubview(_listScrollView);
        _window->contentView()->removeSubview(_boxView);
        if (_searchField->isVisible()) {
            _searchField->stopEdition();
            _window->contentView()->removeSubview(_searchField);
        }
        _window->close();
        delete _window;
        _window = nullptr;

        openedComboBoxes.erase(std::remove(openedComboBoxes.begin(), openedComboBoxes.end(), this),
                               openedComboBoxes.end());

        // Enable the button
        _button->setIsEnabled(true);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
bool ComboBox::isOpen() { return _window != nullptr; }

// ---------------------------------------------------------------------------------------------------------------------
void ComboBox::setListPosition(ListPositionEnumType listPosition) {
    _listPosition = listPosition;
    _button->setType(listPosition == AbovePosition ? Button::ComboBoxUpButtonType : Button::ComboBoxDownButtonType);
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int ComboBox::listViewNbRows(ListView* sender) {
    if (!_nbRowsFn) return 0;

    return _nbRowsFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<View> ComboBox::listViewViewForRow(ListView* sender, int row) {
    if (!_viewForRowFn) return nullptr;

    return _viewForRowFn(this, row);
}

// ---------------------------------------------------------------------------------------------------------------------
void ComboBox::listViewDidSelectRow(ListView* sender, int row) {
    if (_didSelectRowFn) _didSelectRowFn(this, row);
}

// ---------------------------------------------------------------------------------------------------------------------
void ComboBox::listViewDidDeselectRow(ListView* sender, int row) {
    if (_didDeselectRowFn) _didDeselectRowFn(this, row);
}

// ---------------------------------------------------------------------------------------------------------------------
void ComboBox::listViewDidHoverRow(ListView* sender, int row) {
    if (_didHoverRowFn) _didHoverRowFn(this, row);
}

// ---------------------------------------------------------------------------------------------------------------------
void ComboBox::listViewDidConfirmRowSelection(ListView* sender, int row) {
    if (_didConfirmRowSelectionFn) _didConfirmRowSelectionFn(this, row);
}

// ---------------------------------------------------------------------------------------------------------------------
void ComboBox::listViewDidSetFocusState(ListView* sender, bool state) {
    if (_didSetFocusStateFn) _didSetFocusStateFn(this, state);
}

// ---------------------------------------------------------------------------------------------------------------------
void ComboBox::searchFieldTextChanging(SearchField* sender, std::string text) {
    if (_searchFieldTextChangingFn) _searchFieldTextChangingFn(this, text);
}

// ---------------------------------------------------------------------------------------------------------------------
void ComboBox::willRemoveFromSuperview() {
    if (isOpen()) close();
}