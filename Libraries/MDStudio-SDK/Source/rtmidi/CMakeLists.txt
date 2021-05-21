add_library(rtmidi STATIC RtMidi.cpp)

if(APPLE)
    target_compile_definitions(rtmidi PRIVATE __MACOSX_CORE__)
elseif(WIN32)
    target_compile_definitions(rtmidi PRIVATE __WINDOWS_MM__)
else()
    target_compile_definitions(rtmidi PRIVATE __LINUX_ALSA__)
    target_link_libraries(rtmidi asound)
endif()

target_include_directories(rtmidi PUBLIC .)