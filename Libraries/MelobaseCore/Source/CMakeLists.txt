set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC
    melobasecore_event.cpp
    melobasecore_sequence.cpp
    melobasecorescriptmodule.cpp
    metronomecontroller.cpp
    sequenceeditor.cpp
    sequencesdb.cpp
    server.cpp
    studiocontroller.cpp
    utils.cpp
    Sync/databasesync.cpp
    Sync/folderparser.cpp
    Sync/folderpull.cpp
    Sync/folderpush.cpp
    Sync/folderssync.cpp
    Sync/sequenceparser.cpp
    Sync/sequencepull.cpp
    Sync/sequencepush.cpp
    Sync/sequencessync.cpp
)

add_library(MelobaseCore STATIC ${SRC})
target_link_libraries(MelobaseCore MDStudio)

target_include_directories(MDStudio PUBLIC ./)

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  target_compile_definitions(MelobaseCore PRIVATE TARGET_OS_IPHONE)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_definitions(MelobaseCore PRIVATE _LINUX)
endif()
