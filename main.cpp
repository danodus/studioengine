//
//  main.cpp
//  studioengine
//
//  Created by Daniel Cliche on 2020-12-05.
//  Copyright (c) 2020-2021 Daniel Cliche. All rights reserved.
//

#include <studio.h>

#if _WIN32
#include <SDL.h>
#include <SDL_opengl.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#endif

#include <audioscriptmodule.h>
#include <melobasecorescriptmodule.h>
#include <draw.h>
#include <menubar.h>
#include <picojson.h>
#include <platform.h>
#include <ui.h>
#include <window.h>

#include <cassert>
#include <fstream>
#include <iostream>

#ifndef _WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif

#include "Common/helpers.h"
#include "Common/melobasescriptmodule.h"
#include "cursors.h"

// ---------------------------------------------------------------------------------------------------------------------
static SDL_Cursor* createCursorFromImage(const char* image[]) {
    int i, row, col;
    Uint8 data[4 * 32];
    Uint8 mask[4 * 32];
    int hot_x, hot_y;

    i = -1;
    for (row = 0; row < 32; ++row) {
        for (col = 0; col < 32; ++col) {
            if (col % 8) {
                data[i] <<= 1;
                mask[i] <<= 1;
            } else {
                ++i;
                data[i] = mask[i] = 0;
            }
            switch (image[4 + row][col]) {
                case 'X':
                    data[i] |= 0x01;
                    mask[i] |= 0x01;
                    break;
                case '.':
                    mask[i] |= 0x01;
                    break;
                case ' ':
                    break;
            }
        }
    }
    sscanf(image[4 + row], "%d,%d", &hot_x, &hot_y);
    auto cursor = SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
    if (!cursor) {
        std::cout << SDL_GetError() << "\n";
    }
    return cursor;
}

class EmbeddedScriptModule : public MDStudio::ScriptModule {
   public:
    typedef std::function<void(EmbeddedScriptModule* sender)> QuitFnType;

   private:
    MDStudio::MenuBar* _menuBar;

    QuitFnType _quitFn;

    MDStudio::MenuBar* menuBar() { return _menuBar; };
    void quit() {
        if (_quitFn) _quitFn(this);
    }

   public:
    EmbeddedScriptModule(MDStudio::MenuBar* menuBar) : _menuBar(menuBar) {}

    void setQuitFn(QuitFnType quitFn) { _quitFn = quitFn; }

    void init(MDStudio::Script* script) override {
        script->setGlobal("embeddedScriptModule", this);

        script->bindFunction("quit", [](lua_State* L) -> int {
            lua_getglobal(L, "embeddedScriptModule");
            auto embeddedScriptModule = static_cast<EmbeddedScriptModule*>(lua_touserdata(L, -1));
            embeddedScriptModule->quit();
            return 0;
        });

        script->bindFunction("getMenuBar", [](lua_State* L) -> int {
            lua_getglobal(L, "embeddedScriptModule");
            auto embeddedScriptModule = static_cast<EmbeddedScriptModule*>(lua_touserdata(L, -1));
            auto menuBar = std::shared_ptr<MDStudio::MenuBar>(embeddedScriptModule->menuBar(),
                                                              MDStudio::BypassDeleter<MDStudio::MenuBar>());
            MDStudio::registerElement<MDStudio::MenuBar, MDStudio::View>(L, menuBar);
            return 1;
        });
    };
};

class Window : public MDStudio::View {
   public:
    typedef std::function<void(Window* sender)> DidResignKeyWindowFnType;

   private:
    std::shared_ptr<MDStudio::View> _contentView, _backgroundView;

    DidResignKeyWindowFnType _didResignKeyWindowFn = nullptr;

    bool _isKeyWindow = false;

   public:
    Window(MDStudio::View* contentView, bool isKeyWindow)
        : MDStudio::View("windowView", nullptr), _isKeyWindow(isKeyWindow) {
        _contentView = std::shared_ptr<MDStudio::View>(contentView, [](MDStudio::View*) {});
        _backgroundView = std::make_shared<MDStudio::View>("windowBackgroundView", this);
        _backgroundView->setHandleEventFn([](View* sender, const MDStudio::UIEvent* event) {
            return MDStudio::isMouseEvent(event) && MDStudio::isPointInRect(event->pt, sender->resolvedClippedRect());
        });
        addSubview(_backgroundView);
        addSubview(_contentView);
    }

    void show(DidResignKeyWindowFnType didResignKeyWindowFn) {
        _didResignKeyWindowFn = didResignKeyWindowFn;
        // responderChain()->captureResponder(this);
        // if (_isKeyWindow) {
        //    responderChain()->makeFirstResponder(this);
        //}
    }

    void hide() {
        // if (_isKeyWindow) {
        //    responderChain()->makeFirstResponder(nullptr);
        //}
        // responderChain()->releaseResponder(this);
    }

    bool handleEvent(const MDStudio::UIEvent* event) override {
        if (event->type == MDStudio::MOUSE_DOWN_UIEVENT || event->type == MDStudio::MOUSE_UP_UIEVENT) {
            if (!MDStudio::isPointInRect(event->pt, resolvedClippedRect())) {
                // if (_isKeyWindow) responderChain()->makeFirstResponder(nullptr);
                // responderChain()->releaseResponder(this);
                if (_didResignKeyWindowFn && (event->type == MDStudio::MOUSE_UP_UIEVENT)) _didResignKeyWindowFn(this);
                if (_isKeyWindow) return true;
            }
        }
        return false;
    }

    void setFrame(MDStudio::Rect frame) {
        MDStudio::View::setFrame(frame);
        _contentView->setFrame(bounds());
        _backgroundView->setFrame(bounds());
    }
};

float g_windowWidth = 800;
float g_windowHeight = 480;

int g_windowX = SDL_WINDOWPOS_UNDEFINED_DISPLAY(0);
int g_windowY = SDL_WINDOWPOS_UNDEFINED_DISPLAY(0);

float g_scale = 1.0f;
bool g_fullScreen = false;
bool g_menuBarHidden = false;

bool g_loadOnStart = false;

Uint32 g_invokeEventType, g_drawEventType;
MDStudio::DrawContext g_drawContextTop, g_drawContextContent;
std::shared_ptr<MDStudio::View> g_topView, g_contentView;
bool g_isDirtySetTop = false, g_isDirtySetContent = false;
MDStudio::Rect g_dirtyRectTop{}, g_dirtyRectContent{};
std::map<void*, std::shared_ptr<Window>> g_windows;
std::shared_ptr<MDStudio::MenuBar> g_menuBar;

SDL_Cursor* g_arrowCursor;
SDL_Cursor* g_resizeLeftRightCursor;
SDL_Cursor* g_resizeUpDownCursor;
SDL_Cursor* g_openHandCursor;
SDL_Cursor* g_pointingHandCursor;
SDL_Cursor* g_crosshairCursor;
SDL_Cursor* g_iBeamCursor;
SDL_Cursor* g_pencilCursor;

// ---------------------------------------------------------------------------------------------------------------------
void invokeAdded() {
    SDL_Event event;
    SDL_memset(&event, 0, sizeof(event)); /* or SDL_zero(event) */
    event.type = g_invokeEventType;
    event.user.code = 0;
    event.user.data1 = 0;
    event.user.data2 = 0;
    SDL_PushEvent(&event);
}

// ---------------------------------------------------------------------------------------------------------------------
void resizeGLScene(int width, int height) {
    MDStudio::setViewport(MDStudio::makeRect(0, 0, width, height));

    glMatrixMode(GL_PROJECTION);  // Select The Projection Matrix
    glLoadIdentity();             // Reset The Projection Matrix

    glOrtho(0, width / MDStudio::backingStoreScale(), 0, height / MDStudio::backingStoreScale(), 0, 100);

    glMatrixMode(GL_MODELVIEW);  // Select The Modelview Matrix
    glLoadIdentity();
}

// ---------------------------------------------------------------------------------------------------------------------
void dirtySetTop(MDStudio::View* sender, MDStudio::Rect dirtyRect) {
    if (!g_isDirtySetTop) {
        g_isDirtySetTop = true;
        g_dirtyRectTop = dirtyRect;

        SDL_Event event;
        SDL_memset(&event, 0, sizeof(event)); /* or SDL_zero(event) */
        event.type = g_drawEventType;
        event.user.code = 0;
        event.user.data1 = 0;
        event.user.data2 = 0;
        SDL_PushEvent(&event);
    } else {
        g_dirtyRectTop = MDStudio::makeUnionRect(g_dirtyRectTop, dirtyRect);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void dirtySetContent(MDStudio::View* sender, MDStudio::Rect dirtyRect) {
    if (!g_isDirtySetContent) {
        g_isDirtySetContent = true;
        g_dirtyRectContent = dirtyRect;

        SDL_Event event;
        SDL_memset(&event, 0, sizeof(event)); /* or SDL_zero(event) */
        event.type = g_drawEventType;
        event.user.code = 0;
        event.user.data1 = 0;
        event.user.data2 = 0;
        SDL_PushEvent(&event);
    } else {
        g_dirtyRectContent = MDStudio::makeUnionRect(g_dirtyRectContent, dirtyRect);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void createWindow(void* owner, float x, float y, float width, float height, MDStudio::View* contentView,
                  bool isKeyWindow) {
    auto window = std::make_shared<Window>(contentView, isKeyWindow);
    g_topView->addSubview(window);
    window->setFrame(MDStudio::makeRect(x, y, width, height));

    g_topView->updateResponderChain();

    window->show([owner](Window* sender) {
        auto w = static_cast<MDStudio::Window*>(owner);
        w->sendDidResignKeyWindow();
    });

    g_windows[owner] = window;

    g_topView->setFrame(g_topView->frame());
}

// ---------------------------------------------------------------------------------------------------------------------
void destroyWindow(void* owner) {
    g_windows[owner]->hide();
    g_topView->removeSubview(g_windows[owner]);
    g_windows.erase(owner);
    g_topView->setDirty();
}

// ---------------------------------------------------------------------------------------------------------------------
void layout(MDStudio::View* sender, MDStudio::Rect rect) {
    auto r = sender->bounds();
    auto menuBarHeight = g_menuBar->isVisible() ? 20.0f : 0.0f;
    g_menuBar->setFrame(MDStudio::makeRect(0, r.size.height - menuBarHeight, r.size.width, menuBarHeight));
}

// ---------------------------------------------------------------------------------------------------------------------
void handleKey(const SDL_Event& event, bool isKeyDown) {
    if ((event.key.keysym.scancode == SDL_SCANCODE_LCTRL) || (event.key.keysym.scancode == SDL_SCANCODE_RCTRL) ||
        (event.key.keysym.scancode == SDL_SCANCODE_UP) || (event.key.keysym.scancode == SDL_SCANCODE_DOWN) ||
        (event.key.keysym.scancode == SDL_SCANCODE_LEFT) || (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) ||
        (event.key.keysym.scancode == SDL_SCANCODE_DELETE) || (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) ||
        (event.key.keysym.scancode == SDL_SCANCODE_RETURN) || (event.key.keysym.scancode == SDL_SCANCODE_TAB) ||
        (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) || (event.key.keysym.scancode == SDL_SCANCODE_HOME) ||
        (event.key.keysym.scancode == SDL_SCANCODE_END || (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT) ||
         (event.key.keysym.scancode == SDL_SCANCODE_RSHIFT))) {
        MDStudio::UIEvent uiEvent;
        uiEvent.type = isKeyDown ? MDStudio::KEY_UIEVENT : MDStudio::KEY_UP_UIEVENT;
        switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_LCTRL:
            case SDL_SCANCODE_RCTRL:
                uiEvent.key = KEY_CONTROL;
                break;
            case SDL_SCANCODE_UP:
                uiEvent.key = KEY_UP;
                break;
            case SDL_SCANCODE_DOWN:
                uiEvent.key = KEY_DOWN;
                break;
            case SDL_SCANCODE_LEFT:
                uiEvent.key = KEY_LEFT;
                break;
            case SDL_SCANCODE_RIGHT:
                uiEvent.key = KEY_RIGHT;
                break;
            case SDL_SCANCODE_DELETE:
                uiEvent.key = KEY_DELETE;
                break;
            case SDL_SCANCODE_BACKSPACE:
                uiEvent.key = KEY_BACKSPACE;
                break;
            case SDL_SCANCODE_RETURN:
                uiEvent.key = KEY_ENTER;
                break;
            case SDL_SCANCODE_TAB:
                uiEvent.key = KEY_TAB;
                break;
            case SDL_SCANCODE_ESCAPE:
                uiEvent.key = KEY_ESCAPE;
                break;
            case SDL_SCANCODE_HOME:
                uiEvent.key = KEY_HOME;
                break;
            case SDL_SCANCODE_END:
                uiEvent.key = KEY_END;
                break;
        }
        uiEvent.characters = "";
        uiEvent.isARepeat = event.key.repeat != 0;
        uiEvent.modifierFlags = 0;
        if (event.key.keysym.mod & KMOD_SHIFT) uiEvent.modifierFlags |= MODIFIER_FLAG_SHIFT;
        if (event.key.keysym.mod & KMOD_CTRL) {
            uiEvent.modifierFlags |= MODIFIER_FLAG_CONTROL;
            uiEvent.modifierFlags |= MODIFIER_FLAG_COMMAND;
        }
        if (event.key.keysym.mod & KMOD_ALT) uiEvent.modifierFlags |= MODIFIER_FLAG_ALTERNATE;

        if (!g_topView->responderChain()->sendEvent(&uiEvent)) g_contentView->responderChain()->sendEvent(&uiEvent);
    } else if (((event.key.keysym.scancode >= SDL_SCANCODE_A && event.key.keysym.scancode <= SDL_SCANCODE_Z) ||
                (event.key.keysym.scancode >= SDL_SCANCODE_1 && event.key.keysym.scancode <= SDL_SCANCODE_0))) {
        // Handle alpha-numerical key events not handled by text input

        char c;
        if (event.key.keysym.scancode >= SDL_SCANCODE_A && event.key.keysym.scancode <= SDL_SCANCODE_Z) {
            c = 'a' + (event.key.keysym.scancode - SDL_SCANCODE_A);
            if (event.key.keysym.mod & KMOD_SHIFT) c = std::toupper(c);
        } else if (event.key.keysym.scancode == SDL_SCANCODE_0) {
            c = '0';
        } else {
            c = '1' + (event.key.keysym.scancode - SDL_SCANCODE_1);
        }

        MDStudio::UIEvent uiEvent;
        uiEvent.type = isKeyDown ? MDStudio::KEY_UIEVENT : MDStudio::KEY_UP_UIEVENT;
        uiEvent.characters = c;
        uiEvent.isARepeat = event.key.repeat != 0;
        uiEvent.modifierFlags = 0;
        if (event.key.keysym.mod & KMOD_SHIFT) uiEvent.modifierFlags |= MODIFIER_FLAG_SHIFT;
        if (event.key.keysym.mod & KMOD_CTRL) {
            uiEvent.modifierFlags |= MODIFIER_FLAG_CONTROL;
            uiEvent.modifierFlags |= MODIFIER_FLAG_COMMAND;
        }
        if (event.key.keysym.mod & KMOD_ALT) uiEvent.modifierFlags |= MODIFIER_FLAG_ALTERNATE;

        if (!g_topView->responderChain()->sendEvent(&uiEvent)) g_contentView->responderChain()->sendEvent(&uiEvent);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
std::string currentPath() {
    char cwd[FILENAME_MAX];
    getcwd(cwd, sizeof(cwd));
    return cwd;
}

// ---------------------------------------------------------------------------------------------------------------------
std::string parentPath(const std::string& path) {
    size_t found;
    found = path.find_last_of("/\\");
    auto parent = path.substr(0, found);

    return parent;
}

// ---------------------------------------------------------------------------------------------------------------------
bool readConfig(const std::string& path) {
    auto ifs = std::ifstream(path);
    if (ifs.is_open()) {
        picojson::value v;
        std::string err;
        picojson::parse(v, std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>(), &err);
        if (!err.empty()) {
            std::cerr << err << std::endl;
            return false;
        }
        auto o = v.get<picojson::object>();

        int display = 0;

        if (o.find("display") != o.end()) display = static_cast<int>(o["display"].get<double>());
        g_windowX = SDL_WINDOWPOS_UNDEFINED_DISPLAY(display);
        g_windowY = SDL_WINDOWPOS_UNDEFINED_DISPLAY(display);
        if (o.find("width") != o.end()) g_windowWidth = static_cast<int>(o["width"].get<double>());
        if (o.find("height") != o.end()) g_windowHeight = static_cast<int>(o["height"].get<double>());
        if (o.find("scale") != o.end()) g_scale = static_cast<float>(o["scale"].get<double>());
        if (o.find("loadOnStart") != o.end()) g_loadOnStart = static_cast<bool>(o["loadOnStart"].get<bool>());
        if (o.find("fullScreen") != o.end()) g_fullScreen = static_cast<bool>(o["fullScreen"].get<bool>());
        if (o.find("menuBarHidden") != o.end()) g_menuBarHidden = static_cast<bool>(o["menuBarHidden"].get<bool>());
    }
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
void loadScript(MDStudio::UI* ui, const std::string& path, std::vector<MDStudio::ScriptModule*>& modules,
                bool isDebug) {
    g_menuBar->removeAllMenus();
    g_contentView->removeAllSubviews();
    g_contentView->setDrawFn(nullptr);
    g_contentView->setLayoutFn(nullptr);
    ui->loadUI(g_contentView.get(), path, nullptr, modules, isDebug);
    g_topView->setFrame(g_topView->frame());
    g_contentView->setFrame(g_contentView->frame());
}

// ---------------------------------------------------------------------------------------------------------------------
void cursorSet(const MDStudio::ResponderChain* responderChain, MDStudio::Platform::CursorEnumType cursor) {
    std::map<MDStudio::Platform::CursorEnumType, SDL_Cursor*> m{
        {MDStudio::Platform::CursorEnumType::ArrowCursor, g_arrowCursor},
        {MDStudio::Platform::CursorEnumType::ResizeLeftRightCursor, g_resizeLeftRightCursor},
        {MDStudio::Platform::CursorEnumType::ResizeUpDownCursor, g_resizeUpDownCursor},
        {MDStudio::Platform::CursorEnumType::OpenHandCursor, g_openHandCursor},
        {MDStudio::Platform::CursorEnumType::PointingHandCursor, g_pointingHandCursor},
        {MDStudio::Platform::CursorEnumType::CrosshairCursor, g_crosshairCursor},
        {MDStudio::Platform::CursorEnumType::IBeamCursor, g_iBeamCursor},
        {MDStudio::Platform::CursorEnumType::PencilCursor, g_pencilCursor}};

    if (m.find(cursor) != m.end()) {
        SDL_SetCursor(m[cursor]);
    } else {
        SDL_SetCursor(g_arrowCursor);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv) {
    auto currentPath = ::currentPath();
    readConfig(currentPath + "/config.json");

    auto path = currentPath + "/Resources/main.lua";

    if (argc >= 2)
        path = std::string(argv[1]);

    MDStudio::Platform::sharedInstance()->setInvokeFunctionAddedFn(invokeAdded);
    MDStudio::Platform::sharedInstance()->setCreateWindowFn(createWindow);
    MDStudio::Platform::sharedInstance()->setDestroyWindowFn(destroyWindow);

    SDL_Window* sdlWindow;

    SDL_Init(SDL_INIT_VIDEO);  // Initialize SDL2

    g_arrowCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    g_resizeLeftRightCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
    g_resizeUpDownCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
    g_openHandCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    g_pointingHandCursor = createCursorFromImage(g_pointingHandCursorImage);
    g_crosshairCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
    g_iBeamCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    g_pencilCursor = createCursorFromImage(g_pencilCursorImage);

    MDStudio::Platform::sharedInstance()->setCursorSetFn(cursorSet);

    SDL_EnableScreenSaver();

    auto firstEventType = SDL_RegisterEvents(2);
    assert(firstEventType != ((Uint32)-1));

    g_invokeEventType = firstEventType;
    g_drawEventType = firstEventType + 1;

    UInt32 windowFlags = SDL_WINDOW_OPENGL;
    if (g_fullScreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    } else {
        windowFlags |= SDL_WINDOW_RESIZABLE;
    }

    // Create an application window with the following settings:
    sdlWindow = SDL_CreateWindow("Studio Engine", // window title
                                 g_windowX,       // initial x position
                                 g_windowY,       // initial y position
                                 g_windowWidth,   // width, in pixels
                                 g_windowHeight,  // height, in pixels
                                 windowFlags      // flags
    );

    // Check that the window was successfully created
    if (sdlWindow == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    // Get render buffer width/height
    int renderWidth, renderHeight;
    SDL_GL_GetDrawableSize(sdlWindow, &renderWidth, &renderHeight);

    // Create SDL GL context
    auto sdlGLContextDrawing = SDL_GL_CreateContext(sdlWindow);
    assert(sdlGLContextDrawing);
    SDL_GL_MakeCurrent(sdlWindow, sdlGLContextDrawing);
    SDL_GL_SetSwapInterval(1);

    MDStudio::setBackingStoreScale(g_scale * static_cast<float>(renderWidth) / g_windowWidth);

    resizeGLScene(renderWidth, renderHeight);

    g_topView = std::make_shared<MDStudio::View>("topView", nullptr);
    g_topView->setDrawContext(&g_drawContextTop);
    g_topView->createResponderChain();
    g_topView->createTooltipManager();
    g_topView->setDirtySetFn(dirtySetTop);
    g_topView->setLayoutFn(layout);

    g_menuBar = std::make_shared<MDStudio::MenuBar>("menuBar", nullptr);
    g_topView->addSubview(g_menuBar);

    if (g_menuBarHidden) g_menuBar->setIsVisible(false);

    g_contentView = std::make_shared<MDStudio::View>("contentView", nullptr);
    g_contentView->setDrawContext(&g_drawContextContent);
    g_contentView->createResponderChain();
    g_contentView->createTooltipManager();
    g_contentView->setDirtySetFn(dirtySetContent);

    g_topView->setFrame(MDStudio::makeRect(0, 0, g_windowWidth / g_scale, g_windowHeight / g_scale));
    g_contentView->setFrame(MDStudio::makeRect(0, 0, g_windowWidth / g_scale,
                                               g_windowHeight / g_scale - (g_menuBar->isVisible() ? 20.0f : 0.0f)));

    MDStudio::Platform::sharedInstance()->setResourcesPath(parentPath(path));
    MDStudio::Platform::sharedInstance()->setDataPath(currentPath);

    MDStudio::UI* ui;

    MDStudio::AudioScriptModule audioScriptModule;
    MelobaseCore::MelobaseCoreScriptModule melobaseCoreScriptModule;
    MelobaseScriptModule melobaseScriptModule(nullptr, nullptr);
    EmbeddedScriptModule embeddedScriptModule(g_menuBar.get());

    std::vector<MDStudio::ScriptModule*> modules = {&audioScriptModule, &melobaseCoreScriptModule,
                                                    &melobaseScriptModule, &embeddedScriptModule};

    loadCommonScript();                                                    

    ui = new MDStudio::UI;

    if (g_loadOnStart) loadScript(ui, path, modules, false);

    bool running = true;

    embeddedScriptModule.setQuitFn([&running](EmbeddedScriptModule* sender) { running = false; });

    SDL_SetCursor(g_arrowCursor);

    // Handle SDL events
    SDL_Event event;
    while (running) {
        if (SDL_WaitEvent(&event)) {
            if (event.type == g_invokeEventType) {
                MDStudio::Platform::sharedInstance()->process();
            } else if (event.type == g_drawEventType) {
                // Draw content layer
                g_contentView->drawSubviews(g_contentView->bounds());
                g_contentView->draw();
                g_contentView->drawContext()->draw();
                g_isDirtySetContent = false;
                g_dirtyRectContent = MDStudio::makeZeroRect();

                // Draw top layer
                g_topView->drawSubviews(g_topView->bounds(), true);
                g_topView->draw();
                g_topView->drawContext()->draw();
                g_isDirtySetTop = false;
                g_dirtyRectTop = MDStudio::makeZeroRect();

                SDL_GL_SwapWindow(sdlWindow);
            } else {
                switch (event.type) {
                    case SDL_WINDOWEVENT: {
                        switch (event.window.event) {
                            case SDL_WINDOWEVENT_CLOSE:
                                running = false;
                                break;
                            case SDL_WINDOWEVENT_RESIZED:
                                g_windowWidth = event.window.data1;
                                g_windowHeight = event.window.data2;
                                SDL_GL_GetDrawableSize(sdlWindow, &renderWidth, &renderHeight);
                                resizeGLScene(renderWidth, renderHeight);

                                g_topView->setFrame(
                                    MDStudio::makeRect(0, 0, g_windowWidth / g_scale, g_windowHeight / g_scale));
                                g_contentView->setFrame(MDStudio::makeRect(
                                    0, 0, g_windowWidth / g_scale,
                                    g_windowHeight / g_scale - (g_menuBar->isVisible() ? 20.0f : 0.0f)));
                                break;
                        }
                        break;
                    }
                    case SDL_FINGERDOWN: {
                        MDStudio::UIEvent uiEvent{};
                        uiEvent.type = MDStudio::MOUSE_DOWN_UIEVENT;
                        uiEvent.pt.x = (event.tfinger.x * g_windowWidth) / g_scale;
                        uiEvent.pt.y = (g_windowHeight - (event.tfinger.y * g_windowHeight)) / g_scale;
                        uiEvent.modifierFlags = 0;
                        if (!g_topView->responderChain()->sendEvent(&uiEvent))
                            g_contentView->responderChain()->sendEvent(&uiEvent);
                        break;
                    }
                    case SDL_MOUSEBUTTONDOWN: {
                        MDStudio::UIEvent uiEvent{};
                        uiEvent.type = event.button.button == 3 ? MDStudio::RIGHT_MOUSE_DOWN_UIEVENT
                                                                : MDStudio::MOUSE_DOWN_UIEVENT;
                        uiEvent.pt.x = event.button.x / g_scale;
                        uiEvent.pt.y = (g_windowHeight - event.button.y) / g_scale;
                        uiEvent.modifierFlags = 0;
                        auto mod = SDL_GetModState();
                        if (mod & KMOD_SHIFT) uiEvent.modifierFlags |= MODIFIER_FLAG_SHIFT;
                        if (mod & KMOD_CTRL) {
                            uiEvent.modifierFlags |= MODIFIER_FLAG_CONTROL;
                            uiEvent.modifierFlags |= MODIFIER_FLAG_COMMAND;
                        }
                        if (mod & KMOD_ALT) uiEvent.modifierFlags |= MODIFIER_FLAG_ALTERNATE;
                        if (mod & KMOD_GUI) uiEvent.modifierFlags |= MODIFIER_FLAG_COMMAND;
                        if (!g_topView->responderChain()->sendEvent(&uiEvent))
                            g_contentView->responderChain()->sendEvent(&uiEvent);
                        break;
                    }
                    case SDL_FINGERUP: {
                        MDStudio::UIEvent uiEvent{};
                        uiEvent.type = MDStudio::MOUSE_UP_UIEVENT;
                        uiEvent.pt.x = (event.tfinger.x * g_windowWidth) / g_scale;
                        uiEvent.pt.y = (g_windowHeight - (event.tfinger.y * g_windowHeight)) / g_scale;
                        uiEvent.modifierFlags = 0;
                        if (!g_topView->responderChain()->sendEvent(&uiEvent))
                            g_contentView->responderChain()->sendEvent(&uiEvent);
                        break;
                    }
                    case SDL_MOUSEBUTTONUP: {
                        MDStudio::UIEvent uiEvent{};
                        uiEvent.type =
                            event.button.button == 3 ? MDStudio::RIGHT_MOUSE_UP_UIEVENT : MDStudio::MOUSE_UP_UIEVENT;
                        uiEvent.pt.x = event.button.x / g_scale;
                        uiEvent.pt.y = (g_windowHeight - event.button.y) / g_scale;
                        uiEvent.modifierFlags = 0;
                        auto mod = SDL_GetModState();
                        if (mod & KMOD_SHIFT) uiEvent.modifierFlags |= MODIFIER_FLAG_SHIFT;
                        if (mod & KMOD_CTRL) {
                            uiEvent.modifierFlags |= MODIFIER_FLAG_CONTROL;
                            uiEvent.modifierFlags |= MODIFIER_FLAG_COMMAND;
                        }
                        if (mod & KMOD_ALT) uiEvent.modifierFlags |= MODIFIER_FLAG_ALTERNATE;
                        if (!g_topView->responderChain()->sendEvent(&uiEvent))
                            g_contentView->responderChain()->sendEvent(&uiEvent);
                        break;
                    }
                    case SDL_FINGERMOTION: {
                        MDStudio::UIEvent uiEvent{};
                        uiEvent.type = MDStudio::MOUSE_MOVED_UIEVENT;
                        uiEvent.pt.x = (event.tfinger.x * g_windowWidth) / g_scale;
                        uiEvent.pt.y = (g_windowHeight - (event.tfinger.y * g_windowHeight)) / g_scale;
                        uiEvent.modifierFlags = 0;
                        if (!g_topView->responderChain()->sendEvent(&uiEvent))
                            g_contentView->responderChain()->sendEvent(&uiEvent);
                        break;
                    }
                    case SDL_MOUSEMOTION: {
                        MDStudio::UIEvent uiEvent{};
                        uiEvent.type = MDStudio::MOUSE_MOVED_UIEVENT;
                        uiEvent.pt.x = event.motion.x / g_scale;
                        uiEvent.pt.y = (g_windowHeight - event.motion.y) / g_scale;
                        uiEvent.modifierFlags = 0;
                        if (!g_topView->responderChain()->sendEvent(&uiEvent))
                            g_contentView->responderChain()->sendEvent(&uiEvent);
                        break;
                    }
                    case SDL_KEYDOWN: {
                        if (event.key.keysym.scancode == SDL_SCANCODE_F9 ||
                            event.key.keysym.scancode == SDL_SCANCODE_F10) {
                            loadScript(ui, path, modules, event.key.keysym.scancode == SDL_SCANCODE_F9);
                        } else if (event.key.keysym.scancode == SDL_SCANCODE_PAUSE &&
                                   (event.key.keysym.mod & KMOD_CTRL)) {
                            running = false;
                        } else {
                            handleKey(event, true);
                        }
                        break;
                    }
                    case SDL_KEYUP: {
                        if (event.key.keysym.scancode != SDL_SCANCODE_F11 &&
                            event.key.keysym.scancode != SDL_SCANCODE_F12)
                            handleKey(event, false);
                        break;
                    }
                    case SDL_TEXTINPUT: {
                        // Alphanumeric events are handled by handleKey()
                        auto characters = std::string(event.text.text);
                        if (characters.length() > 0 && !std::isalnum(characters[0])) {
                            MDStudio::UIEvent uiEvent{};
                            uiEvent.type = MDStudio::KEY_UIEVENT;
                            uiEvent.key = 0;
                            uiEvent.characters = characters;
                            uiEvent.modifierFlags = 0;
                            uiEvent.isARepeat = false;
                            if (!g_topView->responderChain()->sendEvent(&uiEvent))
                                g_contentView->responderChain()->sendEvent(&uiEvent);
                        }
                        break;
                    }
                    case SDL_MOUSEWHEEL: {
                        MDStudio::UIEvent uiEvent{};
                        uiEvent.type = MDStudio::SCROLL_UIEVENT;
                        int x, y;
                        SDL_GetMouseState(&x, &y);
                        uiEvent.pt.x = x / g_scale;
                        uiEvent.pt.y = (g_windowHeight - y) / g_scale;
                        uiEvent.modifierFlags = 0;
                        uiEvent.deltaY = event.wheel.y * 20.0f;
                        if (!g_topView->responderChain()->sendEvent(&uiEvent))
                            g_contentView->responderChain()->sendEvent(&uiEvent);
                        break;
                    }
                }
            }
        }
    }

    g_contentView->removeAllSubviews();
    g_topView->removeAllSubviews();

    delete ui;

    g_contentView = nullptr;
    g_topView = nullptr;

    SDL_GL_DeleteContext(sdlGLContextDrawing);

    // Close and destroy the window
    SDL_DestroyWindow(sdlWindow);

    // Destroy cursors
    SDL_FreeCursor(g_arrowCursor);
    SDL_FreeCursor(g_resizeUpDownCursor);
    SDL_FreeCursor(g_openHandCursor);
    SDL_FreeCursor(g_pointingHandCursor);
    SDL_FreeCursor(g_crosshairCursor);
    SDL_FreeCursor(g_iBeamCursor);
    SDL_FreeCursor(g_pencilCursor);

    // Clean up
    SDL_Quit();
    return 0;
}
