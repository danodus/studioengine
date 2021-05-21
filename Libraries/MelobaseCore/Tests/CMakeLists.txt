cmake_minimum_required(VERSION 3.12)
project(MelobaseCoreTest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC
    main.cpp
    sequenceutils.cpp
    test_sequencesdb.cpp
    test_sequenceedition.cpp
    test_sync.cpp
    tests.cpp
)

add_executable(MelobaseCoreTest ${SRC})
target_include_directories(MelobaseCoreTest PRIVATE ./)
target_link_libraries(MelobaseCoreTest MDStudio MelobaseCore)

if(UNIX)
target_link_libraries(MelobaseCoreTest MDStudio MelobaseCore -ldl)
endif()

if(APPLE)
   FIND_LIBRARY(COREAUDIO_LIBRARY CoreAudio)
   FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation )
   FIND_LIBRARY(COREMIDI_LIBRARY CoreMIDI )
   FIND_LIBRARY(ACCELERATE_LIBRARY Accelerate )
   target_link_libraries(MelobaseCoreTest ${COREAUDIO_LIBRARY} ${COREFOUNDATION_LIBRARY} ${COREMIDI_LIBRARY} ${ACCELERATE_LIBRARY})
endif()

add_test(NAME MelobaseCore/SequenceEdition/MoveEvents COMMAND MelobaseCoreTest MoveEvents)
add_test(NAME MelobaseCore/SequenceEdition/QuantizeEvents COMMAND MelobaseCoreTest QuantizeEvents)
add_test(NAME MelobaseCore/SequenceEdition/Tracks COMMAND MelobaseCoreTest Tracks)
add_test(NAME MelobaseCore/SequenceEdition/StudioSequenceConversion COMMAND MelobaseCoreTest StudioSequenceConversion)
add_test(NAME MelobaseCore/SequencesDB COMMAND MelobaseCoreTest SequencesDB)
add_test(NAME MelobaseCore/Sync COMMAND MelobaseCoreTest Sync)

