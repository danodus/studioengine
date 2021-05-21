//
//  platform.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-08-09.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef PLATFORM_H
#define PLATFORM_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "utf8.h"

#ifdef _WIN32
time_t timegm(struct tm* tm);
#endif

namespace MDStudio {

class View;
class ResponderChain;

/// \brief Get a timestamp
/// \details Get the number of seconds since January 1st 2001.
/// \return The number of seconds.
double getTimestamp();

/// \brief Convert a timestamp to date & time
/// \details Convert a timestamp to calendar date and time
/// \param timestamp The number of seconds since January 1st 2001.
/// \return The calendar date and time if valid, null otherwise
/// \note May not be thread-safe.
std::tm *timestampToDateTime(double timestamp);

/// \brief Swap 16-bit value from host to big endian
/// \details Swap the 16-bit value from host to big endian
/// \param s 16-bit value
/// \return Swapped value
unsigned short swapUInt16HostToBig(unsigned short s);

/// \brief Swap 32-bit value from host to big endian
/// \details Swap the 32-bit value from host to big endian
/// \param i 32-bit value
/// \return Swapped value
unsigned int swapUInt32HostToBig(unsigned int i);

/// \brief Encore the value using VLE
/// \details Encore the value using variable-length encoding
/// \param value 32-bit value
/// \return Array of bytes
std::vector<unsigned char> vlEncode(unsigned int value);

struct Invoke {
    void* owner;
    std::function<void()> fn;
};

struct DelayedInvoke {
    Invoke invoke;
    double timestamp;
};

/// \brief Platform class
/// \details Platform class acting as a bridge with the platform.
class Platform {
   public:
    /// Types of cursors
    /// @sa setCursor()
    typedef enum {
        ArrowCursor,            ///< Arrow cursor
        ResizeLeftRightCursor,  ///< Resize left cursor
        ResizeUpDownCursor,     ///< Resize up and down cursor
        OpenHandCursor,         ///< Open hand cursor
        PointingHandCursor,     ///< Pointing hand cursor
        CrosshairCursor,        ///< Crosshair cursor
        IBeamCursor,            ///< I-Beam cursor
        PencilCursor,           ///< Pencil cursor
    } CursorEnumType;
    typedef std::function<void()> InvokeFunctionAddedFnType;
    typedef std::function<void(const ResponderChain* responderChain, CursorEnumType cursor)> CursorSetFnType;
    typedef std::function<void(const std::string& name)> DocumentNameSetFnType;
    typedef std::function<void(const std::string& title, const std::string& message, bool isFatal)> AlertFnType;
    typedef std::function<void()> BeepFnType;
    typedef std::function<void(void* owner, float x, float y, float width, float height, MDStudio::View* contentView,
                               bool isKeyWindow)>
        CreateWindowFnType;
    typedef std::function<void(void* owner)> MakeKeyWindowFnType;
    typedef std::function<void(void* owner)> DestroyWindowFnType;
    typedef std::function<void(const std::string& content)> SetPasteboardContentFnType;
    typedef std::function<std::string()> GetPasteboardContentFnType;
    typedef std::function<void(const std::string& message)> ExceptionHandlerFnType;
    typedef std::function<std::string(double timestamp)> TimestampToStrFnType;

   private:
    std::queue<Invoke> _invokeFns;
    std::vector<DelayedInvoke> _delayedInvokes;

    InvokeFunctionAddedFnType _invokeFunctionAddedFn;
    CursorSetFnType _cursorSetFn;
    DocumentNameSetFnType _documentNameSetFn;
    AlertFnType _alertFn;
    BeepFnType _beepFn;
    CreateWindowFnType _createWindowFn;
    MakeKeyWindowFnType _makeKeyWindowFn;
    DestroyWindowFnType _destroyWindowFn;
    SetPasteboardContentFnType _setPasteboardContentFn;
    GetPasteboardContentFnType _getPasteboardContentFn;
    ExceptionHandlerFnType _exceptionHandlerFn;
    TimestampToStrFnType _timestampToStrFn;

    std::thread _delayedInvokesThread;

    std::mutex _invokeMutex, _delayedInvokesMutex;

    std::string _dataPath;
    std::string _resourcesPath;

    std::thread::id _mainThreadID;

    bool _isProcessing;

    std::condition_variable _delayedInvokesWaitCV;
    std::atomic<bool> _stopDelayedInvokesThread;

    std::mutex _delayedInvokesChangedMutex;
    bool _delayedInvokesChanged;

    void updateDelayedInvokes();

    std::string _language;
    std::string _appVersion;
    std::string _appCoreVersion;
    std::string _licenseName;

    std::string _documentName;

    Platform();
    ~Platform();

   public:
    static Platform* sharedInstance();

    // Platform functions
    void setInvokeFunctionAddedFn(InvokeFunctionAddedFnType invokeFunctionAddedFn) {
        _invokeFunctionAddedFn = invokeFunctionAddedFn;
    }
    void setCursorSetFn(CursorSetFnType cursorSetFn) { _cursorSetFn = cursorSetFn; }
    void setDocumentNameSetFn(DocumentNameSetFnType documentNameSetFn) { _documentNameSetFn = documentNameSetFn; }
    void setAlertFn(AlertFnType alertFn) { _alertFn = alertFn; }
    void setBeepFn(BeepFnType beepFn) { _beepFn = beepFn; }
    void setCreateWindowFn(CreateWindowFnType createWindowFn) { _createWindowFn = createWindowFn; }
    void setMakeKeyWindowFn(MakeKeyWindowFnType makeKeyWindowFn) { _makeKeyWindowFn = makeKeyWindowFn; }
    void setDestroyWindowFn(DestroyWindowFnType destroyWindowFn) { _destroyWindowFn = destroyWindowFn; }
    void setSetPasteboardContentFn(SetPasteboardContentFnType setPasteboardContentFn) {
        _setPasteboardContentFn = setPasteboardContentFn;
    }
    void setGetPasteboardContentFn(GetPasteboardContentFnType getPasteboardContentFn) {
        _getPasteboardContentFn = getPasteboardContentFn;
    }
    void setExceptionHandlerFn(ExceptionHandlerFnType exceptionHandlerFn) { _exceptionHandlerFn = exceptionHandlerFn; }
    void setTimestampToStrFn(TimestampToStrFnType timestampToStrFn) { _timestampToStrFn = timestampToStrFn; }
    void setDataPath(const std::string& dataPath) { _dataPath = dataPath; }
    void setResourcesPath(const std::string& resourcesPath) { _resourcesPath = resourcesPath; }
    void setLanguage(const std::string& language) { _language = language; }
    void setAppVersion(const std::string& version) { _appVersion = version; }
    void setAppCoreVersion(const std::string& version) { _appCoreVersion = version; }

    void setLicenseName(const std::string& licenseName) { _licenseName = licenseName; }
    std::string licenseName() { return _licenseName; }

    void process();

    // Client functions
    void invoke(std::function<void()> invokeFn, void* owner = nullptr);
    std::string dataPath() { return _dataPath; }
    std::string resourcesPath() { return _resourcesPath; }
    void setDocumentName(const std::string& name);
    std::string documentName() { return _documentName; }

    // Delayed invokes
    void invokeDelayed(void* owner, std::function<void()> invokeFn, double period);
    void cancelDelayedInvokes(void* owner);

    std::string operatingSystem();
    std::string language() { return _language; }
    std::string appVersion() { return _appVersion; }
    std::string appCoreVersion() { return _appCoreVersion; }
    std::string studioVersion();

    /// \brief Set the cursor
    /// \details Set the cursor
    /// \param cursor The cursor type
    void setCursor(const ResponderChain* responderChain, CursorEnumType cursor);

    void alert(const std::string& title, const std::string& message, bool isFatal);

    void beep();

    void createWindow(void* owner, float x, float y, float width, float height, View* contentView, bool isKeyWindow);

    /// \brief Makes the window the key window
    /// \details Makes the window the key window
    /// \param owner Owner of the window or nullptr for the main window
    void makeKeyWindow(void* owner);

    void destroyWindow(void* owner);

    void setPasteboardContent(const std::string& content);
    std::string getPasteboardContent();

    void throwException(const std::runtime_error& exception);

    /// \brief Get the timestamp as a string
    /// \details Get the timestamp as a string using the platform date and time formatting if available
    /// \param timestamp The number of seconds since January 1st 2001
    /// \return The date and time as a string
    std::string timestampToStr(double timestamp);
};

}  // namespace MDStudio

#endif  // PLATFORM_H
