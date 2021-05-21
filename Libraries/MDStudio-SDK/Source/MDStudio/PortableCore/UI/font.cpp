//
//  font.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2013-09-09.
//  Copyright (c) 2013-2020 Daniel Cliche. All rights reserved.
//

#include "font.h"

#include "../platform.h"

// Using the STL exception library increases the
// chances that someone else using our code will corretly
// catch any exceptions that we throw.
#include <assert.h>

#include <stdexcept>

#ifdef _WIN32

#define NOMINMAX
#include <GL/glew.h>
#include <Windows.h>

#elif _LINUX

#include <SDL2/SDL_opengl.h>

#else

#if TARGET_OS_IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#else
#include <OpenGL/gl.h>
#endif

#endif

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
/// This function gets the first power of 2 >= the
/// int that we pass it.
inline int next_p2(int a) {
    int rval = 2;
    while (rval < a) rval <<= 1;
    return rval;
}

// ---------------------------------------------------------------------------------------------------------------------
void Font::loadChar(FT_Face face, FT_UInt ch, GLuint* tex_base) {
    // The first thing we do is get FreeType to render our character
    // into a bitmap.  This actually requires a couple of FreeType commands:

    // Load the Glyph for our character.
    if (FT_Load_Glyph(face, ch, FT_LOAD_DEFAULT)) throw std::runtime_error("FT_Load_Glyph failed");

    // Move the face's glyph into a Glyph object.
    FT_Glyph glyph;
    if (FT_Get_Glyph(face->glyph, &glyph)) throw std::runtime_error("FT_Get_Glyph failed");

    // Convert the glyph to a bitmap.
    FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

    // This reference will make accessing the bitmap easier
    FT_Bitmap& bitmap = bitmap_glyph->bitmap;

    assert(ch < _nbGlyphs);

    // Use our helper function to get the widths of
    // the bitmap data that we will need in order to create
    // our texture.
    int width = next_p2(bitmap.width);
    int height = next_p2(bitmap.rows);

    // Allocate memory for the texture data.
    GLubyte* expanded_data = new GLubyte[2 * width * height];

    // Here we fill in the data for the expanded bitmap.
    // Notice that we are using two channel bitmap (one for
    // luminocity and one for alpha), but we assign
    // both luminocity and alpha to the value that we
    // find in the FreeType bitmap.
    // We use the ?: operator so that value which we use
    // will be 0 if we are in the padding zone, and whatever
    // is the the Freetype bitmap otherwise.
    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++) {
            expanded_data[2 * (i + j * width)] = 255;
            expanded_data[2 * (i + j * width) + 1] =
                (i >= bitmap.width || j >= bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width * j];
        }

    // Now we just setup some texture paramaters.
    glBindTexture(GL_TEXTURE_2D, tex_base[ch]);

    // Here we actually create the texture itself, notice
    // that we are using GL_LUMINANCE_ALPHA to indicate that
    // we are using 2 channel data.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
                 expanded_data);

    // With the texture created, we don't need to expanded data anymore
    delete[] expanded_data;

    _bitmapGlyphs[ch].left = bitmap_glyph->left;
    _bitmapGlyphs[ch].top = bitmap_glyph->top;
    _bitmapGlyphs[ch].width = bitmap_glyph->bitmap.width;
    _bitmapGlyphs[ch].rows = bitmap_glyph->bitmap.rows;

    FT_Done_Glyph(glyph);
}

// ---------------------------------------------------------------------------------------------------------------------
/// Create a display list coresponding to the give character.
void Font::drawChar(FT_Face face, FT_UInt ch, GLuint* tex_base) const {
    BitmapGlyph bitmap_glyph = _bitmapGlyphs[ch];

    int width = next_p2(bitmap_glyph.width);
    int height = next_p2(bitmap_glyph.rows);

    // So now we can create the display list
    // glNewList(list_base + ch, GL_COMPILE);

    glBindTexture(GL_TEXTURE_2D, tex_base[ch]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glPushMatrix();

    // first we need to move over a little so that
    // the character has the right amount of space
    // between it and the one before it.
    glTranslatef(bitmap_glyph.left / _scale, 0, 0);

    // Now we move down a little in the case that the
    // bitmap extends past the bottom of the line
    // (this is only true for characters like 'g' or 'y'.
    glTranslatef(0, (bitmap_glyph.top - bitmap_glyph.rows) / _scale, 0);

    // Now we need to account for the fact that many of
    // our textures are filled with empty padding space.
    // We figure what portion of the texture is used by
    // the actual character and store that information in
    // the x and y variables, then when we draw the
    // quad, we will only reference the parts of the texture
    // that we contain the character itself.
    float x = (float)bitmap_glyph.width / (float)width, y = (float)bitmap_glyph.rows / (float)height;

    // Here we draw the texturemaped quads.
    // The bitmap that we got from FreeType was not
    // oriented quite like we would like it to be,
    // so we need to link the texture to the quad
    // so that the result will be properly aligned.

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    float vertices[] = {0,
                        bitmap_glyph.rows / _scale,
                        0,
                        0,
                        bitmap_glyph.width / _scale,
                        0,
                        bitmap_glyph.width / _scale,
                        bitmap_glyph.rows / _scale};

    float texCoords[] = {0.0f, 0.0f, 0.0f, y, x, y, x, 0};

    GLubyte indices[] = {0, 1, 2, 0, 2, 3};

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    // glBegin(GL_QUADS);
    // glTexCoord2d(0,0); glVertex2f(0,bitmap.rows / _scale);
    // glTexCoord2d(0,y); glVertex2f(0,0);
    // glTexCoord2d(x,y); glVertex2f(bitmap.width / _scale, 0);
    // glTexCoord2d(x,0); glVertex2f(bitmap.width / _scale, bitmap.rows / _scale);
    // glEnd();
    glPopMatrix();
    glTranslatef(_glyphWidths[ch], 0, 0);
}

// ---------------------------------------------------------------------------------------------------------------------
void Font::init(const char* fname, unsigned int h, float scale) {
    _scale = scale;

    // Create and initilize a freetype font library.
    if (FT_Init_FreeType(&_library)) throw std::runtime_error("FT_Init_FreeType failed");

    // This is where we load in the font information from the file.
    // Of all the places where the code might die, this is the most likely,
    // as FT_New_Face will die if the font file does not exist or is somehow broken.
    if (FT_New_Face(_library, fname, 0, &_face))
        throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");

    // For some twisted reason, Freetype measures font size
    // in terms of 1/64ths of pixels.  Thus, to make a font
    // h pixels high, we need to request a size of h*64.
    //(h << 6 is just a prettier way of writting h*64)
    FT_Set_Char_Size(_face, h << 6, h << 6, _scale * 72, _scale * 72);

    // Get the height
    FT_Size_Metrics metrics = _face->size->metrics;
    _height = static_cast<float>(metrics.height >> 6) / _scale;
    _ascender = static_cast<float>(metrics.ascender >> 6) / _scale;
    _descender = static_cast<float>(metrics.descender >> 6) / _scale;

    _nbGlyphs = _face->num_glyphs;
    _glyphWidths = new float[_nbGlyphs];
    _bitmapGlyphs = new BitmapGlyph[_nbGlyphs];

    _textures = nullptr;

    //
    // Update the glypth widths table
    //

    FT_ULong charcode;
    FT_UInt gindex;

    charcode = FT_Get_First_Char(_face, &gindex);
    while (gindex != 0) {
        assert(gindex < _nbGlyphs);

        FT_UInt ch = gindex;

        // Load the Glyph for our character.
        if (FT_Load_Glyph(_face, ch, FT_LOAD_DEFAULT)) throw std::runtime_error("FT_Load_Glyph failed");

        // Move the face's glyph into a Glyph object.
        FT_Glyph glyph;
        if (FT_Get_Glyph(_face->glyph, &glyph)) throw std::runtime_error("FT_Get_Glyph failed");

        assert(ch < _nbGlyphs);
        _glyphWidths[ch] = static_cast<float>(_face->glyph->advance.x >> 6) / _scale;

        charcode = FT_Get_Next_Char(_face, charcode, &gindex);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void Font::loadTextures() {
    FT_ULong charcode;
    FT_UInt gindex;

    // Allocate some memory to store the texture ids.
    _textures = new GLuint[_nbGlyphs];

    // Here we ask opengl to allocate resources for all the textures and displays lists which we are about to create.
    glGenTextures(static_cast<GLint>(_nbGlyphs), _textures);

    // This is where we actually create each of the fonts display lists.

    charcode = FT_Get_First_Char(_face, &gindex);
    while (gindex != 0) {
        assert(gindex < _nbGlyphs);
        loadChar(_face, gindex, _textures);
        charcode = FT_Get_Next_Char(_face, charcode, &gindex);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
unsigned int Font::charIndex(unsigned long charcode) const { return FT_Get_Char_Index(_face, charcode); }

// ---------------------------------------------------------------------------------------------------------------------
void Font::drawChar(int ch) {
    // TODO: We should not load the textures here because it prevents us to keep the function constant
    if (!_textures) loadTextures();

    drawChar(_face, ch, _textures);
}

// ---------------------------------------------------------------------------------------------------------------------
void Font::clean() {
    delete[] _glyphWidths;
    delete[] _bitmapGlyphs;

    // If the textures was loaded
    if (_textures) {
        glDeleteTextures(static_cast<GLint>(_nbGlyphs), _textures);
        delete[] _textures;
    }

    // We don't need the face information now that the display
    // lists have been created, so we free the assosiated resources.
    FT_Done_Face(_face);

    // Ditto for the library.
    FT_Done_FreeType(_library);
}

// ---------------------------------------------------------------------------------------------------------------------
MultiDPIFont::MultiDPIFont(unsigned int height, std::string path) : _height(height), _path(path) {}

// ---------------------------------------------------------------------------------------------------------------------
Font* MultiDPIFont::fontForScale(float scale) {
    if (scale <= 1.0) {
        if (!_font1x) {
            _font1x = new Font();
            _font1x->init(_path.c_str(), _height, 1.0f);
        }
        return _font1x;
    } else if (scale <= 2.0) {
        if (!_font2x) {
            _font2x = new Font();
            _font2x->init(_path.c_str(), _height, 2.0f);
        }
        return _font2x;
    } else {
        if (!_font4x) {
            _font4x = new Font();
            _font4x->init(_path.c_str(), _height, 4.0f);
        }
        return _font4x;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
MultiDPIFont::~MultiDPIFont() {
    if (_font1x != nullptr) {
        _font1x->clean();
        delete _font1x;
    }

    if (_font2x != nullptr) {
        _font2x->clean();
        delete _font2x;
    }

    if (_font4x != nullptr) {
        _font4x->clean();
        delete _font4x;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
SystemFonts::SystemFonts() {
    std::string path = Platform::sharedInstance()->resourcesPath() + "/OpenSans-Semibold.ttf";
    _semiboldFont = new MultiDPIFont(13, path);
    _semiboldFontSmall = new MultiDPIFont(10, path);
    _semiboldFontTiny = new MultiDPIFont(8, path);

    path = Platform::sharedInstance()->resourcesPath() + "/DroidSansMono.ttf";
    _monoFont = new MultiDPIFont(13, path);
}

// ---------------------------------------------------------------------------------------------------------------------
SystemFonts* SystemFonts::sharedInstance() {
    static SystemFonts instance;
    return &instance;
}

// ---------------------------------------------------------------------------------------------------------------------
SystemFonts::~SystemFonts() {
    if (_semiboldFont != nullptr) delete _semiboldFont;
    if (_semiboldFontSmall != nullptr) delete _semiboldFontSmall;
    if (_semiboldFontTiny != nullptr) delete _semiboldFontTiny;
    if (_monoFont != nullptr) delete _monoFont;
}
