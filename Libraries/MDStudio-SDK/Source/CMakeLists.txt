if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
add_subdirectory(rtmidi)
add_subdirectory(portaudio)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRCDIR .)
set(LIBAIFF ${SRCDIR}/libaiff-5.0)
set(LUA ${SRCDIR}/lua)
set(PORTABLECORE ${SRCDIR}/MDStudio/PortableCore)
set(LRDB ${SRCDIR}/lrdb)
set(PICOJSON ${SRCDIR}/picojson)
set(ASIO ${SRCDIR}/asio)
set(FREETYPE ${SRCDIR}/freetype)
set(PNG ${SRCDIR}/libpng)
set(ZLIB ${SRCDIR}/zlib)
set(EXPAT ${SRCDIR}/expat)
set(SQLITE ${SRCDIR}/sqlite)
set(PORTABLECOREDB ${PORTABLECORE}/DB)
set(PORTABLECOREAUDIO ${PORTABLECORE}/Audio)
set(PORTABLECOREUI ${PORTABLECORE}/UI)
set(GLEW ${SRCDIR}/glew-2.1.0)
set(CPPHTTPLIB ${SRCDIR}/cpp-httplib)

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
set(MIXER mixer_ios.cpp)
else()
set(MIXER mixer.cpp)
endif()

set(SRC

    ${LUA}/src/lapi.c
    ${LUA}/src/lcode.c
    ${LUA}/src/lctype.c
    ${LUA}/src/ldebug.c
    ${LUA}/src/ldo.c
    ${LUA}/src/ldump.c
    ${LUA}/src/lfunc.c
    ${LUA}/src/lgc.c
    ${LUA}/src/llex.c
    ${LUA}/src/lmem.c
    ${LUA}/src/lobject.c
    ${LUA}/src/lopcodes.c
    ${LUA}/src/lparser.c
    ${LUA}/src/lstate.c
    ${LUA}/src/lstring.c
    ${LUA}/src/ltable.c
    ${LUA}/src/ltm.c
    ${LUA}/src/lundump.c
    ${LUA}/src/lvm.c
    ${LUA}/src/lzio.c

    ${LUA}/src/lauxlib.c
    ${LUA}/src/lbaselib.c
    ${LUA}/src/lbitlib.c
    ${LUA}/src/lcorolib.c
    ${LUA}/src/ldblib.c
    ${LUA}/src/liolib.c
    ${LUA}/src/lmathlib.c
    ${LUA}/src/loslib.c
    ${LUA}/src/lstrlib.c
    ${LUA}/src/ltablib.c
    ${LUA}/src/lutf8lib.c
    ${LUA}/src/loadlib.c
    ${LUA}/src/linit.c

    ${PORTABLECORE}/any.h
    ${PORTABLECORE}/app.cpp
    ${PORTABLECORE}/app.h
    ${PORTABLECORE}/base64.h
    ${PORTABLECORE}/fulltypename.h
    ${PORTABLECORE}/mongoose.cpp
    ${PORTABLECORE}/mongoose.h
    ${PORTABLECORE}/pasteboard.cpp
    ${PORTABLECORE}/pasteboard.h
    ${PORTABLECORE}/platform.cpp
    ${PORTABLECORE}/platform.h
    ${PORTABLECORE}/plist.cpp
    ${PORTABLECORE}/plist.h
    ${PORTABLECORE}/plistdate.cpp
    ${PORTABLECORE}/plistdate.h
    ${PORTABLECORE}/property.h
    ${PORTABLECORE}/script.cpp
    ${PORTABLECORE}/script.h
    ${PORTABLECORE}/types.h
    ${PORTABLECORE}/undomanager.cpp
    ${PORTABLECORE}/undomanager.h
    ${PORTABLECORE}/uricodec.cpp
    ${PORTABLECORE}/uricodec.h
    ${PORTABLECORE}/ustring.cpp
    ${PORTABLECORE}/ustring.h
    ${PORTABLECORE}/utf8.h
    ${PORTABLECORE}/utf8checked.h
    ${PORTABLECORE}/utf8core.h
    ${PORTABLECORE}/utf8unchecked.h

    ${PORTABLECOREDB}/db.cpp
    ${PORTABLECOREDB}/db.h

    ${PORTABLECOREAUDIO}/atomicops.h
    ${PORTABLECOREAUDIO}/audioexport.cpp
    ${PORTABLECOREAUDIO}/audioexport.h
    ${PORTABLECOREAUDIO}/audioplayerunit.cpp
    ${PORTABLECOREAUDIO}/audioplayerunit.h
    ${PORTABLECOREAUDIO}/audioscriptmodule.cpp
    ${PORTABLECOREAUDIO}/audioscriptmodule.h
    ${PORTABLECOREAUDIO}/Chorus/chorusmodel.cpp
    ${PORTABLECOREAUDIO}/Chorus/chorusmodel.h
    ${PORTABLECOREAUDIO}/event.cpp
    ${PORTABLECOREAUDIO}/event.h
    ${PORTABLECOREAUDIO}/filter_k12z4.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/Biquad.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/Cascade.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/Common.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/Design.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/Dsp.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/Elliptic.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/Filter.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/Layout.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/MathSupplement.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/Params.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/RBJ.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/RootFinder.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/SmoothedFilter.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/State.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/Types.h
    ${PORTABLECOREAUDIO}/Filters/include/DspFilters/Utilities.h
    ${PORTABLECOREAUDIO}/Filters/source/Biquad.cpp
    ${PORTABLECOREAUDIO}/Filters/source/Cascade.cpp
    ${PORTABLECOREAUDIO}/Filters/source/Design.cpp
    ${PORTABLECOREAUDIO}/Filters/source/Filter.cpp
    ${PORTABLECOREAUDIO}/Filters/source/Param.cpp
    ${PORTABLECOREAUDIO}/Filters/source/RBJ.cpp
    ${PORTABLECOREAUDIO}/Filters/source/RootFinder.cpp
    ${PORTABLECOREAUDIO}/Filters/source/State.cpp
    ${PORTABLECOREAUDIO}/Reverb/comb.cpp
    ${PORTABLECOREAUDIO}/Reverb/revmodel.cpp
    ${PORTABLECOREAUDIO}/fmunit.cpp
    ${PORTABLECOREAUDIO}/fmunit.h
    ${PORTABLECOREAUDIO}/instrument.cpp
    ${PORTABLECOREAUDIO}/instrument.h
    ${PORTABLECOREAUDIO}/instrumentmanager.cpp
    ${PORTABLECOREAUDIO}/instrumentmanager.h
    ${PORTABLECOREAUDIO}/lowpassfilterunit.cpp
    ${PORTABLECOREAUDIO}/lowpassfilterunit.h
    ${PORTABLECOREAUDIO}/metronome.cpp
    ${PORTABLECOREAUDIO}/metronome.h
    ${PORTABLECOREAUDIO}/midifile.cpp
    ${PORTABLECOREAUDIO}/midifile.h
    ${PORTABLECOREAUDIO}/${MIXER}
    ${PORTABLECOREAUDIO}/mixer.h
    ${PORTABLECOREAUDIO}/multiinstrument.cpp
    ${PORTABLECOREAUDIO}/multiinstrument.h
    ${PORTABLECOREAUDIO}/sample.cpp
    ${PORTABLECOREAUDIO}/sample.h
    ${PORTABLECOREAUDIO}/samplerunit.cpp
    ${PORTABLECOREAUDIO}/samplerunit.h
    ${PORTABLECOREAUDIO}/sequence.cpp
    ${PORTABLECOREAUDIO}/sequence.h
    ${PORTABLECOREAUDIO}/sequencer.cpp
    ${PORTABLECOREAUDIO}/sequencer.h
    ${PORTABLECOREAUDIO}/sineunit.cpp
    ${PORTABLECOREAUDIO}/sineunit.h
    ${PORTABLECOREAUDIO}/soundfont2.cpp
    ${PORTABLECOREAUDIO}/soundfont2.h
    ${PORTABLECOREAUDIO}/studio.cpp
    ${PORTABLECOREAUDIO}/studio.h
    ${PORTABLECOREAUDIO}/unit.cpp
    ${PORTABLECOREAUDIO}/unit.h
    ${PORTABLECOREAUDIO}/voice.h

    ${PORTABLECOREUI}/animation.cpp
    ${PORTABLECOREUI}/animation.h
    ${PORTABLECOREUI}/bezier.cpp
    ${PORTABLECOREUI}/bezier.h
    ${PORTABLECOREUI}/boxview.cpp
    ${PORTABLECOREUI}/boxview.h
    ${PORTABLECOREUI}/button.cpp
    ${PORTABLECOREUI}/button.h
    ${PORTABLECOREUI}/color.cpp
    ${PORTABLECOREUI}/color.h
    ${PORTABLECOREUI}/column.cpp
    ${PORTABLECOREUI}/column.h
    ${PORTABLECOREUI}/combobox.cpp
    ${PORTABLECOREUI}/combobox.h
    ${PORTABLECOREUI}/control.cpp
    ${PORTABLECOREUI}/control.h
    ${PORTABLECOREUI}/draw.cpp
    ${PORTABLECOREUI}/draw.h
    ${PORTABLECOREUI}/drawcontext.cpp
    ${PORTABLECOREUI}/drawcontext.h
    ${PORTABLECOREUI}/font.cpp
    ${PORTABLECOREUI}/font.h
    ${PORTABLECOREUI}/image.cpp
    ${PORTABLECOREUI}/image.h
    ${PORTABLECOREUI}/imageview.cpp
    ${PORTABLECOREUI}/imageview.h
    ${PORTABLECOREUI}/keyboard.cpp
    ${PORTABLECOREUI}/keyboard.h
    ${PORTABLECOREUI}/keyboardh.cpp
    ${PORTABLECOREUI}/keyboardh.h
    ${PORTABLECOREUI}/keyboardv.cpp
    ${PORTABLECOREUI}/keyboardv.h
    ${PORTABLECOREUI}/labelview.cpp
    ${PORTABLECOREUI}/labelview.h
    ${PORTABLECOREUI}/levelindicator.cpp
    ${PORTABLECOREUI}/levelindicator.h
    ${PORTABLECOREUI}/listitemview.cpp
    ${PORTABLECOREUI}/listitemview.h
    ${PORTABLECOREUI}/listview.cpp
    ${PORTABLECOREUI}/listview.h
    ${PORTABLECOREUI}/menu.cpp
    ${PORTABLECOREUI}/menu.h
    ${PORTABLECOREUI}/menubar.cpp
    ${PORTABLECOREUI}/menubar.h
    ${PORTABLECOREUI}/menuitem.cpp
    ${PORTABLECOREUI}/menuitem.h
    ${PORTABLECOREUI}/path.cpp
    ${PORTABLECOREUI}/path.h
    ${PORTABLECOREUI}/point.cpp
    ${PORTABLECOREUI}/point.h
    ${PORTABLECOREUI}/progressindicator.cpp
    ${PORTABLECOREUI}/progressindicator.h
    ${PORTABLECOREUI}/rect.cpp
    ${PORTABLECOREUI}/rect.h
    ${PORTABLECOREUI}/responder.cpp
    ${PORTABLECOREUI}/responder.h
    ${PORTABLECOREUI}/responderchain.cpp
    ${PORTABLECOREUI}/responderchain.h
    ${PORTABLECOREUI}/scrollbar.cpp
    ${PORTABLECOREUI}/scrollbar.h
    ${PORTABLECOREUI}/scrollview.cpp
    ${PORTABLECOREUI}/scrollview.h
    ${PORTABLECOREUI}/searchfield.cpp
    ${PORTABLECOREUI}/searchfield.h
    ${PORTABLECOREUI}/segmentedcontrol.cpp
    ${PORTABLECOREUI}/segmentedcontrol.h
    ${PORTABLECOREUI}/sheet.cpp
    ${PORTABLECOREUI}/sheet.h
    ${PORTABLECOREUI}/size.cpp
    ${PORTABLECOREUI}/size.h
    ${PORTABLECOREUI}/slider.cpp
    ${PORTABLECOREUI}/slider.h
    ${PORTABLECOREUI}/splitviewh.cpp
    ${PORTABLECOREUI}/splitviewh.h
    ${PORTABLECOREUI}/splitviewmultih.cpp
    ${PORTABLECOREUI}/splitviewmultih.h
    ${PORTABLECOREUI}/splitviewv.cpp
    ${PORTABLECOREUI}/splitviewv.h
    ${PORTABLECOREUI}/stepper.cpp
    ${PORTABLECOREUI}/stepper.h
    ${PORTABLECOREUI}/svg.cpp
    ${PORTABLECOREUI}/svg.h
    ${PORTABLECOREUI}/svgparsers.cpp
    ${PORTABLECOREUI}/svgparsers.h
    ${PORTABLECOREUI}/svgview.cpp
    ${PORTABLECOREUI}/svgview.h
    ${PORTABLECOREUI}/tableview.cpp
    ${PORTABLECOREUI}/tableview.h
    ${PORTABLECOREUI}/textfield.cpp
    ${PORTABLECOREUI}/textfield.h
    ${PORTABLECOREUI}/textview.cpp
    ${PORTABLECOREUI}/textview.h
    ${PORTABLECOREUI}/tooltip.cpp
    ${PORTABLECOREUI}/tooltip.h
    ${PORTABLECOREUI}/tooltipmanager.cpp
    ${PORTABLECOREUI}/tooltipmanager.h
    ${PORTABLECOREUI}/treeview.cpp
    ${PORTABLECOREUI}/treeview.h
    ${PORTABLECOREUI}/triangulate.cpp
    ${PORTABLECOREUI}/triangulate.h
    ${PORTABLECOREUI}/ui.cpp
    ${PORTABLECOREUI}/ui.h
    ${PORTABLECOREUI}/uievent.cpp
    ${PORTABLECOREUI}/uievent.h
    ${PORTABLECOREUI}/uiscriptmodule.cpp
    ${PORTABLECOREUI}/uiscriptmodule.h
    ${PORTABLECOREUI}/vector2.cpp
    ${PORTABLECOREUI}/vector2.h
    ${PORTABLECOREUI}/view.cpp
    ${PORTABLECOREUI}/view.h
    ${PORTABLECOREUI}/webview.cpp
    ${PORTABLECOREUI}/webview.h
    ${PORTABLECOREUI}/window.cpp
    ${PORTABLECOREUI}/window.h
)

set(MIDI_SRC
  ${PORTABLECOREAUDIO}/midi.cpp
  ${PORTABLECOREAUDIO}/midi.h
)

set(MDNS_SRC
  ${PORTABLECORE}/mdns.cpp
  ${PORTABLECORE}/mdns.h
  ${PORTABLECORE}/mdnsd.cpp
  ${PORTABLECORE}/mdnsd.h
)

set(FREETYPE_SRC
  ${FREETYPE}/src/autofit/autofit.c
  ${FREETYPE}/src/base/ftadvanc.c
  ${FREETYPE}/src/base/ftbbox.c
  ${FREETYPE}/src/base/ftbitmap.c
  ${FREETYPE}/src/base/ftcalc.c
  ${FREETYPE}/src/base/ftcid.c
  ${FREETYPE}/src/base/ftdbgmem.c
  ${FREETYPE}/src/base/ftdebug.c
  ${FREETYPE}/src/base/ftfstype.c
  ${FREETYPE}/src/base/ftgasp.c
  ${FREETYPE}/src/base/ftgloadr.c
  ${FREETYPE}/src/base/ftglyph.c
  ${FREETYPE}/src/base/ftgxval.c
  ${FREETYPE}/src/base/ftinit.c
  ${FREETYPE}/src/base/ftlcdfil.c
  ${FREETYPE}/src/base/ftmm.c
  ${FREETYPE}/src/base/ftobjs.c
  ${FREETYPE}/src/base/ftotval.c
  ${FREETYPE}/src/base/ftoutln.c
  ${FREETYPE}/src/base/ftpatent.c
  ${FREETYPE}/src/base/ftpfr.c
  ${FREETYPE}/src/base/ftrfork.c
  ${FREETYPE}/src/base/ftsnames.c
  ${FREETYPE}/src/base/ftstream.c
  ${FREETYPE}/src/base/ftstroke.c
  ${FREETYPE}/src/base/ftsynth.c
  ${FREETYPE}/src/base/ftsystem.c
  ${FREETYPE}/src/base/fttrigon.c
  ${FREETYPE}/src/base/fttype1.c
  ${FREETYPE}/src/base/ftutil.c
  ${FREETYPE}/src/base/ftwinfnt.c
  ${FREETYPE}/src/base/ftxf86.c
  ${FREETYPE}/src/bdf/bdf.c
  ${FREETYPE}/src/bzip2/ftbzip2.c
  ${FREETYPE}/src/cache/ftcache.c
  ${FREETYPE}/src/cff/cff.c
  ${FREETYPE}/src/cid/type1cid.c
  ${FREETYPE}/src/gzip/ftgzip.c
  ${FREETYPE}/src/lzw/ftlzw.c
  ${FREETYPE}/src/pcf/pcf.c
  ${FREETYPE}/src/pfr/pfr.c
  ${FREETYPE}/src/psaux/psaux.c
  ${FREETYPE}/src/pshinter/pshinter.c
  ${FREETYPE}/src/psnames/psmodule.c
  ${FREETYPE}/src/raster/raster.c
  ${FREETYPE}/src/sfnt/sfnt.c
  ${FREETYPE}/src/smooth/smooth.c
  ${FREETYPE}/src/truetype/truetype.c
  ${FREETYPE}/src/type1/type1.c
  ${FREETYPE}/src/type42/type42.c
  ${FREETYPE}/src/winfonts/winfnt.c
)

set(PNG_SRC
  ${PNG}/png.c
  ${PNG}/pngerror.c
  ${PNG}/pngget.c
  ${PNG}/pngmem.c
  ${PNG}/pngpread.c
  ${PNG}/pngread.c
  ${PNG}/pngrio.c
  ${PNG}/pngrtran.c
  ${PNG}/pngrutil.c
  ${PNG}/pngset.c
  ${PNG}/pngtrans.c
  ${PNG}/pngwio.c
  ${PNG}/pngwrite.c
  ${PNG}/pngwtran.c
  ${PNG}/pngwutil.c
)

set(PNGARM_SRC
  ${PNG}/arm/arm_init.c
  ${PNG}/arm/filter_neon_intrinsics.c
  ${PNG}/arm/filter_neon.S
)  

set(ZLIB_SRC
  ${ZLIB}/adler32.c
  ${ZLIB}/compress.c
  ${ZLIB}/crc32.c
  ${ZLIB}/deflate.c
  ${ZLIB}/gzclose.c
  ${ZLIB}/gzlib.c
  ${ZLIB}/gzread.c
  ${ZLIB}/gzwrite.c
  ${ZLIB}/inflate.c
  ${ZLIB}/infback.c
  ${ZLIB}/inftrees.c
  ${ZLIB}/inffast.c
  ${ZLIB}/trees.c
  ${ZLIB}/uncompr.c
  ${ZLIB}/zutil.c
)

set(EXPAT_SRC
  ${EXPAT}/lib/xmlparse.c
  ${EXPAT}/lib/xmlrole.c
  ${EXPAT}/lib/xmltok.c 
  ${EXPAT}/lib/xmltok_impl.c 
  ${EXPAT}/lib/xmltok_ns.c
)

set(LIBAIFF_SRC
  ${LIBAIFF}/iff.c
  ${LIBAIFF}/aifx.c
  ${LIBAIFF}/lpcm.c
  ${LIBAIFF}/g711.c
  ${LIBAIFF}/float32.c
  ${LIBAIFF}/libaiff.c
  ${LIBAIFF}/extended.c
  ${LIBAIFF}/pascal.c
)

set(SQLITE_SRC
  ${SQLITE}/sqlite3.c
)

if(WIN32)
set(GLEW_SRC
  ${GLEW}/src/glew.c
)
endif()

set(ALL_SRC ${SRC} ${FREETYPE_SRC} ${PNG_SRC} ${ZLIB_SRC} ${EXPAT_SRC} ${LIBAIFF_SRC} ${SQLITE_SRC} ${GLEW_SRC})

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
  set(ALL_SRC ${ALL_SRC} ${PNGARM_SRC})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  set_source_files_properties(
    ${FREETYPE_SRC} ${EXPAT_SRC} ${LIBAIFF_SRC} ${SQLITE_SRC} ${ZLIB_SRC}
    PROPERTIES
    COMPILE_OPTIONS "-Wno-everything"
  ) 
else()
  set(ALL_SRC ${ALL_SRC} ${MIDI_SRC})
endif()

if(UNIX OR WIN32)
  set(ALL_SRC ${ALL_SRC} ${MDNS_SRC})
endif()

add_library(MDStudio STATIC ${ALL_SRC})

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  target_compile_definitions(MDStudio PRIVATE TARGET_OS_IPHONE)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries(MDStudio rtmidi portaudio_static ${CMAKE_DL_LIBS})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "iOS" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_definitions(MDStudio PRIVATE Z_HAVE_UNISTD_H)
endif()

target_compile_definitions(MDStudio PRIVATE FT2_BUILD_LIBRARY HAVE_MEMMOVE XML_STATIC)

# Additional compile definitions for EXPAT
target_compile_definitions(MDStudio PRIVATE HAVE_MEMMOVE XML_STATIC)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_definitions(MDStudio PRIVATE _LINUX LUA_USE_POSIX)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_definitions(MDStudio PUBLIC GLEW_STATIC)
  target_include_directories(MDStudio PUBLIC ${GLEW}/include)
endif()

target_include_directories(MDStudio PUBLIC ${PORTABLECORE} ${PORTABLECOREAUDIO} ${PORTABLECOREUI} ${PORTABLECOREDB} ${PORTABLECOREAUDIO}/Filters/include ${LRDB}/include ${PICOJSON} ${ASIO}/include ${FREETYPE}/include ${PNG} ${ZLIB} ${LIBAIFF} ${LUA}/src ${RTMIDI} ${EXPAT}/lib ${SQLITE} ${CPPHTTPLIB})
