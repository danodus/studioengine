//
//  platform.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-08-09.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "platform.h"

#include <assert.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#define PLATFORM_STUDIO_VERSION "2.5.0"

using namespace MDStudio;

#ifdef _WIN32
// ---------------------------------------------------------------------------------------------------------------------
time_t timegm(struct tm* tm) { return _mkgmtime(tm); }
#endif

// ---------------------------------------------------------------------------------------------------------------------
Platform::Platform() {
    _mainThreadID = std::this_thread::get_id();
    _invokeFunctionAddedFn = nullptr;
    _cursorSetFn = nullptr;
    _documentNameSetFn = nullptr;
    _alertFn = nullptr;
    _beepFn = nullptr;
    _createWindowFn = nullptr;
    _makeKeyWindowFn = nullptr;
    _destroyWindowFn = nullptr;
    _setPasteboardContentFn = nullptr;
    _getPasteboardContentFn = nullptr;
    _exceptionHandlerFn = nullptr;
    _timestampToStrFn = nullptr;
    _isProcessing = false;
    _stopDelayedInvokesThread = false;
    _delayedInvokesChanged = false;

    _delayedInvokesThread = std::thread(&Platform::updateDelayedInvokes, this);
}

// ---------------------------------------------------------------------------------------------------------------------
Platform::~Platform() {
    _stopDelayedInvokesThread = true;

    // Notify the delayed invokes thread that a change has occured
    // We do so in order to make sure that the thread is not waiting
    std::unique_lock<std::mutex> lk(_delayedInvokesChangedMutex);
    _delayedInvokesChanged = true;
    lk.unlock();
    _delayedInvokesWaitCV.notify_all();

    _delayedInvokesThread.join();
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::updateDelayedInvokes() {
    while (!_stopDelayedInvokesThread) {
        _delayedInvokesMutex.lock();

        // Find the minimum timestamp
        auto minTimestamp = std::numeric_limits<double>::max();
        for (auto delayedInvoke : _delayedInvokes) {
            if (delayedInvoke.timestamp < minTimestamp) minTimestamp = delayedInvoke.timestamp;
        }

        _delayedInvokesMutex.unlock();

        // Wait until the minimum timestamp is reached or a new delayed invoke
        // has been added

        if (minTimestamp != std::numeric_limits<double>::max()) {
            auto delta = minTimestamp - getTimestamp();
            if (delta < 0) delta = 0;

            std::unique_lock<std::mutex> lk(_delayedInvokesChangedMutex);
            _delayedInvokesWaitCV.wait_for(lk, std::chrono::milliseconds((long long)(delta * 1000)),
                                           [this] { return _delayedInvokesChanged; });
            _delayedInvokesChanged = false;
        } else {
            std::unique_lock<std::mutex> lk(_delayedInvokesChangedMutex);
            _delayedInvokesWaitCV.wait(lk, [this] { return _delayedInvokesChanged; });
            _delayedInvokesChanged = false;
        }

        // Process the invokes
        _delayedInvokesMutex.lock();
        std::vector<DelayedInvoke>::iterator it;
        for (it = _delayedInvokes.begin(); it != _delayedInvokes.end();) {
            DelayedInvoke delayedInvoke = *it;
            double timestamp = getTimestamp();
            if (timestamp >= delayedInvoke.timestamp) {
                invoke(delayedInvoke.invoke.fn, delayedInvoke.invoke.owner);
                it = _delayedInvokes.erase(it);
            } else {
                ++it;
            }
        }
        _delayedInvokesMutex.unlock();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
Platform* Platform::sharedInstance() {
    static Platform instance;
    return &instance;
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::invoke(std::function<void()> invokeFn, void* owner) {
    _invokeMutex.lock();
    Invoke inv;
    inv.owner = owner;
    inv.fn = invokeFn;
    _invokeFns.push(inv);
    if (_invokeFunctionAddedFn) _invokeFunctionAddedFn();
    _invokeMutex.unlock();
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::process() {
    // If we are already processing, we do nothing
    if (_isProcessing) return;

    _isProcessing = true;

    Invoke invokeToExecute;
    bool isInvokeToExecuteAvailable = false;

    // We keep processing until both lists are empty
    do {
        isInvokeToExecuteAvailable = false;
        _invokeMutex.lock();
        if (_invokeFns.size() > 0) {
            invokeToExecute = _invokeFns.front();
            _invokeFns.pop();
            isInvokeToExecuteAvailable = true;
        }
        _invokeMutex.unlock();

        if (isInvokeToExecuteAvailable) {
            try {
                invokeToExecute.fn();
            } catch (const std::runtime_error& e) {
                throwException(e);
            }
        }
    } while (isInvokeToExecuteAvailable);

    _isProcessing = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::invokeDelayed(void* owner, std::function<void()> invokeFn, double period) {
    _delayedInvokesMutex.lock();
    DelayedInvoke delayedInvoke;
    delayedInvoke.invoke.owner = owner;
    delayedInvoke.invoke.fn = invokeFn;
    delayedInvoke.timestamp = getTimestamp() + period;
    _delayedInvokes.push_back(delayedInvoke);
    _delayedInvokesMutex.unlock();

    // Notify the delayed invokes thread that a change has occured
    std::unique_lock<std::mutex> lk(_delayedInvokesChangedMutex);
    _delayedInvokesChanged = true;
    _delayedInvokesWaitCV.notify_all();
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::cancelDelayedInvokes(void* owner) {
    _delayedInvokesMutex.lock();
    std::vector<DelayedInvoke>::iterator it;

    for (it = _delayedInvokes.begin(); it != _delayedInvokes.end();) {
        DelayedInvoke delayedInvoke = *it;
        if (delayedInvoke.invoke.owner == owner) {
            it = _delayedInvokes.erase(it);
        } else {
            ++it;
        }
    }

    // Also remove from the current invoke list
    _invokeMutex.lock();

    std::queue<Invoke> newQueue;

    while (_invokeFns.size() > 0) {
        Invoke inv = _invokeFns.front();
        if (inv.owner != owner) {
            newQueue.push(inv);
        }
        _invokeFns.pop();
    }

    _invokeFns = newQueue;

    _invokeMutex.unlock();

    _delayedInvokesMutex.unlock();

    // Notify the delayed invokes thread that a change has occured
    std::unique_lock<std::mutex> lk(_delayedInvokesChangedMutex);
    _delayedInvokesChanged = true;
    _delayedInvokesWaitCV.notify_all();
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::setCursor(const ResponderChain* responderChain, CursorEnumType cursor) {
    if (_cursorSetFn) _cursorSetFn(responderChain, cursor);
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::setDocumentName(const std::string& name) {
    _documentName = name;
    if (_documentNameSetFn) _documentNameSetFn(_documentName);
}

// ---------------------------------------------------------------------------------------------------------------------
std::string Platform::operatingSystem() {
#ifdef _WIN32
    return "windows";
#else   // _WIN32
    return "osx";
#endif  // _WIN32
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::alert(const std::string& title, const std::string& message, bool isFatal) {
    if (_alertFn) _alertFn(title, message, isFatal);
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::beep() {
    if (_beepFn) _beepFn();
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::createWindow(void* owner, float x, float y, float width, float height, View* contentView,
                            bool isKeyWindow) {
    if (_createWindowFn) return _createWindowFn(owner, x, y, width, height, contentView, isKeyWindow);
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::makeKeyWindow(void* owner) {
    if (_makeKeyWindowFn) _makeKeyWindowFn(owner);
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::destroyWindow(void* owner) {
    if (_destroyWindowFn) _destroyWindowFn(owner);
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::setPasteboardContent(const std::string& content) {
    if (_setPasteboardContentFn) _setPasteboardContentFn(content);
}

// ---------------------------------------------------------------------------------------------------------------------
std::string Platform::getPasteboardContent() {
    if (_getPasteboardContentFn) return _getPasteboardContentFn();

    return std::string();
}

// ---------------------------------------------------------------------------------------------------------------------
void Platform::throwException(const std::runtime_error& exception) {
    if (_exceptionHandlerFn) {
        _exceptionHandlerFn(exception.what());
    } else {
        throw(exception);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
std::string Platform::studioVersion() { return PLATFORM_STUDIO_VERSION; }

// ---------------------------------------------------------------------------------------------------------------------
std::string Platform::timestampToStr(double timestamp) {
    if (_timestampToStrFn) {
        return _timestampToStrFn(timestamp);
    } else {

        // ISO format

        auto t = timestampToDateTime(timestamp);
        std::stringstream ss;
        if (t) {
            ss << std::setfill('0') << t->tm_year + 1900 << "-" << std::setfill('0') << std::setw(2) << t->tm_mon + 1
               << "-" << std::setfill('0') << std::setw(2) << t->tm_mday << ", " << t->tm_hour << ":"
               << std::setfill('0') << std::setw(2) << t->tm_min;
        } else {
            ss << "?";
        }
        return ss.str();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
double MDStudio::getTimestamp() {
    typedef std::chrono::duration<double> doublePrecisionDurationType;
    typedef std::chrono::time_point<std::chrono::system_clock, doublePrecisionDurationType> timePointType;

    std::tm tm;
    tm.tm_sec = 0;
    tm.tm_min = 0;
    tm.tm_hour = 0;
    tm.tm_mday = 1;
    tm.tm_mon = 0;
    tm.tm_year = (2001 - 1900);
    tm.tm_isdst = -1;
    std::time_t refTime = ::timegm(&tm);

    timePointType referenceTP = std::chrono::system_clock::from_time_t(refTime);
    timePointType currentTP = std::chrono::system_clock::now();

    doublePrecisionDurationType duration = currentTP - referenceTP;
    double timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.0;

    return timestamp;
}

// ---------------------------------------------------------------------------------------------------------------------
std::tm* MDStudio::timestampToDateTime(double timestamp) {
    typedef std::chrono::duration<double> doublePrecisionDurationType;
    typedef std::chrono::time_point<std::chrono::system_clock, doublePrecisionDurationType> timePointType;

    // Duration in seconds between 2001 and sequence date

    std::tm tm;
    tm.tm_sec = 0;
    tm.tm_min = 0;
    tm.tm_hour = 0;
    tm.tm_mday = 1;
    tm.tm_mon = 0;
    tm.tm_year = (2001 - 1900);
    tm.tm_isdst = -1;

    std::time_t refTime = timegm(&tm);
    std::chrono::system_clock::time_point rtp = std::chrono::system_clock::from_time_t(refTime);

    std::chrono::system_clock::time_point tp = rtp + std::chrono::milliseconds((long long)(timestamp * 1000.0));

    auto z = std::chrono::system_clock::to_time_t(tp);

    return std::localtime(&z);
}

// ---------------------------------------------------------------------------------------------------------------------
static bool isHostBigEndian() {
    unsigned char swapTest[2] = {1, 0};

    if (*(unsigned short*)swapTest == 1) {
        // Little endian
        return false;
    } else {
        // Big endian
        return true;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned short MDStudio::swapUInt16HostToBig(unsigned short s) {
    if (!isHostBigEndian()) {
        unsigned char b1, b2;

        b1 = s & 255;
        b2 = (s >> 8) & 255;

        return (b1 << 8) + b2;
    }
    return s;
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int MDStudio::swapUInt32HostToBig(unsigned int i) {
    if (!isHostBigEndian()) {
        unsigned char b1, b2, b3, b4;

        b1 = i & 255;
        b2 = (i >> 8) & 255;
        b3 = (i >> 16) & 255;
        b4 = (i >> 24) & 255;

        return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
    }
    return i;
}

// ---------------------------------------------------------------------------------------------------------------------
// Encode a value as a variable length vector of bytes
std::vector<unsigned char> MDStudio::vlEncode(unsigned int value) {
    std::vector<unsigned char> data;
    unsigned char bytes[4];

    // Variable-length encoding
    unsigned char n1 = value & 0x7f;
    value >>= 7;  // LSB
    unsigned char n2 = value & 0x7f;
    value >>= 7;
    unsigned char n3 = value & 0x7f;
    value >>= 7;
    unsigned char n4 = value & 0x7f;  // MSB

    if (n4) {
        bytes[0] = n4 | 0x80;
        bytes[1] = n3 | 0x80;
        bytes[2] = n2 | 0x80;
        bytes[3] = n1;
        for (std::size_t i = 0; i < 4; i++) data.push_back(bytes[i]);
    } else if (n3) {
        bytes[0] = n3 | 0x80;
        bytes[1] = n2 | 0x80;
        bytes[2] = n1;
        for (std::size_t i = 0; i < 3; i++) data.push_back(bytes[i]);
    } else if (n2) {
        bytes[0] = n2 | 0x80;
        bytes[1] = n1;
        for (std::size_t i = 0; i < 2; i++) data.push_back(bytes[i]);
    } else {
        bytes[0] = n1;
        data.push_back(bytes[0]);
    }

    return data;
}
