cmake_minimum_required(VERSION 3.12)
project(MDStudioTest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC
    main.cpp
    test_pasteboard.cpp
    test_plist.cpp
    test_undomanager.cpp
    test_importexport.cpp
    tests.cpp
)

add_executable(MDStudioTest ${SRC})
target_include_directories(MDStudioTest PRIVATE ./)
target_link_libraries(MDStudioTest MDStudio)

if(UNIX)
target_link_libraries(MDStudioTest MDStudio -ldl)
endif()

if(APPLE)
   FIND_LIBRARY(COREAUDIO_LIBRARY CoreAudio)
   FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation )
   FIND_LIBRARY(COREMIDI_LIBRARY CoreMIDI )
   FIND_LIBRARY(ACCELERATE_LIBRARY Accelerate )
   target_link_libraries(MDStudioTest ${COREAUDIO_LIBRARY} ${COREFOUNDATION_LIBRARY} ${COREMIDI_LIBRARY} ${ACCELERATE_LIBRARY})
endif()

set (source "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
set (destination "${CMAKE_CURRENT_BINARY_DIR}/Resources")
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination} 
    DEPENDS ${destination}
    COMMENT "symbolic link resources folder from ${source} => ${destination}"
)

add_test(NAME MDStudio/Plist COMMAND MDStudioTest Plist)
add_test(NAME MDStudio/UndoManager COMMAND MDStudioTest UndoManager)
add_test(NAME MDStudio/PasteBoard COMMAND MDStudioTest PasteBoard)
add_test(NAME MDStudio/ImportExport COMMAND MDStudioTest ImportExport)

