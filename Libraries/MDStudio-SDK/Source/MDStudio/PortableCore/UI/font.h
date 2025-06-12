//
//  font.h
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2020 Daniel Cliche. All rights reserved.
//

#ifndef FONT_H
#define FONT_H

// FreeType Headers

// clang-format off
#include <ft2build.h>
#include <freetype.h>
// clang-format on
#include <ftglyph.h>
#include <ftoutln.h>
#include <fttrigon.h>

#include <string>
#include <cstdint>

namespace MDStudio {

struct BitmapGlyph {
    int width;
    int height;
    int left;
    int top;
    int rows;
};

class Font {
    void makeDlist(FT_Face face, FT_UInt ch, uint32_t list_base, uint32_t* tex_base);

    unsigned long _nbGlyphs;
    float* _glyphWidths;
    BitmapGlyph* _bitmapGlyphs;

    FT_Library _library;
    FT_Face _face;

    uint32_t* _textures;  ///< Holds the texture id's
    uint32_t _listBase;   ///< Holds the first display list id

    float _height;
    float _ascender;
    float _descender;

    float _scale;

    void loadTextures();

    void loadChar(FT_Face face, FT_UInt ch, uint32_t* tex_base);

    void drawChar(FT_Face face, FT_UInt ch, uint32_t* tex_base) const;

   public:
    // The init function will create a font of
    // of the height h from the file fname.
    void init(const char* fname, unsigned int h, float scale);

    // Free all the resources assosiated with the font.
    void clean();

    float height() const { return _height; }
    float ascender() const { return _ascender; }
    float descender() const { return _descender; }

    unsigned int charIndex(unsigned long charcode) const;
    float glyphWidth(size_t index) const { return _glyphWidths[index]; }

    void drawChar(int ch);
};

class MultiDPIFont {
    Font* _font1x = nullptr;
    Font* _font2x = nullptr;
    Font* _font4x = nullptr;

    unsigned int _height;
    std::string _path;

   public:
    MultiDPIFont(unsigned int height, std::string path);
    ~MultiDPIFont();

    Font* fontForScale(float scale);
};

class SystemFonts {
    ~SystemFonts();
    MultiDPIFont *_semiboldFont, *_semiboldFontSmall, *_semiboldFontTiny, *_monoFont;

   public:
    SystemFonts();
    static SystemFonts* sharedInstance();

    MultiDPIFont* semiboldFont() { return _semiboldFont; }
    MultiDPIFont* semiboldFontSmall() { return _semiboldFontSmall; }
    MultiDPIFont* semiboldFontTiny() { return _semiboldFontTiny; }
    MultiDPIFont* monoFont() { return _monoFont; }
};

}  // namespace MDStudio

#endif  // FONT_H
