//
//  uiscriptmodule.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2018-04-07.
//  Copyright © 2018-2021 Daniel Cliche. All rights reserved.
//

#include "uiscriptmodule.h"

// UI

#include "animation.h"
#include "boxview.h"
#include "button.h"
#include "combobox.h"
#include "draw.h"
#include "drawcontext.h"
#include "imageview.h"
#include "keyboardh.h"
#include "keyboardv.h"
#include "labelview.h"
#include "listitemview.h"
#include "listview.h"
#include "menu.h"
#include "menubar.h"
#include "platform.h"
#include "progressindicator.h"
#include "scrollview.h"
#include "segmentedcontrol.h"
#include "slider.h"
#include "splitviewh.h"
#include "splitviewmultih.h"
#include "stepper.h"
#include "svgparsers.h"
#include "svgview.h"
#include "tableview.h"
#include "textfield.h"
#include "textview.h"
#include "treeview.h"
#include "webview.h"
#include "window.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
inline Point getPoint(lua_State* L, int index = -2) {
    lua_pushstring(L, "x");
    lua_gettable(L, index);
    auto x = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "y");
    lua_gettable(L, index);
    auto y = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    return makePoint(static_cast<float>(x), static_cast<float>(y));
}

// ---------------------------------------------------------------------------------------------------------------------
inline Size getSize(lua_State* L, int index = -2) {
    lua_pushstring(L, "width");
    lua_gettable(L, index);
    auto width = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "height");
    lua_gettable(L, index);
    auto height = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    return makeSize(static_cast<float>(width), static_cast<float>(height));
}

// ---------------------------------------------------------------------------------------------------------------------
inline Rect getRect(lua_State* L, int index = -2) {
    lua_pushstring(L, "origin");
    lua_gettable(L, index);

    if (!lua_istable(L, -1)) luaL_error(L, "Invalid origin");

    auto point = getPoint(L);

    // Pop origin
    lua_pop(L, 1);

    lua_pushstring(L, "size");
    lua_gettable(L, index);

    if (!lua_istable(L, -1)) luaL_error(L, "Invalid size");

    auto size = getSize(L);

    // Pop size
    lua_pop(L, 1);

    return makeRect(point.x, point.y, size.width, size.height);
}

// ---------------------------------------------------------------------------------------------------------------------
inline Color getColor(lua_State* L, int index = -2) {
    lua_pushstring(L, "red");
    lua_gettable(L, index);
    auto red = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "green");
    lua_gettable(L, index);
    auto green = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "blue");
    lua_gettable(L, index);
    auto blue = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "alpha");
    lua_gettable(L, index);
    auto alpha = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    return makeColor(static_cast<float>(red), static_cast<float>(green), static_cast<float>(blue),
                     static_cast<float>(alpha));
}

// ---------------------------------------------------------------------------------------------------------------------
// View
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
static int createDrawContext(lua_State* L) {
    std::shared_ptr<DrawContext> drawContext(new DrawContext());
    registerElement<DrawContext>(L, drawContext);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createView(lua_State* L) {
    // Stack: name

    const char* name = luaL_checkstring(L, 1);

    std::shared_ptr<View> view(new View(name, nullptr));
    registerElement<View>(L, view);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int addSubview(lua_State* L) {
    // Stack: view, childView

    auto view = getElement<View>(L);
    auto p = lua_touserdata(L, 2);
    if (p != NULL) {
        auto childView = *((std::shared_ptr<View>*)p);
        view->addSubview(childView);
    }

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setLayoutFn(lua_State* L) {
    // Stack: view, callback

    // Store the reference to the Lua function in a variable to be used later
    int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

    // Stack: view

    auto view = getElement<View>(L);
    lua_getglobal(L, "script");
    auto script = (MDStudio::Script*)lua_touserdata(L, -1);

    view->setLayoutFn([=](View* sender, Rect rect) {
        // Push the callback onto the stack using the Lua reference we stored in the registry
        lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
        if (sender->owner()) {
            int* ownerRef = (int*)sender->owner();
            lua_rawgeti(L, LUA_REGISTRYINDEX, *ownerRef);
        }
        registerElement<View>(L, std::shared_ptr<View>(sender, BypassDeleter<View>()));
        if (lua_pcall(L, sender->owner() ? 2 : 1, 0, 0) != 0) {
            script->error(lua_tostring(L, -1));
        }
    });

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setDrawFn(lua_State* L) {
    // Stack: view, callback

    // Store the reference to the Lua function in a variable to be used later
    int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

    // Stack: view

    auto view = getElement<View>(L);
    lua_getglobal(L, "script");
    auto script = (MDStudio::Script*)lua_touserdata(L, -1);

    view->setDrawFn([=](View* sender) {
        // Push the callback onto the stack using the Lua reference we stored in the registry
        lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
        if (sender->owner()) {
            int* ownerRef = (int*)sender->owner();
            lua_rawgeti(L, LUA_REGISTRYINDEX, *ownerRef);
        }
        registerElement<View>(L, std::shared_ptr<View>(sender, BypassDeleter<View>()));
        if (lua_pcall(L, sender->owner() ? 2 : 1, 0, 0) != 0) {
            // Since we are currently drawing, we report the error later on in order to terminate the script when the
            // drawing is finished
            MDStudio::Platform::sharedInstance()->invoke([=] { script->error(lua_tostring(L, -1)); });
        }
    });

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setHandleEventFn(lua_State* L) {
    // Stack: view, callback

    // Store the reference to the Lua function in a variable to be used later
    int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

    // Stack: view

    auto view = getElement<View>(L);
    lua_getglobal(L, "script");
    auto script = (MDStudio::Script*)lua_touserdata(L, -1);

    view->setHandleEventFn([=](View* sender, const UIEvent* event) -> bool {
        // Push the callback onto the stack using the Lua reference we stored in the registry
        lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
        if (sender->owner()) {
            int* ownerRef = (int*)sender->owner();
            lua_rawgeti(L, LUA_REGISTRYINDEX, *ownerRef);
        }
        registerElement<View>(L, std::shared_ptr<View>(sender, BypassDeleter<View>()));

        lua_newtable(L);

        lua_pushinteger(L, event->type);
        lua_setfield(L, -2, "type");

        lua_newtable(L);

        lua_pushnumber(L, event->pt.x);
        lua_setfield(L, -2, "x");

        lua_pushnumber(L, event->pt.y);
        lua_setfield(L, -2, "y");

        lua_setfield(L, -2, "pt");

        lua_pushinteger(L, event->key);
        lua_setfield(L, -2, "key");

        lua_pushstring(L, event->characters.c_str());
        lua_setfield(L, -2, "characters");

        lua_pushboolean(L, event->isARepeat ? 1 : 0);
        lua_setfield(L, -2, "isARepeat");

        lua_pushnumber(L, event->deltaX);
        lua_setfield(L, -2, "deltaX");

        lua_pushnumber(L, event->deltaY);
        lua_setfield(L, -2, "deltaY");

        lua_pushnumber(L, event->deltaZ);
        lua_setfield(L, -2, "deltaZ");

        lua_pushinteger(L, event->phase);
        lua_setfield(L, -2, "phase");

        lua_pushinteger(L, event->modifierFlags);
        lua_setfield(L, -2, "modifierFlags");

        bool ret = false;

        if (lua_pcall(L, sender->owner() ? 3 : 2, 1, 0) == 0) {
            if (!lua_isboolean(L, -1)) luaL_error(L, "Boolean return value expected");
            ret = lua_toboolean(L, -1) != 0;
        } else {
            script->error(lua_tostring(L, -1));
        }

        return ret;
    });

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setDisposeFn(lua_State* L) {
    // Stack: view, callback

    // Store the reference to the Lua function in a variable to be used later
    int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

    // Stack: view

    auto view = getElement<View>(L);
    lua_getglobal(L, "script");
    auto script = (MDStudio::Script*)lua_touserdata(L, -1);

    view->setDisposeFn([=](View* sender) {
        // Push the callback onto the stack using the Lua reference we stored in the registry
        lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
        if (sender->owner()) {
            int* ownerRef = (int*)sender->owner();
            lua_rawgeti(L, LUA_REGISTRYINDEX, *ownerRef);
        }
        registerElement<View>(L, std::shared_ptr<View>(sender, BypassDeleter<View>()));
        if (lua_pcall(L, sender->owner() ? 2 : 1, 0, 0) != 0) {
            script->error(lua_tostring(L, -1));
        }
    });

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setFrame(lua_State* L) {
    // Stack: view, frame (table)
    auto view = getElement<View>(L);

    if (!lua_istable(L, -1)) luaL_error(L, "Invalid frame");

    auto rect = getRect(L);

    view->setFrame(rect);

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int rect(lua_State* L) {
    // Stack: view

    auto view = getElement<View>(L);

    lua_newtable(L);

    lua_newtable(L);

    lua_pushnumber(L, view->rect().origin.x);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, view->rect().origin.y);
    lua_setfield(L, -2, "y");

    lua_setfield(L, -2, "origin");

    lua_newtable(L);

    lua_pushnumber(L, view->rect().size.width);
    lua_setfield(L, -2, "width");

    lua_pushnumber(L, view->rect().size.height);
    lua_setfield(L, -2, "height");

    lua_setfield(L, -2, "size");

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int resolvedClippedRect(lua_State* L) {
    // Stack: view

    auto view = getElement<View>(L);
    auto resolvedClippedRect = view->resolvedClippedRect();

    lua_newtable(L);

    lua_newtable(L);

    lua_pushnumber(L, resolvedClippedRect.origin.x);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, resolvedClippedRect.origin.y);
    lua_setfield(L, -2, "y");

    lua_setfield(L, -2, "origin");

    lua_newtable(L);

    lua_pushnumber(L, resolvedClippedRect.size.width);
    lua_setfield(L, -2, "width");

    lua_pushnumber(L, resolvedClippedRect.size.height);
    lua_setfield(L, -2, "height");

    lua_setfield(L, -2, "size");

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int frame(lua_State* L) {
    // Stack: view

    auto view = getElement<View>(L);

    lua_newtable(L);

    lua_newtable(L);

    lua_pushnumber(L, view->frame().origin.x);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, view->frame().origin.y);
    lua_setfield(L, -2, "y");

    lua_setfield(L, -2, "origin");

    lua_newtable(L);

    lua_pushnumber(L, view->frame().size.width);
    lua_setfield(L, -2, "width");

    lua_pushnumber(L, view->frame().size.height);
    lua_setfield(L, -2, "height");

    lua_setfield(L, -2, "size");

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int bounds(lua_State* L) {
    // Stack: view

    auto view = *((std::shared_ptr<View>*)lua_touserdata(L, 1));

    lua_newtable(L);

    lua_newtable(L);

    lua_pushnumber(L, view->bounds().origin.x);
    lua_setfield(L, -2, "x");

    lua_pushnumber(L, view->bounds().origin.y);
    lua_setfield(L, -2, "y");

    lua_setfield(L, -2, "origin");

    lua_newtable(L);

    lua_pushnumber(L, view->bounds().size.width);
    lua_setfield(L, -2, "width");

    lua_pushnumber(L, view->bounds().size.height);
    lua_setfield(L, -2, "height");

    lua_setfield(L, -2, "size");

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int subviews(lua_State* L) {
    // Stack: view

    auto view = getElement<View>(L);

    auto subviews = view->subviews();

    lua_newtable(L);

    lua_Integer i = 1;
    for (auto v : subviews) {
        lua_pushinteger(L, i);
        registerElement<View>(L, v);
        lua_settable(L, -3);
        ++i;
    }

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createScrollView(lua_State* L) {
    // Stack: name, contentView

    const char* name = luaL_checkstring(L, 1);
    std::shared_ptr<View> contentView = *((std::shared_ptr<View>*)lua_touserdata(L, 2));

    bool isContentToTop = false;
    if (lua_gettop(L) > 2) {
        if (!lua_isboolean(L, 3)) luaL_error(L, "Boolean expected");
        isContentToTop = lua_toboolean(L, 3);
    }
    std::shared_ptr<ScrollView> scrollView(new ScrollView(name, nullptr, contentView, isContentToTop));
    registerElement<ScrollView, View>(L, scrollView);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setScrollViewContentSize(lua_State* L) {
    // Stack: view, size

    std::shared_ptr<ScrollView> scrollView = getElement<ScrollView>(L);
    Size size = getSize(L, 2);

    scrollView->setContentSize(size);
    scrollView->setPos(makePoint(0.0f, size.height));
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setScrollViewPosChangedFn(lua_State* L) {
    // Stack: view, callback

    // Store the reference to the Lua function in a variable to be used later
    int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

    // Stack: viewIndex

    auto scrollView = getElement<ScrollView>(L);
    lua_getglobal(L, "script");
    auto script = (MDStudio::Script*)lua_touserdata(L, -1);
    scrollView->setPosChangedFn([=](ScrollView* sender, Point pos) {
        // Push the callback onto the stack using the Lua reference we stored in the registry
        lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
        registerElement<ScrollView, Control, View>(L, std::shared_ptr<ScrollView>(sender, BypassDeleter<ScrollView>()));

        lua_newtable(L);

        lua_pushnumber(L, pos.x);
        lua_setfield(L, -2, "x");

        lua_pushnumber(L, pos.y);
        lua_setfield(L, -2, "y");

        if (lua_pcall(L, 2, 0, 0) != 0) {
            script->error(lua_tostring(L, -1));
        }
    });

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createButton(lua_State* L) {
    // Stack: name, title

    const char* name = luaL_checkstring(L, 1);
    const char* title = luaL_checkstring(L, 2);

    std::shared_ptr<Image> image = nullptr;
    if (lua_gettop(L) > 2) image = getElement<Image>(L, 3);

    std::shared_ptr<Button> button(new Button(name, nullptr, title, image));
    registerElement<Button, Control, View>(L, button);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setButtonTitle(lua_State* L) {
    // Stack: view, title

    std::shared_ptr<Button> button = getElement<Button>(L);
    const char* title = lua_tostring(L, 2);

    button->setTitle(title);

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createLabelView(lua_State* L) {
    // Stack: name, title

    const char* name = luaL_checkstring(L, 1);
    const char* title = luaL_checkstring(L, 2);

    std::shared_ptr<LabelView> labelView(new LabelView(name, nullptr, title));
    registerElement<LabelView, View>(L, labelView);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int labelViewContentSize(lua_State* L) {
    // Stack: view

    std::shared_ptr<LabelView> labelView = getElement<LabelView>(L);

    lua_newtable(L);

    lua_pushnumber(L, labelView->contentSize().width);
    lua_setfield(L, -2, "width");

    lua_pushnumber(L, labelView->contentSize().height);
    lua_setfield(L, -2, "height");

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setLabelViewTextAlign(lua_State* L) {
    // Stack: view, align

    std::shared_ptr<LabelView> labelView = getElement<LabelView>(L);
    lua_Integer align = luaL_checkinteger(L, 2);

    labelView->setTextAlign(align == 1   ? LabelView::CenterTextAlign
                            : align == 2 ? LabelView::RightTextAlign
                                         : LabelView::LeftTextAlign);

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setLabelViewTitle(lua_State* L) {
    // Stack: view, title

    std::shared_ptr<LabelView> labelView = getElement<LabelView>(L);
    const char* title = lua_tostring(L, 2);

    labelView->setTitle(title);

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createTextView(lua_State* L) {
    // Stack: name

    const char* name = luaL_checkstring(L, 1);

    std::shared_ptr<TextView> textView(new TextView(name, nullptr));
    textView->setFont(SystemFonts::sharedInstance()->monoFont());
    registerElement<TextView, Control, View>(L, textView);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setTextViewText(lua_State* L) {
    // Stack: view, title

    std::shared_ptr<TextView> textView = getElement<TextView>(L);
    const char* title = luaL_checkstring(L, 2);

    textView->setText(title);

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int textViewContentSize(lua_State* L) {
    // Stack: view

    std::shared_ptr<TextView> textView = getElement<TextView>(L);

    lua_newtable(L);

    lua_pushnumber(L, textView->contentSize().width);
    lua_setfield(L, -2, "width");

    lua_pushnumber(L, textView->contentSize().height);
    lua_setfield(L, -2, "height");

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createTextField(lua_State* L) {
    // Stack: name

    const char* name = luaL_checkstring(L, 1);

    std::shared_ptr<TextField> textField(new TextField(name, nullptr));
    registerElement<TextField, Control, View>(L, textField);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setTextFieldTextDidChangeFn(lua_State* L) {
    // Stack: view, callback

    // Store the reference to the Lua function in a variable to be used later
    int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

    // Stack: viewIndex

    auto textField = getElement<TextField>(L);
    lua_getglobal(L, "script");
    auto script = (MDStudio::Script*)lua_touserdata(L, -1);
    textField->setTextDidChangeFn([=](TextField* sender, std::string text) {
        // Push the callback onto the stack using the Lua reference we stored in the registry
        lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
        registerElement<TextField, Control, View>(L, std::shared_ptr<TextField>(sender, BypassDeleter<TextField>()));
        lua_pushstring(L, text.c_str());
        if (lua_pcall(L, 2, 0, 0) != 0) {
            script->error(lua_tostring(L, -1));
        }
    });

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createBoxView(lua_State* L) {
    // Stack: name

    const char* name = luaL_checkstring(L, 1);

    std::shared_ptr<BoxView> boxView(new BoxView(name, nullptr));
    registerElement<BoxView, View>(L, boxView);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setBoxViewCornerRadius(lua_State* L) {
    // Stack: view, cornerRadius

    std::shared_ptr<BoxView> boxView = getElement<BoxView>(L);
    lua_Number cornerRadius = luaL_checknumber(L, 2);

    boxView->setCornerRadius(static_cast<float>(cornerRadius));

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createSlider(lua_State* L) {
    // Stack: name, min, max, pos

    const char* name = luaL_checkstring(L, 1);

    lua_Number min = luaL_checknumber(L, 2);
    lua_Number max = luaL_checknumber(L, 3);
    lua_Number pos = luaL_checknumber(L, 4);
    std::shared_ptr<Slider> slider(new Slider(name, nullptr, min, max, pos));
    registerElement<Slider, Control, View>(L, slider);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setSliderPosChangedFn(lua_State* L) {
    // Stack: view, callback

    // Store the reference to the Lua function in a variable to be used later
    int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

    // Stack: viewIndex

    auto slider = getElement<Slider>(L);
    lua_getglobal(L, "script");
    auto script = (MDStudio::Script*)lua_touserdata(L, -1);
    slider->setPosChangedFn([=](Slider* sender, float pos) {
        // Push the callback onto the stack using the Lua reference we stored in the registry
        lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
        registerElement<Slider, Control, View>(L, std::shared_ptr<Slider>(sender, BypassDeleter<Slider>()));
        lua_pushnumber(L, pos);
        if (lua_pcall(L, 2, 0, 0) != 0) {
            script->error(lua_tostring(L, -1));
        }
    });

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
static int sliderPos(lua_State* L) {
    // Stack: view

    auto slider = getElement<Slider>(L);

    lua_pushnumber(L, slider->pos());
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createComboBox(lua_State* L) {
    // Stack: name

    const char* name = luaL_checkstring(L, 1);

    std::shared_ptr<ComboBox> comboBox(new ComboBox(name, nullptr));
    registerElement<ComboBox, Control, View>(L, comboBox);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createStepper(lua_State* L) {
    // Stack: name, delta, min, max, value

    const char* name = luaL_checkstring(L, 1);

    lua_Number delta = luaL_checknumber(L, 2);
    lua_Number min = luaL_checknumber(L, 3);
    lua_Number max = luaL_checknumber(L, 4);
    lua_Number value = luaL_checknumber(L, 5);

    std::shared_ptr<Stepper> stepper(new Stepper(name, nullptr, delta, min, max, value));
    registerElement<Stepper, Control, View>(L, stepper);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createImage(lua_State* L) {
    // Stack: imagePath

    const char* imagePath = luaL_checkstring(L, 1);

    std::shared_ptr<Image> image = std::make_shared<Image>(imagePath, true);
    registerElement<Image>(L, image);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createImageView(lua_State* L) {
    // Stack: name

    const char* name = luaL_checkstring(L, 1);
    auto image = getElement<Image>(L, 2);

    std::shared_ptr<ImageView> imageView = std::make_shared<ImageView>(name, nullptr, image);
    registerElement<ImageView, View>(L, imageView);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createPath(lua_State* L) {
    // Stack:

    std::shared_ptr<Path> path = std::make_shared<Path>();
    registerElement<Path>(L, path);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createSVG(lua_State* L) {
    // Stack: s

    const char* s = luaL_checkstring(L, 1);

    std::shared_ptr<SVG> svg = std::make_shared<SVG>(s);
    registerElement<SVG>(L, svg);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createSVGView(lua_State* L) {
    // Stack: name, svg

    const char* name = luaL_checkstring(L, 1);
    auto svg = getElement<SVG>(L, 2);

    std::shared_ptr<SVGView> svgView = std::make_shared<SVGView>(name, nullptr, svg.get());
    registerElement<SVGView, View>(L, svgView);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createWebView(lua_State* L) {
    // Stack: name, s

    auto name = luaL_checkstring(L, 1);
    auto s = luaL_checkstring(L, 2);

    std::shared_ptr<WebView> webView = std::make_shared<WebView>(name, nullptr, s);
    registerElement<WebView, View>(L, webView);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createProgressIndicator(lua_State* L) {
    // Stack: name

    const char* name = luaL_checkstring(L, 1);
    lua_Number max = luaL_checknumber(L, 2);

    std::shared_ptr<ProgressIndicator> progressIndicator(new ProgressIndicator(name, nullptr, max));
    registerElement<ProgressIndicator, View>(L, progressIndicator);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createListView(lua_State* L) {
    // Stack: name, rowHeight

    const char* name = luaL_checkstring(L, 1);
    float rowHeight = luaL_checknumber(L, 2);

    std::shared_ptr<ListView> listView(new ListView(name, nullptr, rowHeight));
    registerElement<ListView, Control, View>(L, listView);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createSegmentedControl(lua_State* L) {
    // Stack: name

    const char* name = luaL_checkstring(L, 1);

    std::vector<Any> items;

    luaL_checktype(L, 2, LUA_TTABLE);
    lua_pushnil(L);
    while (lua_next(L, 2)) {
        // Uses 'key' (index -2) and 'value' (index -1)
        if (lua_isstring(L, -1)) {
            items.push_back(std::string(luaL_checkstring(L, -1)));
        } else if (lua_isuserdata(L, -1)) {
            auto imageView = getElement<Image>(L, -1);
            items.push_back(imageView);
        }
        lua_pop(L, 1);
    }

    std::shared_ptr<SegmentedControl> segmentedControl(new SegmentedControl(name, nullptr, items));
    registerElement<SegmentedControl, Control, View>(L, segmentedControl);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createSplitViewH(lua_State* L) {
    // Stack: name, leftView, rightView, splitPos

    const char* name = luaL_checkstring(L, 1);
    auto leftView = *((std::shared_ptr<View>*)lua_touserdata(L, 2));
    auto rightView = *((std::shared_ptr<View>*)lua_touserdata(L, 3));
    auto splitPos = luaL_checknumber(L, 4);

    std::shared_ptr<SplitViewH> splitViewH(new SplitViewH(name, nullptr, leftView, rightView, splitPos));
    registerElement<SplitViewH, View>(L, splitViewH);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createSplitViewMultiH(lua_State* L) {
    // Stack: name, views (table), splitPos (table)

    const char* name = luaL_checkstring(L, 1);

    std::vector<std::shared_ptr<View>> views;
    std::vector<std::pair<float, bool>> pos;

    luaL_checktype(L, 2, LUA_TTABLE);
    lua_pushnil(L);
    while (lua_next(L, 2)) {
        // Uses 'key' (index -2) and 'value' (index -1)
        views.push_back(*((std::shared_ptr<View>*)lua_touserdata(L, -1)));
        lua_pop(L, 1);
    }

    luaL_checktype(L, 3, LUA_TTABLE);
    lua_pushnil(L);
    while (lua_next(L, 3)) {
        // Uses 'key' (index -2) and 'value' (index -1)

        lua_pushnumber(L, 1);
        lua_gettable(L, -2);
        auto width = luaL_checknumber(L, -1);
        lua_pop(L, 1);

        lua_pushnumber(L, 2);
        lua_gettable(L, -2);
        if (!lua_isboolean(L, -1)) {
            luaL_error(L, "Boolean expected");
        }
        auto isResizable = lua_toboolean(L, -1);
        lua_pop(L, 1);

        pos.push_back(std::make_pair(width, isResizable != 0));
        lua_pop(L, 1);
    }

    auto splitViewMultiH = std::make_shared<SplitViewMultiH>(name, nullptr, views, pos);
    registerElement<SplitViewMultiH, View>(L, splitViewMultiH);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createWindow(lua_State* L) {
    // Stack:
    auto window = std::make_shared<Window>();
    registerElement<Window>(L, window);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createAnimation(lua_State* L) {
    auto animation = std::make_shared<Animation>();
    registerElement<Animation>(L, animation);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createLinearAnimationPath(lua_State* L) {
    // Store the reference to the Lua function in a variable to be used later
    int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_getglobal(L, "script");
    auto script = (MDStudio::Script*)lua_touserdata(L, -1);

    auto startPt = getPoint(L, 1);
    auto endPt = getPoint(L, 2);
    float speed = luaL_checknumber(L, 3);
    if (!lua_isboolean(L, 4)) luaL_error(L, "Boolean expected");
    auto isRepeating = lua_toboolean(L, 4) != 0;

    auto animationFn = [=](Point pt) {
        // Push the callback onto the stack using the Lua reference we stored in the registry
        lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);

        lua_newtable(L);
        lua_pushnumber(L, pt.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, pt.y);
        lua_setfield(L, -2, "y");

        if (lua_pcall(L, 1, 0, 0) != 0) {
            script->error(lua_tostring(L, -1));
        }
    };

    auto linearAnimationPath = std::make_shared<LinearAnimationPath>(startPt, endPt, speed, isRepeating, animationFn);
    registerElement<LinearAnimationPath>(L, linearAnimationPath);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createMenu(lua_State* L) {
    auto name = luaL_checkstring(L, 1);
    auto title = luaL_checkstring(L, 2);
    auto menu = std::make_shared<Menu>(name, nullptr, title);
    registerElement<Menu, View>(L, menu);
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createMenuItem(lua_State* L) {
    auto title = luaL_checkstring(L, 1);
    auto menuItem = std::make_shared<MenuItem>(title);
    registerElement<MenuItem>(L, menuItem);
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createMenuBar(lua_State* L) {
    auto name = luaL_checkstring(L, 1);
    auto menuBar = std::make_shared<MenuBar>(name, nullptr);
    registerElement<MenuBar, View>(L, menuBar);
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int createKeyboardH(lua_State* L) {
    auto name = luaL_checkstring(L, 1);
    auto keyboardH = std::make_shared<KeyboardH>(name, nullptr);
    registerElement<KeyboardH, Keyboard, Control, View>(L, keyboardH);
    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int getTopView(lua_State* L) {
    lua_getglobal(L, "uiScriptModule");
    MDStudio::UIScriptModule* uiScriptModule = (MDStudio::UIScriptModule*)lua_touserdata(L, -1);

    auto topView = std::shared_ptr<View>(uiScriptModule->topView(), BypassDeleter<View>());
    registerElement<View>(L, topView);

    return 1;
}

// ---------------------------------------------------------------------------------------------------------------------
static int setButtonClickedFn(lua_State* L) {
    // Stack: view, callback

    // store the reference to the Lua function in a variable to be used later
    int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

    // Stack: view

    auto button = getElement<Button>(L);
    lua_getglobal(L, "script");
    auto script = (MDStudio::Script*)lua_touserdata(L, -1);

    button->setClickedFn([=](Button* sender) {
        // Push the callback onto the stack using the Lua reference we stored in the registry
        lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
        registerElement<Button, Control, View>(L, std::shared_ptr<Button>(sender, BypassDeleter<Button>()));
        if (lua_pcall(L, 1, 0, 0) != 0) {
            script->error(lua_tostring(L, -1));
        }
    });

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
void UIScriptModule::init(Script* script) {
    script->setGlobal("uiScriptModule", this);

    // Draw Context
    std::vector<struct luaL_Reg> drawContextTableDefinition = {
        {"new", createDrawContext},
        {"__gc", destroyElement<DrawContext>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<DrawContext>(L, 1);
             auto e2 = getElement<DrawContext>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"pushStates",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             drawContext->pushStates();
             return 0;
         }},
        {"popStates",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             drawContext->popStates();
             return 0;
         }},
        {"translation",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto translation = drawContext->translation();

             lua_newtable(L);

             lua_pushnumber(L, translation.x);
             lua_setfield(L, -2, "x");

             lua_pushnumber(L, translation.y);
             lua_setfield(L, -2, "y");
             return 1;
         }},
        {"rotation",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto rotation = drawContext->rotation();
             lua_pushnumber(L, rotation);
             return 1;
         }},
        {"setTranslation",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto translation = getPoint(L, 2);
             drawContext->setTranslation(translation);
             return 0;
         }},
        {"setRotation",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto rotation = (float)luaL_checknumber(L, 2);
             drawContext->setRotation(rotation);
             return 0;
         }},
        {"setFillColor",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto fillColor = getColor(L, 2);
             drawContext->setFillColor(fillColor);
             return 0;
         }},
        {"setStrokeColor",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto strokeColor = getColor(L, 2);
             drawContext->setStrokeColor(strokeColor);
             return 0;
         }},
        {"setStrokeWidth",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto strokeWidth = luaL_checknumber(L, 2);
             drawContext->setStrokeWidth(static_cast<float>(strokeWidth));
             return 0;
         }},
        {"drawRect",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto rect = getRect(L, 2);
             drawContext->drawRect(rect);
             return 0;
         }},
        {"drawRoundRect",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto rect = getRect(L, 2);
             auto radius = luaL_checknumber(L, 3);
             drawContext->drawRoundRect(rect, static_cast<float>(radius));
             return 0;
         }},
        {"drawLine",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto p1 = getPoint(L, 2);
             auto p2 = getPoint(L, 3);
             drawContext->drawLine(p1, p2);
             return 0;
         }},
        {"drawTriangle",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto p1 = getPoint(L, 2);
             auto p2 = getPoint(L, 3);
             auto p3 = getPoint(L, 4);
             drawContext->drawTriangle(p1, p2, p3);
             return 0;
         }},
        {"drawCircle",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto pt = getPoint(L, 2);
             auto radius = luaL_checknumber(L, 3);
             drawContext->drawCircle(pt, static_cast<float>(radius));
             return 0;
         }},
        {"drawEllipse",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto pt = getPoint(L, 2);
             auto radiusX = luaL_checknumber(L, 3);
             auto radiusY = luaL_checknumber(L, 4);
             drawContext->drawEllipse(pt, static_cast<float>(radiusX), static_cast<float>(radiusY));
             return 0;
         }},
        {"drawPolygon",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto path = getElement<Path>(L, 2);
             drawContext->drawPolygon(path.get());
             return 0;
         }},
        {"drawPolyline",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto path = getElement<Path>(L, 2);
             drawContext->drawPolyline(path.get());
             return 0;
         }},
        {"drawArc",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto pt = getPoint(L, 2);
             auto radius = luaL_checknumber(L, 3);
             auto startAngle = luaL_checknumber(L, 4);
             auto arcAngle = luaL_checknumber(L, 5);
             drawContext->drawArc(pt, static_cast<float>(radius), static_cast<double>(startAngle),
                                  static_cast<double>(arcAngle));
             return 0;
         }},
        {"drawImage",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto pt = getPoint(L, 2);
             auto image = getElement<Image>(L, 3);
             auto color = getColor(L, 4);
             drawContext->drawImage(pt, image, color);
             return 0;
         }},
        {"drawText",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto pt = getPoint(L, 2);
             auto text = luaL_checkstring(L, 3);
             auto font = SystemFonts::sharedInstance()->semiboldFontTiny();
             drawContext->drawText(font, pt, text);
             return 0;
         }},
        {"drawLeftText",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto rect = getRect(L, 2);
             auto text = luaL_checkstring(L, 3);
             auto font = SystemFonts::sharedInstance()->semiboldFontTiny();
             drawContext->drawLeftText(font, rect, text);
             return 0;
         }},
        {"drawCenteredText",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto rect = getRect(L, 2);
             auto text = luaL_checkstring(L, 3);
             auto font = SystemFonts::sharedInstance()->semiboldFontTiny();
             drawContext->drawCenteredText(font, rect, text);
             return 0;
         }},
        {"drawRightText",
         [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             auto rect = getRect(L, 2);
             auto text = luaL_checkstring(L, 3);
             auto font = SystemFonts::sharedInstance()->semiboldFontTiny();
             drawContext->drawRightText(font, rect, text);
             return 0;
         }},
        {"draw", [](lua_State* L) -> int {
             auto drawContext = getElement<DrawContext>(L);
             drawContext->draw();
             return 0;
         }}};
    script->bindTable<DrawContext>("DrawContext", {drawContextTableDefinition});

    // View
    std::vector<struct luaL_Reg> viewTableHeaderDefinition = {
        {"new", createView}, {"__gc", destroyElement<View>}, {"__eq", [](lua_State* L) -> int {
                                                                  auto e1 = getElement<View>(L, 1);
                                                                  auto e2 = getElement<View>(L, 2);
                                                                  lua_pushboolean(L, e1 == e2);
                                                                  return 1;
                                                              }}};

    std::vector<struct luaL_Reg> viewTableContentDefinition = {
        {"addSubview", addSubview},
        {"setLayoutFn", setLayoutFn},
        {"setDrawFn", setDrawFn},
        {"setHandleEventFn", setHandleEventFn},
        {"setDisposeFn", setDisposeFn},
        {"setFrame", setFrame},
        {"frame", frame},
        {"rect", rect},
        {"resolvedClippedRect", resolvedClippedRect},
        {"bounds", bounds},
        {"subviews", subviews},
        {"name",
         [](lua_State* L) -> int {
             auto view = getElement<View>(L);
             lua_pushstring(L, view->name().c_str());
             return 1;
         }},
        {"removeAllSubviews",
         [](lua_State* L) -> int {
             auto view = getElement<View>(L);
             view->removeAllSubviews();
             return 0;
         }},
        {"setOwner",
         [](lua_State* L) -> int {
             auto view = getElement<View>(L);
             if (lua_isnil(L, -1)) {
                 if (view->owner()) {
                     int* ownerRef = (int*)view->owner();
                     delete ownerRef;
                 }
             } else {
                 int* ownerRef = new int;
                 *ownerRef = luaL_ref(L, LUA_REGISTRYINDEX);
                 view->setOwner(ownerRef);
             }
             return 0;
         }},
        {"owner",
         [](lua_State* L) -> int {
             auto view = getElement<View>(L);
             if (view->owner()) {
                 int* ownerRef = (int*)view->owner();
                 lua_rawgeti(L, LUA_REGISTRYINDEX, *ownerRef);
             } else {
                 lua_pushnil(L);
             }
             return 1;
         }},
        {"drawContext",
         [](lua_State* L) -> int {
             auto view = getElement<View>(L);
             registerElement<DrawContext>(
                 L, std::shared_ptr<DrawContext>(view->drawContext(), BypassDeleter<DrawContext>()));
             return 1;
         }},
        {"setDirty",
         [](lua_State* L) -> int {
             auto view = getElement<View>(L);
             view->setDirty();
             return 0;
         }},
        {"setTooltipText",
         [](lua_State* L) -> int {
             auto view = getElement<View>(L);
             auto text = luaL_checkstring(L, 2);
             view->setTooltipText(text);
             return 0;
         }},
        {"tooltipText", [](lua_State* L) -> int {
             auto view = getElement<View>(L);
             lua_pushstring(L, view->tooltipText().c_str());
             return 1;
         }}};

    script->bindTable<View>("View", {viewTableHeaderDefinition, viewTableContentDefinition});

    // ScrollView
    std::vector<struct luaL_Reg> scrollViewTableDefinition = {
        {"new", createScrollView},
        {"__gc", destroyElement<ScrollView, View>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<ScrollView>(L, 1);
             auto e2 = getElement<ScrollView>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setContentSize", setScrollViewContentSize},
        {"setPosChangedFn", setScrollViewPosChangedFn},
        {"setPos",
         [](lua_State* L) -> int {
             auto scrollView = getElement<ScrollView>(L);
             auto pos = getPoint(L, 2);
             scrollView->setPos(pos);
             return 0;
         }},
        {"scrollToVisibleRectV",
         [](lua_State* L) -> int {
             auto scrollView = getElement<ScrollView>(L);
             auto rect = getRect(L, 2);
             scrollView->scrollToVisibleRectV(rect);
             return 0;
         }},
        {"setIsHorizScrollBarVisible",
         [](lua_State* L) -> int {
             auto scrollView = getElement<ScrollView>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             auto isHorizScrollBarVisible = lua_toboolean(L, 2) != 0;
             scrollView->setIsHorizScrollBarVisible(isHorizScrollBarVisible);
             return 0;
         }},
        {"setIsVertScrollBarVisible",
         [](lua_State* L) -> int {
             auto scrollView = getElement<ScrollView>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             auto isVertScrollBarVisible = lua_toboolean(L, 2) != 0;
             scrollView->setIsVertScrollBarVisible(isVertScrollBarVisible);
             return 0;
         }},
        {"setIsScrollingWithDrag", [](lua_State* L) -> int {
             auto scrollView = getElement<ScrollView>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             auto isScrollingWithDrag = lua_toboolean(L, 2) != 0;
             scrollView->setIsScrollingWithDrag(isScrollingWithDrag);
             return 0;
         }}};
    script->bindTable<ScrollView, View>("ScrollView", {scrollViewTableDefinition, viewTableContentDefinition});

    // Button
    std::vector<struct luaL_Reg> buttonTableDefinition = {
        {"new", createButton},
        {"__gc", destroyElement<Button, Control, View>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<Button>(L, 1);
             auto e2 = getElement<Button>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setTitle", setButtonTitle},
        {"setClickedFn", setButtonClickedFn},
        {"setType",
         [](lua_State* L) -> int {
             auto button = getElement<Button>(L);
             lua_Integer type = luaL_checkinteger(L, 2);
             const Button::ButtonEnumType types[11] = {
                 Button::StandardButtonType,   Button::CheckBoxButtonType,     Button::CustomCheckBoxButtonType,
                 Button::RadioButtonType,      Button::OKButtonType,           Button::CancelButtonType,
                 Button::ComboBoxUpButtonType, Button::ComboBoxDownButtonType, Button::SegmentedControlButtonType,
                 Button::DisclosureButtonType, Button::SortButtonType};
             if (type >= 0 && type < 11) button->setType(types[type]);
             return 0;
         }},
        {"state",
         [](lua_State* L) -> int {
             auto button = getElement<Button>(L);
             lua_pushboolean(L, button->state() ? 1 : 0);
             return 1;
         }},
        {"setState",
         [](lua_State* L) -> int {
             auto button = getElement<Button>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             int state = lua_toboolean(L, 2);
             button->setState(state != 0);
             return 0;
         }},
        {"setBorderColor",
         [](lua_State* L) -> int {
             auto button = getElement<Button>(L);
             auto borderColor = getColor(L, 2);
             button->setBorderColor(borderColor);
             return 0;
         }},
        {"setHighlightColor",
         [](lua_State* L) -> int {
             auto button = getElement<Button>(L);
             auto borderColor = getColor(L, 2);
             button->setHighlightColor(borderColor);
             return 0;
         }},
        {"setTextColor", [](lua_State* L) -> int {
             auto button = getElement<Button>(L);
             auto textColor = getColor(L, 2);
             button->setTextColor(textColor);
             return 0;
         }}};
    script->bindTable<Button, Control, View>("Button", {buttonTableDefinition, viewTableContentDefinition});

    // LabelView
    std::vector<struct luaL_Reg> labelViewTableDefinition = {{"new", createLabelView},
                                                             {"__gc", destroyElement<LabelView, View>},
                                                             {"__eq",
                                                              [](lua_State* L) -> int {
                                                                  auto e1 = getElement<LabelView>(L, 1);
                                                                  auto e2 = getElement<LabelView>(L, 2);
                                                                  lua_pushboolean(L, e1 == e2);
                                                                  return 1;
                                                              }},
                                                             {"setTitle", setLabelViewTitle},
                                                             {"setTextAlign", setLabelViewTextAlign},
                                                             {"contentSize", labelViewContentSize},
                                                             {"setTextColor",
                                                              [](lua_State* L) -> int {
                                                                  auto labelView = getElement<LabelView>(L);
                                                                  auto textColor = getColor(L, 2);
                                                                  labelView->setTextColor(textColor);
                                                                  return 0;
                                                              }},
                                                             {"setFillColor", [](lua_State* L) -> int {
                                                                  auto labelView = getElement<LabelView>(L);
                                                                  auto fillColor = getColor(L, 2);
                                                                  labelView->setFillColor(fillColor);
                                                                  return 0;
                                                              }}};
    script->bindTable<LabelView, View>("LabelView", {labelViewTableDefinition, viewTableContentDefinition});

    // TextView
    std::vector<struct luaL_Reg> textViewTableDefinition = {{"new", createTextView},
                                                            {"__gc", destroyElement<TextView, Control, View>},
                                                            {"__eq",
                                                             [](lua_State* L) -> int {
                                                                 auto e1 = getElement<TextView>(L, 1);
                                                                 auto e2 = getElement<TextView>(L, 2);
                                                                 lua_pushboolean(L, e1 == e2);
                                                                 return 1;
                                                             }},
                                                            {"setText", setTextViewText},
                                                            {"setIsEnabled",
                                                             [](lua_State* L) -> int {
                                                                 auto textView = getElement<TextView>(L);
                                                                 if (!lua_isboolean(L, 2))
                                                                     luaL_error(L, "Boolean expected");
                                                                 int isEnabled = lua_toboolean(L, 2);
                                                                 textView->setIsEnabled(isEnabled != 0);
                                                                 return 0;
                                                             }},
                                                            {"contentSize", textViewContentSize}};
    script->bindTable<TextView, Control, View>("TextView", {textViewTableDefinition, viewTableContentDefinition});

    // TextField
    std::vector<struct luaL_Reg> textFieldTableDefinition = {
        {"new", createTextField},
        {"__gc", destroyElement<TextField, Control, View>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<TextField>(L, 1);
             auto e2 = getElement<TextField>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setBorderColor",
         [](lua_State* L) -> int {
             auto textField = getElement<TextField>(L);
             auto color = getColor(L, 2);
             textField->setBorderColor(color);
             return 0;
         }},
        {"setBorderSize",
         [](lua_State* L) -> int {
             auto textField = getElement<TextField>(L);
             lua_Number borderSize = luaL_checknumber(L, 2);
             textField->setBorderSize(static_cast<float>(borderSize));
             return 0;
         }},
        {"text",
         [](lua_State* L) -> int {
             auto textField = getElement<TextField>(L);
             auto text = textField->text();
             lua_pushstring(L, text.c_str());
             return 1;
         }},
        {"setText",
         [](lua_State* L) -> int {
             auto textField = getElement<TextField>(L);
             auto text = luaL_checkstring(L, 2);
             if (!lua_isboolean(L, 3)) luaL_error(L, "Boolean expected");
             bool isDelegateNotified = lua_toboolean(L, 3) != 0;
             textField->setText(text, isDelegateNotified);
             return 0;
         }},
        {"setTextDidChangeFn", setTextFieldTextDidChangeFn}};
    script->bindTable<TextField, Control, View>("TextField", {textFieldTableDefinition, viewTableContentDefinition});

    // BoxView
    std::vector<struct luaL_Reg> boxViewTableDefinition = {{"new", createBoxView},
                                                           {"__gc", destroyElement<BoxView, View>},
                                                           {"__eq",
                                                            [](lua_State* L) -> int {
                                                                auto e1 = getElement<BoxView>(L, 1);
                                                                auto e2 = getElement<BoxView>(L, 2);
                                                                lua_pushboolean(L, e1 == e2);
                                                                return 1;
                                                            }},
                                                           {"setCornerRadius", setBoxViewCornerRadius},
                                                           {"setFillColor",
                                                            [](lua_State* L) -> int {
                                                                auto boxView = getElement<BoxView>(L);
                                                                auto fillColor = getColor(L, 2);
                                                                boxView->setFillColor(fillColor);
                                                                return 0;
                                                            }},
                                                           {"setFillColors",
                                                            [](lua_State* L) -> int {
                                                                auto boxView = getElement<BoxView>(L);
                                                                auto bottomFillColor = getColor(L, 2);
                                                                auto topFillColor = getColor(L, 3);
                                                                boxView->setFillColors(bottomFillColor, topFillColor);
                                                                return 0;
                                                            }},
                                                           {"setBorderColor", [](lua_State* L) -> int {
                                                                auto boxView = getElement<BoxView>(L);
                                                                auto borderColor = getColor(L, 2);
                                                                boxView->setBorderColor(borderColor);
                                                                return 0;
                                                            }}};
    script->bindTable<BoxView, View>("BoxView", {boxViewTableDefinition, viewTableContentDefinition});

    // Slider
    std::vector<luaL_Reg> sliderTableDefinition{
        {"new", createSlider},
        {"__gc", destroyElement<Slider, Control, View>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<Slider>(L, 1);
             auto e2 = getElement<Slider>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"pos", sliderPos},
        {"setPosChangedFn", setSliderPosChangedFn},
        {"setType",
         [](lua_State* L) -> int {
             auto slider = getElement<Slider>(L);
             lua_Integer type = luaL_checkinteger(L, 2);
             const Slider::SliderEnumType types[2] = {Slider::LinearSliderType, Slider::RadialSliderType};
             if (type >= 0 && type < 2) slider->setType(types[type]);
             return 0;
         }},
        {"setThumbImage",
         [](lua_State* L) -> int {
             auto slider = getElement<Slider>(L);
             auto thumbImage = getElement<Image>(L, 2);
             slider->setThumbImage(thumbImage);
             return 0;
         }},
        {"setMinRailImage",
         [](lua_State* L) -> int {
             auto slider = getElement<Slider>(L);
             auto minRailImage = getElement<Image>(L, 2);
             slider->setMinRailImage(minRailImage);
             return 0;
         }},
        {"setMiddleRailImage",
         [](lua_State* L) -> int {
             auto slider = getElement<Slider>(L);
             auto middleRailImage = getElement<Image>(L, 2);
             slider->setMiddleRailImage(middleRailImage);
             return 0;
         }},
        {"setMaxRailImage",
         [](lua_State* L) -> int {
             auto slider = getElement<Slider>(L);
             auto maxRailImage = getElement<Image>(L, 2);
             slider->setMaxRailImage(maxRailImage);
             return 0;
         }},
        {"setMin",
         [](lua_State* L) -> int {
             auto slider = getElement<Slider>(L);
             auto min = luaL_checknumber(L, 2);
             slider->setMin(min);
             return 0;
         }},
        {"setMax",
         [](lua_State* L) -> int {
             auto slider = getElement<Slider>(L);
             auto max = luaL_checknumber(L, 2);
             slider->setMax(max);
             return 0;
         }},
        {"setPos", [](lua_State* L) -> int {
             auto slider = getElement<Slider>(L);
             auto pos = luaL_checknumber(L, 2);
             if (!lua_isboolean(L, 3)) luaL_error(L, "Boolean expected");
             bool isDelegateNotified = lua_toboolean(L, 3) != 0;
             slider->setPos(pos, isDelegateNotified);
             return 0;
         }}};
    script->bindTable<Slider, Control, View>("Slider", {sliderTableDefinition, viewTableContentDefinition});

    // ComboBox
    std::vector<struct luaL_Reg> comboBoxTableDefinition = {
        {"new", createComboBox},
        {"__gc", destroyElement<ComboBox, Control, View>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<ComboBox>(L, 1);
             auto e2 = getElement<ComboBox>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setListPosition",
         [](lua_State* L) -> int {
             auto comboBox = getElement<ComboBox>(L);
             auto listPosition = luaL_checkinteger(L, 2);
             const ComboBox::ListPositionEnumType listPositions[2] = {ComboBox::AbovePosition, ComboBox::BelowPosition};
             if (listPosition >= 0 && listPosition < 2) comboBox->setListPosition(listPositions[listPosition]);
             return 0;
         }},
        {"setSearchFieldIsVisible",
         [](lua_State* L) -> int {
             auto comboBox = getElement<ComboBox>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             int isVisible = lua_toboolean(L, 2);
             comboBox->setSearchFieldIsVisible(isVisible);
             return 0;
         }},
        {"setTitle",
         [](lua_State* L) -> int {
             auto comboBox = getElement<ComboBox>(L);
             auto title = luaL_checkstring(L, 2);
             comboBox->setTitle(title);
             return 0;
         }},
        {"setIsHorizScrollBarVisible",
         [](lua_State* L) -> int {
             auto comboBox = getElement<ComboBox>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             auto isHorizScrollBarVisible = lua_toboolean(L, 2) != 0;
             comboBox->setIsHorizScrollBarVisible(isHorizScrollBarVisible);
             return 0;
         }},
        {"setIsVertScrollBarVisible",
         [](lua_State* L) -> int {
             auto comboBox = getElement<ComboBox>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             auto isVertScrollBarVisible = lua_toboolean(L, 2) != 0;
             comboBox->setIsVertScrollBarVisible(isVertScrollBarVisible);
             return 0;
         }},
        {"setIsCapturing",
         [](lua_State* L) -> int {
             auto comboBox = getElement<ComboBox>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             auto isCapturing = lua_toboolean(L, 2) != 0;
             comboBox->setIsCapturing(isCapturing);
             return 0;
         }},
        {"setIsScrollingWithDrag",
         [](lua_State* L) -> int {
             auto comboBox = getElement<ComboBox>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             auto isScrollingWithDrag = lua_toboolean(L, 2) != 0;
             comboBox->setIsScrollingWithDrag(isScrollingWithDrag);
             return 0;
         }},
        {"setNbRowsFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto comboBox = getElement<ComboBox>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             comboBox->setNbRowsFn([=](ComboBox* sender) -> unsigned int {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ComboBox, Control, View>(L,
                                                          std::shared_ptr<ComboBox>(sender, BypassDeleter<ComboBox>()));
                 if (lua_pcall(L, 1, 1, 0) == 0) {
                     auto nbRows = luaL_checkinteger(L, -1);
                     return static_cast<unsigned int>(nbRows);
                 } else {
                     script->error(lua_tostring(L, -1));
                 }
                 return 0;
             });
             return 0;
         }},
        {"setViewForRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto comboBox = getElement<ComboBox>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             comboBox->setViewForRowFn([=](ComboBox* sender, int row) -> std::shared_ptr<View> {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ComboBox, Control, View>(L,
                                                          std::shared_ptr<ComboBox>(sender, BypassDeleter<ComboBox>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 1, 0) == 0) {
                     auto view = getElement<View>(L, -1);
                     return view;
                 } else {
                     script->error(lua_tostring(L, -1));
                 }
                 return nullptr;
             });
             return 0;
         }},
        {"setDidSelectRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto comboBox = getElement<ComboBox>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             comboBox->setDidSelectRowFn([=](ComboBox* sender, int row) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ComboBox, Control, View>(L,
                                                          std::shared_ptr<ComboBox>(sender, BypassDeleter<ComboBox>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidDeselectRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto comboBox = getElement<ComboBox>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             comboBox->setDidDeselectRowFn([=](ComboBox* sender, int row) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ComboBox, Control, View>(L,
                                                          std::shared_ptr<ComboBox>(sender, BypassDeleter<ComboBox>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidHoverRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto comboBox = getElement<ComboBox>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             comboBox->setDidHoverRowFn([=](ComboBox* sender, int row) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ComboBox, Control, View>(L,
                                                          std::shared_ptr<ComboBox>(sender, BypassDeleter<ComboBox>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 };
             });
             return 0;
         }},
        {"setDidConfirmRowSelectionFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto comboBox = getElement<ComboBox>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             comboBox->setDidConfirmRowSelectionFn([=](ComboBox* sender, int row) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ComboBox, Control, View>(L,
                                                          std::shared_ptr<ComboBox>(sender, BypassDeleter<ComboBox>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidSetFocusStateFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto comboBox = getElement<ComboBox>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             comboBox->setDidSetFocusStateFn([=](ComboBox* sender, bool state) {
                 if (!script->isRunning()) return;
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ComboBox, Control, View>(L,
                                                          std::shared_ptr<ComboBox>(sender, BypassDeleter<ComboBox>()));
                 lua_pushboolean(L, state ? 1 : 0);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"reload",
         [](lua_State* L) -> int {
             auto comboBox = getElement<ComboBox>(L);
             comboBox->reload();
             return 0;
         }},
        {"rowViews",
         [](lua_State* L) -> int {
             auto comboBox = getElement<ComboBox>(L);

             auto rowViews = comboBox->rowViews();

             lua_newtable(L);

             lua_Integer i = 1;
             for (auto v : rowViews) {
                 lua_pushinteger(L, i);
                 registerElement<View>(L, v);
                 lua_settable(L, -3);
                 ++i;
             }

             return 1;
         }},
        {"scrollToVisibleRectV",
         [](lua_State* L) -> int {
             auto comboBox = getElement<ComboBox>(L);
             auto rect = getRect(L, 2);
             comboBox->scrollToVisibleRectV(rect);
             return 0;
         }},
        {"close",
         [](lua_State* L) -> int {
             auto comboBox = getElement<ComboBox>(L);
             comboBox->close();
             return 0;
         }},
        {"setMaxHeight",
         [](lua_State* L) -> int {
             auto comboBox = getElement<ComboBox>(L);
             auto maxHeight = luaL_checknumber(L, 2);
             comboBox->setMaxHeight(maxHeight);
             return 0;
         }},
        {"maxHeight", [](lua_State* L) -> int {
             auto comboBox = getElement<ComboBox>(L);
             lua_pushnumber(L, comboBox->maxHeight());
             return 1;
         }}};
    script->bindTable<ComboBox, Control, View>("ComboBox", {comboBoxTableDefinition, viewTableContentDefinition});

    // Stepper
    std::vector<struct luaL_Reg> stepperTableDefinition = {{"new", createStepper},
                                                           {"__gc", destroyElement<Stepper, Control, View>},
                                                           {"__eq", [](lua_State* L) -> int {
                                                                auto e1 = getElement<Stepper>(L, 1);
                                                                auto e2 = getElement<Stepper>(L, 2);
                                                                lua_pushboolean(L, e1 == e2);
                                                                return 1;
                                                            }}};
    script->bindTable<Stepper, Control, View>("Stepper", {stepperTableDefinition, viewTableContentDefinition});

    // Image
    std::vector<struct luaL_Reg> imageTableDefinition = {{"new", createImage},
                                                         {"__gc", destroyElement<Image>},
                                                         {"__eq",
                                                          [](lua_State* L) -> int {
                                                              auto e1 = getElement<Image>(L, 1);
                                                              auto e2 = getElement<Image>(L, 2);
                                                              lua_pushboolean(L, e1 == e2);
                                                              return 1;
                                                          }},
                                                         {"size", [](lua_State* L) -> int {
                                                              std::shared_ptr<Image> image = getElement<Image>(L);

                                                              lua_newtable(L);

                                                              lua_pushnumber(L, image->size().width);
                                                              lua_setfield(L, -2, "width");

                                                              lua_pushnumber(L, image->size().height);
                                                              lua_setfield(L, -2, "height");
                                                              return 1;
                                                          }}};
    script->bindTable<Image>("Image", {imageTableDefinition});

    // ImageView
    std::vector<struct luaL_Reg> imageViewTableDefinition = {{"new", createImageView},
                                                             {"__gc", destroyElement<ImageView, View>},
                                                             {"__eq",
                                                              [](lua_State* L) -> int {
                                                                  auto e1 = getElement<ImageView>(L, 1);
                                                                  auto e2 = getElement<ImageView>(L, 2);
                                                                  lua_pushboolean(L, e1 == e2);
                                                                  return 1;
                                                              }},
                                                             {"setColor", [](lua_State* L) -> int {
                                                                  auto imageView = getElement<ImageView>(L);
                                                                  auto color = getColor(L, 2);
                                                                  imageView->setColor(color);
                                                                  return 0;
                                                              }}};
    script->bindTable<ImageView, View>("ImageView", {imageViewTableDefinition, viewTableContentDefinition});

    // Path
    std::vector<struct luaL_Reg> pathTableDefinition = {{"new", createPath},
                                                        {"__gc", destroyElement<Path>},
                                                        {"__eq",
                                                         [](lua_State* L) -> int {
                                                             auto e1 = getElement<Path>(L, 1);
                                                             auto e2 = getElement<Path>(L, 2);
                                                             lua_pushboolean(L, e1 == e2);
                                                             return 1;
                                                         }},
                                                        {"addPoint", [](lua_State* L) -> int {
                                                             auto path = getElement<Path>(L, 1);
                                                             auto point = getPoint(L, 2);
                                                             path->addPoint(point);
                                                             return 0;
                                                         }}};
    script->bindTable<Path>("Path", {pathTableDefinition});

    // SVG
    std::vector<struct luaL_Reg> svgTableDefinition = {
        {"new", createSVG}, {"__gc", destroyElement<SVG>}, {"__eq", [](lua_State* L) -> int {
                                                                auto e1 = getElement<SVG>(L, 1);
                                                                auto e2 = getElement<SVG>(L, 2);
                                                                lua_pushboolean(L, e1 == e2);
                                                                return 1;
                                                            }}};
    script->bindTable<SVG>("SVG", {svgTableDefinition});

    // SVGView
    std::vector<struct luaL_Reg> svgViewTableDefinition = {
        {"new", createSVGView}, {"__gc", destroyElement<SVGView, View>}, {"__eq", [](lua_State* L) -> int {
                                                                              auto e1 = getElement<SVGView>(L, 1);
                                                                              auto e2 = getElement<SVGView>(L, 2);
                                                                              lua_pushboolean(L, e1 == e2);
                                                                              return 1;
                                                                          }}};
    script->bindTable<SVGView, View>("SVGView", {svgViewTableDefinition, viewTableContentDefinition});

    // WebView
    std::vector<struct luaL_Reg> webViewTableDefinition = {
        {"new", createWebView},
        {"__gc", destroyElement<WebView, View>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<WebView>(L, 1);
             auto e2 = getElement<WebView>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"contentHeight", [](lua_State* L) -> int {
             auto webView = getElement<WebView>(L);
             auto width = luaL_checknumber(L, 2);
             lua_pushnumber(L, webView->contentHeight(static_cast<float>(width)));
             return 1;
         }}};
    script->bindTable<WebView, View>("WebView", {webViewTableDefinition, viewTableContentDefinition});

    // ProgressIndicator
    std::vector<struct luaL_Reg> progressIndicatorTableDefinition = {{"new", createProgressIndicator},
                                                                     {"__gc", destroyElement<ProgressIndicator, View>},
                                                                     {"__eq", [](lua_State* L) -> int {
                                                                          auto e1 = getElement<ProgressIndicator>(L, 1);
                                                                          auto e2 = getElement<ProgressIndicator>(L, 2);
                                                                          lua_pushboolean(L, e1 == e2);
                                                                          return 1;
                                                                      }}};
    script->bindTable<ProgressIndicator, View>("ProgressIndicator",
                                               {progressIndicatorTableDefinition, viewTableContentDefinition});

    // ListView
    std::vector<struct luaL_Reg> listViewTableDefinition = {
        {"new", createListView},
        {"__gc", destroyElement<ListView, Control, View>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<ListView>(L, 1);
             auto e2 = getElement<ListView>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"contentHeight",
         [](lua_State* L) -> int {
             auto listView = getElement<ListView>(L);
             lua_pushnumber(L, listView->contentHeight());
             return 1;
         }},
        {"setNbRowsFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto listView = getElement<ListView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             listView->setNbRowsFn([=](ListView* sender) -> unsigned int {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ListView, Control, View>(L,
                                                          std::shared_ptr<ListView>(sender, BypassDeleter<ListView>()));
                 if (lua_pcall(L, 1, 1, 0) == 0) {
                     auto nbRows = luaL_checkinteger(L, -1);
                     return static_cast<unsigned int>(nbRows);
                 } else {
                     script->error(lua_tostring(L, -1));
                 }
                 return 0;
             });
             return 0;
         }},
        {"setViewForRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto listView = getElement<ListView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             listView->setViewForRowFn([=](ListView* sender, int row) -> std::shared_ptr<View> {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ListView, Control, View>(L,
                                                          std::shared_ptr<ListView>(sender, BypassDeleter<ListView>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 1, 0) == 0) {
                     auto view = getElement<View>(L, -1);
                     return view;
                 } else {
                     script->error(lua_tostring(L, -1));
                 }
                 return nullptr;
             });
             return 0;
         }},
        {"setDidSelectRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto listView = getElement<ListView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             listView->setDidSelectRowFn([=](ListView* sender, int row) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ListView, Control, View>(L,
                                                          std::shared_ptr<ListView>(sender, BypassDeleter<ListView>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidDeselectRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto listView = getElement<ListView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             listView->setDidDeselectRowFn([=](ListView* sender, int row) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ListView, Control, View>(L,
                                                          std::shared_ptr<ListView>(sender, BypassDeleter<ListView>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidHoverRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto listView = getElement<ListView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             listView->setDidHoverRowFn([=](ListView* sender, int row) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ListView, Control, View>(L,
                                                          std::shared_ptr<ListView>(sender, BypassDeleter<ListView>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 };
             });
             return 0;
         }},
        {"setDidConfirmRowSelectionFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto listView = getElement<ListView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             listView->setDidConfirmRowSelectionFn([=](ListView* sender, int row) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ListView, Control, View>(L,
                                                          std::shared_ptr<ListView>(sender, BypassDeleter<ListView>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidSetFocusStateFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto listView = getElement<ListView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             listView->setDidSetFocusStateFn([=](ListView* sender, bool state) {
                 if (!script->isRunning()) return;
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<ListView, Control, View>(L,
                                                          std::shared_ptr<ListView>(sender, BypassDeleter<ListView>()));
                 lua_pushboolean(L, state ? 1 : 0);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"reload", [](lua_State* L) -> int {
             auto listView = getElement<ListView>(L);
             listView->reload();
             return 0;
         }}};
    script->bindTable<ListView, Control, View>("ListView", {listViewTableDefinition, viewTableContentDefinition});

    // ListItemView
    std::vector<struct luaL_Reg> listItemViewTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             const char* name = luaL_checkstring(L, 1);
             const char* title = luaL_checkstring(L, 2);

             auto listItemView = std::make_shared<ListItemView>(name, nullptr, title);
             registerElement<ListItemView, View>(L, listItemView);

             return 1;
         }},
        {"__gc", destroyElement<ListItemView, View>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<ListItemView>(L, 1);
             auto e2 = getElement<ListItemView>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},

        {"setIsHighlighted",
         [](lua_State* L) -> int {
             auto listItemView = getElement<ListItemView>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             int state = lua_toboolean(L, 2);
             listItemView->setIsHighlighted(state != 0);
             return 0;
         }},
        {"setIsHovering",
         [](lua_State* L) -> int {
             auto listItemView = getElement<ListItemView>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             int state = lua_toboolean(L, 2);
             listItemView->setIsHovering(state != 0);
             return 0;
         }},
        {"setFocusState", [](lua_State* L) -> int {
             auto listItemView = getElement<ListItemView>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             int state = lua_toboolean(L, 2);
             listItemView->setFocusState(state != 0);
             return 0;
         }}};
    script->bindTable<ListItemView, View>("ListItemView", {listItemViewTableDefinition, viewTableContentDefinition});

    // Column
    std::vector<struct luaL_Reg> columnTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             const char* title = luaL_checkstring(L, 1);
             auto width = luaL_checknumber(L, 2);
             if (!lua_isboolean(L, 3)) luaL_error(L, "Boolean expected");
             auto isResizable = lua_toboolean(L, 3);
             if (!lua_isboolean(L, 4)) luaL_error(L, "Boolean expected");
             auto isSortAvailable = lua_toboolean(L, 5);
             if (!lua_isboolean(L, 5)) luaL_error(L, "Boolean expected");
             auto isSortEnabled = lua_toboolean(L, 5);
             auto sortDirection = luaL_checkinteger(L, 6);

             auto column = std::make_shared<Column>(title, width, isResizable, isSortAvailable != 0, isSortEnabled != 0,
                                                    sortDirection == 1 ? Column::Descending : Column::Ascending);
             registerElement<Column>(L, column);

             return 1;
         }},
        {"__gc", destroyElement<Column>},
        {"__eq", [](lua_State* L) -> int {
             auto e1 = getElement<Column>(L, 1);
             auto e2 = getElement<Column>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }}};
    script->bindTable<Column>("Column", {columnTableDefinition});

    // TableView
    std::vector<struct luaL_Reg> tableViewTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             const char* name = luaL_checkstring(L, 1);
             lua_Number rowHeight = luaL_checknumber(L, 2);

             auto tableView = std::make_shared<TableView>(name, nullptr, rowHeight);
             registerElement<TableView, Control, View>(L, tableView);

             return 1;
         }},
        {"__gc", destroyElement<TableView, Control, View>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<TableView>(L, 1);
             auto e2 = getElement<TableView>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setNbColumnsFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto tableView = getElement<TableView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             tableView->setNbColumnsFn([=](TableView* sender) -> unsigned int {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TableView, Control, View>(
                     L, std::shared_ptr<TableView>(sender, BypassDeleter<TableView>()));
                 if (lua_pcall(L, 1, 1, 0) == 0) {
                     auto nbColumns = luaL_checkinteger(L, -1);
                     return static_cast<unsigned int>(nbColumns);
                 } else {
                     script->error(lua_tostring(L, -1));
                 }
                 return 0;
             });
             return 0;
         }},
        {"setColumnAtIndexFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto tableView = getElement<TableView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             tableView->setColumnAtIndexFn([=](TableView* sender, int index) -> std::shared_ptr<Column> {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TableView, Control, View>(
                     L, std::shared_ptr<TableView>(sender, BypassDeleter<TableView>()));
                 lua_pushinteger(L, index + 1);
                 if (lua_pcall(L, 2, 1, 0) == 0) {
                     auto column = getElement<Column>(L, -1);
                     return column;
                 } else {
                     script->error(lua_tostring(L, -1));
                 }
                 return nullptr;
             });
             return 0;
         }},
        {"setNbRowsFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto tableView = getElement<TableView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             tableView->setNbRowsFn([=](TableView* sender) -> unsigned int {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TableView, Control, View>(
                     L, std::shared_ptr<TableView>(sender, BypassDeleter<TableView>()));
                 if (lua_pcall(L, 1, 1, 0) == 0) {
                     auto nbRows = luaL_checkinteger(L, -1);
                     return static_cast<unsigned int>(nbRows);
                 } else {
                     script->error(lua_tostring(L, -1));
                 }
                 return 0;
             });
             return 0;
         }},
        {"setViewForRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto tableView = getElement<TableView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             tableView->setViewForRowFn([=](TableView* sender, int row) -> std::shared_ptr<View> {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TableView, Control, View>(
                     L, std::shared_ptr<TableView>(sender, BypassDeleter<TableView>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 1, 0) == 0) {
                     auto view = getElement<View>(L, -1);
                     return view;
                 } else {
                     script->error(lua_tostring(L, -1));
                 }
                 return nullptr;
             });
             return 0;
         }},
        {"setDidSelectRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto tableView = getElement<TableView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             tableView->setDidSelectRowFn([=](TableView* sender, int row) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TableView, Control, View>(
                     L, std::shared_ptr<TableView>(sender, BypassDeleter<TableView>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidDeselectRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto tableView = getElement<TableView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             tableView->setDidDeselectRowFn([=](TableView* sender, int row) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TableView, Control, View>(
                     L, std::shared_ptr<TableView>(sender, BypassDeleter<TableView>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidHoverRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto tableView = getElement<TableView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             tableView->setDidHoverRowFn([=](TableView* sender, int row) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TableView, Control, View>(
                     L, std::shared_ptr<TableView>(sender, BypassDeleter<TableView>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 };
             });
             return 0;
         }},
        {"setDidConfirmRowSelectionFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto tableView = getElement<TableView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             tableView->setDidConfirmRowSelectionFn([=](TableView* sender, int row) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TableView, Control, View>(
                     L, std::shared_ptr<TableView>(sender, BypassDeleter<TableView>()));
                 lua_pushinteger(L, row + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidSetFocusStateFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto tableView = getElement<TableView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             tableView->setDidSetFocusStateFn([=](TableView* sender, bool state) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TableView, Control, View>(
                     L, std::shared_ptr<TableView>(sender, BypassDeleter<TableView>()));
                 lua_pushboolean(L, state ? 1 : 0);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidResizeColumnFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto tableView = getElement<TableView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             tableView->setDidResizeColumnFn([=](TableView* sender, int columnIndex, float width) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TableView, Control, View>(
                     L, std::shared_ptr<TableView>(sender, BypassDeleter<TableView>()));
                 lua_pushinteger(L, columnIndex + 1);
                 lua_pushnumber(L, width);
                 if (lua_pcall(L, 3, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"reload",
         [](lua_State* L) -> int {
             auto tableView = getElement<TableView>(L);
             tableView->reload();
             return 0;
         }},
        {"visibleRowViews",
         [](lua_State* L) -> int {
             auto tableView = getElement<TableView>(L);

             auto rowViews = tableView->visibleRowViews();

             lua_newtable(L);

             lua_Integer i = 1;
             for (auto v : rowViews) {
                 lua_pushinteger(L, i);
                 registerElement<View>(L, v);
                 lua_settable(L, -3);
                 ++i;
             }

             return 1;
         }},
        {"viewAtRow",
         [](lua_State* L) -> int {
             auto tableView = getElement<TableView>(L);
             auto row = (int)luaL_checkinteger(L, 2);
             auto rowView = tableView->viewAtRow(row);
             registerElement<View>(L, rowView);

             return 1;
         }},
        {"scrollToVisibleRectV", [](lua_State* L) -> int {
             auto tableView = getElement<TableView>(L);
             auto rect = getRect(L, 2);
             tableView->scrollToVisibleRectV(rect);
             return 0;
         }}};
    script->bindTable<TableView, Control, View>("TableView", {tableViewTableDefinition, viewTableContentDefinition});

    // TreeView
    std::vector<struct luaL_Reg> treeViewTableDefinition = {
        {"new",
         [](lua_State* L) -> int {
             const char* name = luaL_checkstring(L, 1);
             Size itemSize = getSize(L, 2);

             auto treeView = std::make_shared<TreeView>(name, nullptr, itemSize);
             registerElement<TreeView, Control, View>(L, treeView);

             return 1;
         }},
        {"__gc", destroyElement<TreeView, Control, View>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<TreeView>(L, 1);
             auto e2 = getElement<TreeView>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setNbRowsFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto listView = getElement<TreeView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             listView->setNbRowsFn([=](TreeView* sender, std::vector<int> indexPath) -> unsigned int {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TreeView, Control, View>(L,
                                                          std::shared_ptr<TreeView>(sender, BypassDeleter<TreeView>()));

                 lua_newtable(L);
                 lua_Integer i = 1;
                 for (auto v : indexPath) {
                     lua_pushinteger(L, i);
                     lua_pushinteger(L, v);
                     lua_settable(L, -3);
                     ++i;
                 }

                 if (lua_pcall(L, 2, 1, 0) == 0) {
                     auto nbRows = luaL_checkinteger(L, -1);
                     return static_cast<unsigned int>(nbRows);
                 } else {
                     script->error(lua_tostring(L, -1));
                 }
                 return 0;
             });
             return 0;
         }},
        {"setViewForIndexPathFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto treeView = getElement<TreeView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             treeView->setViewForIndexPathFn(
                 [=](TreeView* sender, std::vector<int> indexPath, bool* isExpanded) -> std::shared_ptr<View> {
                     // Push the callback onto the stack using the Lua reference we stored in the registry
                     lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                     registerElement<TreeView, Control, View>(
                         L, std::shared_ptr<TreeView>(sender, BypassDeleter<TreeView>()));

                     lua_newtable(L);
                     lua_Integer i = 1;
                     for (auto v : indexPath) {
                         lua_pushinteger(L, i);
                         lua_pushinteger(L, v + 1);
                         lua_settable(L, -3);
                         ++i;
                     }

                     if (lua_pcall(L, 2, 2, 0) == 0) {
                         auto view = getElement<View>(L, -2);
                         if (!lua_isboolean(L, -1)) {
                             luaL_error(L, "Boolean expected");
                         }
                         *isExpanded = lua_toboolean(L, -1);
                         return view;
                     } else {
                         script->error(lua_tostring(L, -1));
                     }
                     return nullptr;
                 });
             return 0;
         }},
        {"setDidSelectRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto treeView = getElement<TreeView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             treeView->setDidSelectRowFn([=](TreeView* sender, std::vector<int> indexPath) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TreeView, Control, View>(L,
                                                          std::shared_ptr<TreeView>(sender, BypassDeleter<TreeView>()));

                 lua_newtable(L);
                 lua_Integer i = 1;
                 for (auto v : indexPath) {
                     lua_pushinteger(L, i);
                     lua_pushinteger(L, v + 1);
                     lua_settable(L, -3);
                     ++i;
                 }

                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidDeselectRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto treeView = getElement<TreeView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             treeView->setDidDeselectRowFn([=](TreeView* sender, std::vector<int> indexPath) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TreeView, Control, View>(L,
                                                          std::shared_ptr<TreeView>(sender, BypassDeleter<TreeView>()));

                 lua_newtable(L);
                 lua_Integer i = 1;
                 for (auto v : indexPath) {
                     lua_pushinteger(L, i);
                     lua_pushinteger(L, v + 1);
                     lua_settable(L, -3);
                     ++i;
                 }

                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidHoverRowFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto treeView = getElement<TreeView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             treeView->setDidHoverRowFn([=](TreeView* sender, std::vector<int> indexPath) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TreeView, Control, View>(L,
                                                          std::shared_ptr<TreeView>(sender, BypassDeleter<TreeView>()));

                 lua_newtable(L);
                 lua_Integer i = 1;
                 for (auto v : indexPath) {
                     lua_pushinteger(L, i);
                     lua_pushinteger(L, v + 1);
                     lua_settable(L, -3);
                     ++i;
                 }

                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidConfirmRowSelectionFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto treeView = getElement<TreeView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             treeView->setDidConfirmRowSelectionFn([=](TreeView* sender, std::vector<int> indexPath) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TreeView, Control, View>(L,
                                                          std::shared_ptr<TreeView>(sender, BypassDeleter<TreeView>()));

                 lua_newtable(L);
                 lua_Integer i = 1;
                 for (auto v : indexPath) {
                     lua_pushinteger(L, i);
                     lua_pushinteger(L, v + 1);
                     lua_settable(L, -3);
                     ++i;
                 }

                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setDidSetFocusStateFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto treeView = getElement<TreeView>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             treeView->setDidSetFocusStateFn([=](TreeView* sender, bool state) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<TreeView, Control, View>(L,
                                                          std::shared_ptr<TreeView>(sender, BypassDeleter<TreeView>()));
                 lua_pushboolean(L, state ? 1 : 0);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"items",
         [](lua_State* L) -> int {
             auto treeView = getElement<TreeView>(L);

             auto items = treeView->items();

             lua_newtable(L);  // {...}
             lua_Integer i = 1;
             for (auto item : items) {
                 lua_pushinteger(L, i);

                 lua_newtable(L);  //  {{...}}

                 lua_pushinteger(L, 1);

                 lua_newtable(L);  // {{{...}}}
                 lua_Integer j = 1;
                 for (auto v : item.first) {
                     lua_pushinteger(L, j);
                     lua_pushinteger(L, v + 1);
                     lua_settable(L, -3);  //  {{{...}}}
                     ++j;
                 }

                 lua_settable(L, -3);  // {{...}}

                 lua_pushinteger(L, 2);

                 auto view = item.second;
                 registerElement<View>(L, view);

                 lua_settable(L, -3);  // {{...}}

                 lua_settable(L, -3);  // {...}

                 ++i;
             }

             return 1;
         }},
        {"viewAtIndexPath",
         [](lua_State* L) -> int {
             auto treeView = getElement<TreeView>(L);

             std::vector<int> indexPath;
             auto length = lua_rawlen(L, 2);
             for (int i = 1; i <= length; ++i) {
                 lua_pushinteger(L, i);
                 lua_gettable(L, 2);
                 auto index = luaL_checkinteger(L, -1);
                 indexPath.push_back(static_cast<int>(index - 1));
                 lua_pop(L, 1);
             }
             auto view = treeView->viewAtIndexPath(indexPath);
             registerElement<View>(L, view);

             return 1;
         }},
        {"reload", [](lua_State* L) -> int {
             auto treeView = getElement<TreeView>(L);
             treeView->reload();
             return 0;
         }}};
    script->bindTable<TreeView, Control, View>("TreeView", {treeViewTableDefinition, viewTableContentDefinition});

    // SegmentedControl
    std::vector<struct luaL_Reg> segmentedControlTableDefinition = {
        {"new", createSegmentedControl},
        {"__gc", destroyElement<SegmentedControl, Control, View>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<SegmentedControl>(L, 1);
             auto e2 = getElement<SegmentedControl>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"setDidSelectSegmentFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto segmentedControl = getElement<SegmentedControl>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             segmentedControl->setDidSelectSegmentFn([=](SegmentedControl* sender, int selectedSegment) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<SegmentedControl, Control, View>(
                     L, std::shared_ptr<SegmentedControl>(sender, BypassDeleter<SegmentedControl>()));
                 lua_pushinteger(L, selectedSegment + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"selectedSegment",
         [](lua_State* L) -> int {
             auto segmentedControl = getElement<SegmentedControl>(L);
             lua_pushinteger(L, segmentedControl->selectedSegment() + 1);
             return 1;
         }},
        {"setSelectedSegment",
         [](lua_State* L) -> int {
             auto segmentedControl = getElement<SegmentedControl>(L);
             auto selectedSegment = static_cast<int>(luaL_checkinteger(L, 2));
             if (!lua_isboolean(L, 3)) luaL_error(L, "Boolean expected");
             bool isDelegateNotified = lua_toboolean(L, 3) != 0;
             segmentedControl->setSelectedSegment(selectedSegment - 1, isDelegateNotified);
             return 0;
         }},
        {"setBorderColor",
         [](lua_State* L) -> int {
             auto segmentedControl = getElement<SegmentedControl>(L);
             auto borderColor = getColor(L, 2);
             segmentedControl->setBorderColor(borderColor);
             return 0;
         }},
        {"setTextColor",
         [](lua_State* L) -> int {
             auto segmentedControl = getElement<SegmentedControl>(L);
             auto textColor = getColor(L, 2);
             segmentedControl->setTextColor(textColor);
             return 0;
         }},
        {"setHighlightColor", [](lua_State* L) -> int {
             auto segmentedControl = getElement<SegmentedControl>(L);
             auto highlightColor = getColor(L, 2);
             segmentedControl->setHighlightColor(highlightColor);
             return 0;
         }}};
    script->bindTable<SegmentedControl, Control, View>("SegmentedControl",
                                                       {segmentedControlTableDefinition, viewTableContentDefinition});

    // SplitViewH
    std::vector<struct luaL_Reg> splitViewHTableDefinition = {{"new", createSplitViewH},
                                                              {"__gc", destroyElement<SplitViewH, View>},
                                                              {"__eq", [](lua_State* L) -> int {
                                                                   auto e1 = getElement<SplitViewH>(L, 1);
                                                                   auto e2 = getElement<SplitViewH>(L, 2);
                                                                   lua_pushboolean(L, e1 == e2);
                                                                   return 1;
                                                               }}};
    script->bindTable<SplitViewH, View>("SplitViewH", {splitViewHTableDefinition, viewTableContentDefinition});

    // SplitViewMultiH
    std::vector<struct luaL_Reg> splitViewMultiHTableDefinition = {{"new", createSplitViewMultiH},
                                                                   {"__gc", destroyElement<SplitViewMultiH, View>},
                                                                   {"__eq", [](lua_State* L) -> int {
                                                                        auto e1 = getElement<SplitViewMultiH>(L, 1);
                                                                        auto e2 = getElement<SplitViewMultiH>(L, 2);
                                                                        lua_pushboolean(L, e1 == e2);
                                                                        return 1;
                                                                    }}};
    script->bindTable<SplitViewMultiH, View>("SplitViewMultiH",
                                             {splitViewMultiHTableDefinition, viewTableContentDefinition});

    // Window
    std::vector<struct luaL_Reg> windowTableDefinition = {
        {"new", createWindow},
        {"__gc", destroyElement<Window>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<Window>(L, 1);
             auto e2 = getElement<Window>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"contentView",
         [](lua_State* L) -> int {
             auto window = getElement<Window>(L);
             auto contentView = window->contentView();
             registerElement<View>(L, std::shared_ptr<View>(contentView, BypassDeleter<View>()));
             return 1;
         }},
        {"open",
         [](lua_State* L) -> int {
             auto window = getElement<Window>(L, 1);
             auto rect = getRect(L, 2);
             window->open(rect, true);
             return 0;
         }},
        {"close", [](lua_State* L) -> int {
             auto window = getElement<Window>(L, 1);
             window->close();
             return 0;
         }}};
    script->bindTable<Window>("Window", {windowTableDefinition});

    // Animation
    std::vector<struct luaL_Reg> animationTableDefinition = {
        {"new", createAnimation},
        {"__gc", destroyElement<Animation>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<Animation>(L, 1);
             auto e2 = getElement<Animation>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"addPath",
         [](lua_State* L) -> int {
             auto animation = getElement<Animation>(L);
             auto p = lua_touserdata(L, 2);
             if (p != NULL) {
                 auto animationPath = *((std::shared_ptr<AnimationPath>*)p);
                 animation->addPath(animationPath);
             }
             return 0;
         }},
        {"start",
         [](lua_State* L) -> int {
             auto animation = getElement<Animation>(L);
             animation->start();
             return 0;
         }},
        {"stop",
         [](lua_State* L) -> int {
             auto animation = getElement<Animation>(L);
             animation->stop();
             return 0;
         }},
        {"setAnimationDidFinishFn", [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto animation = getElement<Animation>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             animation->setAnimationDidFinishFn([=](Animation* sender) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<Animation>(L, std::shared_ptr<Animation>(sender, BypassDeleter<Animation>()));
                 if (lua_pcall(L, 1, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }}};
    script->bindTable<Animation>("Animation", {animationTableDefinition});

    // LinearAnimationPath
    std::vector<struct luaL_Reg> linearAnimationPathTableDefinition = {
        {"new", createLinearAnimationPath},
        {"__gc", destroyElement<LinearAnimationPath>},
        {"__eq", [](lua_State* L) -> int {
             auto e1 = getElement<LinearAnimationPath>(L, 1);
             auto e2 = getElement<LinearAnimationPath>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }}};
    script->bindTable<LinearAnimationPath>("LinearAnimationPath", {linearAnimationPathTableDefinition});

    // Menu
    std::vector<struct luaL_Reg> menuTableDefinition = {
        {"new", createMenu},
        {"__gc", destroyElement<Menu, View>},
        {"__eq",
         [](lua_State* L) -> int {
             auto e1 = getElement<Menu>(L, 1);
             auto e2 = getElement<Menu>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }},
        {"addMenuItem",
         [](lua_State* L) -> int {
             auto menu = getElement<Menu>(L);
             auto menuItem = getElement<MenuItem>(L, 2);
             menu->addMenuItem(menuItem);
             return 0;
         }},
        {"contentSize",
         [](lua_State* L) -> int {
             auto menu = getElement<Menu>(L);
             auto contentSize = menu->contentSize();

             lua_newtable(L);

             lua_pushnumber(L, contentSize.width);
             lua_setfield(L, -2, "width");

             lua_pushnumber(L, contentSize.height);
             lua_setfield(L, -2, "height");

             return 1;
         }},
        {"popUpContextMenu",
         [](lua_State* L) -> int {
             auto menu = getElement<Menu>(L);
             auto location = getPoint(L, 2);
             auto view = getElement<View>(L, 3);
             Menu::popUpContextMenu(menu, location, view.get());
             return 0;
         }},
        {"closePopUp",
         [](lua_State* L) -> int {
             auto menu = getElement<Menu>(L);
             Menu::closePopUp();
             return 0;
         }},
        {"setDidSelectItemFn", [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto menu = getElement<Menu>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             menu->setDidSelectItemFn([=](Menu* sender, int itemIndex) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<Menu, View>(L, std::shared_ptr<Menu>(sender, BypassDeleter<Menu>()));
                 lua_pushinteger(L, itemIndex + 1);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }}};
    script->bindTable<Menu, View>("Menu", {menuTableDefinition, viewTableContentDefinition});

    // MenuItem
    std::vector<struct luaL_Reg> menuItemTableDefinition = {
        {"new", createMenuItem}, {"__gc", destroyElement<MenuItem>}, {"__eq", [](lua_State* L) -> int {
                                                                          auto e1 = getElement<MenuItem>(L, 1);
                                                                          auto e2 = getElement<MenuItem>(L, 2);
                                                                          lua_pushboolean(L, e1 == e2);
                                                                          return 1;
                                                                      }}};
    script->bindTable<MenuItem>("MenuItem", {menuItemTableDefinition});

    // MenuBar
    std::vector<struct luaL_Reg> menuBarTableDefinition = {{"new", createMenuBar},
                                                           {"__gc", destroyElement<MenuBar, View>},
                                                           {"__eq",
                                                            [](lua_State* L) -> int {
                                                                auto e1 = getElement<MenuBar>(L, 1);
                                                                auto e2 = getElement<MenuBar>(L, 2);
                                                                lua_pushboolean(L, e1 == e2);
                                                                return 1;
                                                            }},
                                                           {"addMenu", [](lua_State* L) -> int {
                                                                auto menuBar = getElement<MenuBar>(L);
                                                                auto menu = getElement<Menu>(L, 2);
                                                                menuBar->addMenu(menu);
                                                                return 0;
                                                            }}};
    script->bindTable<MenuBar, View>("MenuBar", {menuBarTableDefinition, viewTableContentDefinition});

    // Keyboard
    std::vector<struct luaL_Reg> keyboardTableContentDefinition = {
        {"contentSize",
         [](lua_State* L) -> int {
             std::shared_ptr<Keyboard> keyboard = getElement<Keyboard>(L);

             lua_newtable(L);

             lua_pushnumber(L, keyboard->contentSize().width);
             lua_setfield(L, -2, "width");

             lua_pushnumber(L, keyboard->contentSize().height);
             lua_setfield(L, -2, "height");

             return 1;
         }},
        {"setKeyPressedState",
         [](lua_State* L) -> int {
             std::shared_ptr<Keyboard> keyboard = getElement<Keyboard>(L);
             int channel = luaL_checkinteger(L, 2);
             int pitch = luaL_checkinteger(L, 3);
             if (!lua_isboolean(L, 4)) luaL_error(L, "Boolean expected");
             int state = lua_toboolean(L, 4);
             keyboard->setKeyPressedState(channel, pitch, state != 0);
             return 0;
         }},
        {"setIsDrumKit",
         [](lua_State* L) -> int {
             std::shared_ptr<Keyboard> keyboard = getElement<Keyboard>(L);
             if (!lua_isboolean(L, 2)) luaL_error(L, "Boolean expected");
             int isDrumKit = lua_toboolean(L, 2);
             keyboard->setIsDrumKit(isDrumKit != 0);
             return 0;
         }},
        {"setKeyPressedFn",
         [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto keyboard = getElement<Keyboard>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             keyboard->setKeyPressedFn([=](Keyboard* sender, int pitch) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<Keyboard, Control, View>(L,
                                                          std::shared_ptr<Keyboard>(sender, BypassDeleter<Keyboard>()));
                 lua_pushinteger(L, pitch);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }},
        {"setKeyReleasedFn", [](lua_State* L) -> int {
             int callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
             auto keyboard = getElement<Keyboard>(L);
             lua_getglobal(L, "script");
             auto script = (MDStudio::Script*)lua_touserdata(L, -1);
             keyboard->setKeyReleasedFn([=](Keyboard* sender, int pitch) {
                 // Push the callback onto the stack using the Lua reference we stored in the registry
                 lua_rawgeti(L, LUA_REGISTRYINDEX, callbackRef);
                 registerElement<Keyboard, Control, View>(L,
                                                          std::shared_ptr<Keyboard>(sender, BypassDeleter<Keyboard>()));
                 lua_pushinteger(L, pitch);
                 if (lua_pcall(L, 2, 0, 0) != 0) {
                     script->error(lua_tostring(L, -1));
                 }
             });
             return 0;
         }}};

    script->bindTable<Keyboard, Control, View>("Keyboard",
                                               {keyboardTableContentDefinition, viewTableContentDefinition});

    // KeyboardH
    std::vector<struct luaL_Reg> keyboardHTableDefinition = {
        {"new", createKeyboardH},
        {"__gc", destroyElement<KeyboardH, Keyboard, Control, View>},
        {"__eq", [](lua_State* L) -> int {
             auto e1 = getElement<KeyboardH>(L, 1);
             auto e2 = getElement<KeyboardH>(L, 2);
             lua_pushboolean(L, e1 == e2);
             return 1;
         }}};
    script->bindTable<KeyboardH, Keyboard, Control, View>(
        "KeyboardH", {keyboardHTableDefinition, keyboardTableContentDefinition, viewTableContentDefinition});

    //
    // Global
    //

    script->bindFunction("getTopView", &getTopView);

    script->bindFunction("setContentSize", [](lua_State* L) -> int {
        lua_getglobal(L, "uiScriptModule");
        MDStudio::UIScriptModule* uiScriptModule = (MDStudio::UIScriptModule*)lua_touserdata(L, -1);
        uiScriptModule->setContentSize(getSize(L, 1));
        return 0;
    });

    script->bindFunction("asListItemView", [](lua_State* L) -> int {
        auto view = getElement<View>(L);
        auto listItemView = std::dynamic_pointer_cast<ListItemView>(view);
        registerElement<ListItemView, View>(L, listItemView);
        return 1;
    });

    script->bindFunction("asLabelView", [](lua_State* L) -> int {
        auto view = getElement<View>(L);
        auto labelView = std::dynamic_pointer_cast<LabelView>(view);
        registerElement<LabelView, View>(L, labelView);
        return 1;
    });

    script->bindFunction("makePoint", [](lua_State* L) -> int {
        auto x = luaL_checknumber(L, 1);
        auto y = luaL_checknumber(L, 2);

        lua_newtable(L);

        lua_pushnumber(L, x);
        lua_setfield(L, -2, "x");

        lua_pushnumber(L, y);
        lua_setfield(L, -2, "y");

        return 1;
    });

    script->bindFunction("makeSize", [](lua_State* L) -> int {
        auto width = luaL_checknumber(L, 1);
        auto height = luaL_checknumber(L, 2);

        lua_newtable(L);

        lua_pushnumber(L, width);
        lua_setfield(L, -2, "width");

        lua_pushnumber(L, height);
        lua_setfield(L, -2, "height");

        return 1;
    });

    script->bindFunction("makeRect", [](lua_State* L) -> int {
        auto x = luaL_checknumber(L, 1);
        auto y = luaL_checknumber(L, 2);
        auto width = luaL_checknumber(L, 3);
        auto height = luaL_checknumber(L, 4);

        lua_newtable(L);

        lua_newtable(L);

        lua_pushnumber(L, x);
        lua_setfield(L, -2, "x");

        lua_pushnumber(L, y);
        lua_setfield(L, -2, "y");

        lua_setfield(L, -2, "origin");

        lua_newtable(L);

        lua_pushnumber(L, width);
        lua_setfield(L, -2, "width");

        lua_pushnumber(L, height);
        lua_setfield(L, -2, "height");

        lua_setfield(L, -2, "size");

        return 1;
    });

    script->bindFunction("makeColor", [](lua_State* L) -> int {
        auto red = luaL_checknumber(L, 1);
        auto green = luaL_checknumber(L, 2);
        auto blue = luaL_checknumber(L, 3);
        auto alpha = luaL_checknumber(L, 4);

        lua_newtable(L);

        lua_pushnumber(L, red);
        lua_setfield(L, -2, "red");

        lua_pushnumber(L, green);
        lua_setfield(L, -2, "green");

        lua_pushnumber(L, blue);
        lua_setfield(L, -2, "blue");

        lua_pushnumber(L, alpha);
        lua_setfield(L, -2, "alpha");

        return 1;
    });

    script->bindFunction("parseSVGPath", [](lua_State* L) -> int {
        auto path = getElement<Path>(L, 1);
        auto s = luaL_checkstring(L, 2);

        parseSVGPath(path.get(), s);
        return 0;
    });
}
