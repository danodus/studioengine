cmake_minimum_required(VERSION 3.12)

project(studioengine)

include(CTest)

add_subdirectory(Libraries/MDStudio-SDK)
add_subdirectory(Libraries/MelobaseCore)

enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRCDIR ./Common)

set(SRC
    main.cpp
    ${SRCDIR}/aboutviewcontroller.cpp
    ${SRCDIR}/arrangementview.cpp
    ${SRCDIR}/arrangementviewcontroller.cpp
    ${SRCDIR}/audioexportviewcontroller.cpp
    ${SRCDIR}/borderview.cpp
    ${SRCDIR}/dbview.cpp
    ${SRCDIR}/dbviewcontroller.cpp
    ${SRCDIR}/eventlistitemview.cpp
    ${SRCDIR}/folderlistitemview.cpp
    ${SRCDIR}/foldersview.cpp
    ${SRCDIR}/helpers.cpp
    ${SRCDIR}/keyboardv.cpp
    ${SRCDIR}/melobaseapp.cpp
    ${SRCDIR}/melobasescriptmodule.cpp
    ${SRCDIR}/midihub.cpp
    ${SRCDIR}/midiimport.cpp
    ${SRCDIR}/midiimportviewcontroller.cpp
    ${SRCDIR}/modalviewcontroller.cpp
    ${SRCDIR}/newsequenceviewcontroller.cpp
    ${SRCDIR}/pianorolleventslistview.cpp
    ${SRCDIR}/pianorolleventsview.cpp
    ${SRCDIR}/pianorollheaderview.cpp
    ${SRCDIR}/pianorollmainview.cpp
    ${SRCDIR}/pianorollpropertiesview.cpp
    ${SRCDIR}/pianorollpropertiesviewcontroller.cpp
    ${SRCDIR}/pianorollutilitiesview.cpp
    ${SRCDIR}/pianorollview.cpp
    ${SRCDIR}/pianorollviewcontroller.cpp
    ${SRCDIR}/preferencesviewcontroller.cpp
    ${SRCDIR}/programcomboboxcontroller.cpp
    ${SRCDIR}/rulerview.cpp
    ${SRCDIR}/segmentedcontrolv.cpp
    ${SRCDIR}/sequencelistitemview.cpp
    ${SRCDIR}/sequencesview.cpp
    ${SRCDIR}/sequenceview.cpp
    ${SRCDIR}/sequenceviewcontroller.cpp
    ${SRCDIR}/studiochannelview.cpp
    ${SRCDIR}/studiochannelviewcontroller.cpp
    ${SRCDIR}/studioview.cpp
    ${SRCDIR}/topview.cpp
    ${SRCDIR}/topviewcontroller.cpp
    ${SRCDIR}/trackclipsview.cpp
    ${SRCDIR}/trackheaderview.cpp
    ${SRCDIR}/trackview.cpp
    ${SRCDIR}/zoneview.cpp
    ${SRCDIR}/zoneviewcontroller.cpp
)

# Find SDL2 and OpenGL

IF(UNIX)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
find_package(SDL2 REQUIRED COMPONENTS main)
ENDIF()

find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME} ${SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ./ ${SDL2_INCLUDE_DIRS} ${SDL2main_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} MDStudio MelobaseCore ${SDL2_LIBS} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})

IF (APPLE)
FIND_LIBRARY(COREAUDIO_LIBRARY CoreAudio)
FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation )
FIND_LIBRARY(COREMIDI_LIBRARY CoreMIDI )
FIND_LIBRARY(ACCELERATE_LIBRARY Accelerate )
target_link_libraries(${PROJECT_NAME} ${COREAUDIO_LIBRARY} ${COREFOUNDATION_LIBRARY} ${COREMIDI_LIBRARY} ${ACCELERATE_LIBRARY})
ENDIF (APPLE)

set (source "${CMAKE_SOURCE_DIR}/Resources")
set (destination "${CMAKE_CURRENT_BINARY_DIR}/Resources")
add_custom_command(
TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination} 
DEPENDS ${destination}
COMMENT "symbolic link resources folder from ${source} => ${destination}"
)

