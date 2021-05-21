//
//  image.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-15.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "image.h"

#include <png.h>
#include <stdlib.h>
#include <string.h>

#include "../platform.h"

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
Image::Image(const std::string& path, bool isExternal) {
    _textureLoaded = false;
    _data = nullptr;
    _scale = 1.0f;
    _width = 0.0f;
    _height = 0.0f;
    _textureWidth = 0.0f;
    _textureHeight = 0.0f;

    std::string fullPath =
        isExternal ? path : MDStudio::Platform::sharedInstance()->resourcesPath() + std::string("/") + path;

    loadPNGImage(fullPath.c_str());
}

// ---------------------------------------------------------------------------------------------------------------------
void Image::bindTexture() {
    // TODO: We should not load the texture here because it prevents us to keep the function constant
    if (!_textureLoaded) {
        glGenTextures(1, &_textureID);
        glBindTexture(GL_TEXTURE_2D, _textureID);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _textureWidth, _textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data);
        _textureLoaded = true;
    } else {
        glBindTexture(GL_TEXTURE_2D, _textureID);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
Image::~Image() {
    if (_textureLoaded) glDeleteTextures(1, &_textureID);

    if (_data != nullptr) free(_data);
}

// ---------------------------------------------------------------------------------------------------------------------
bool Image::loadPNGImage(const char* name) {
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    int color_type, interlace_type;
    FILE* fp;

    if ((fp = fopen(name, "rb")) == NULL) return false;

    std::string str = std::string(name);
    size_t found = str.find_last_of("/\\");
    std::string strName = str.substr(found + 1);
    if (strName.find("@2x") != std::string::npos) {
        _scale = 2.0f;
    }

    // Create and initialize the png_struct with the desired error handler functions.  If you want to use the
    // default stderr and longjump method, you can supply NULL for the last three parameters.  We also supply the
    // the compiler header file version, so that we know if the application was compiled with a compatible version
    // of the library.
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png_ptr == NULL) {
        fclose(fp);
        return false;
    }

    // Allocate/initialize the memory for image information.
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return false;
    }

    // Set error handling if you are using the setjmp/longjmp method (this is the normal method of
    // doing things with libpng).
    if (setjmp(png_jmpbuf(png_ptr))) {
        // Free all of the memory associated with the png_ptr and info_ptr
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        // If we get here, we had a problem reading the file
        return false;
    }

    // Set up the output control if you are using standard C streams
    png_init_io(png_ptr, fp);

    // If we have already read some of the signature
    png_set_sig_bytes(png_ptr, sig_read);

    // If you have enough memory to read in the entire image at once, and you need to specify only
    // transforms that can be controlled with one of the PNG_TRANSFORM_* bits (this presently excludes
    // dithering, filling, setting background, and doing gamma adjustment), then you can read the
    // entire image (including pixels) into the info structure with this call
    //
    // PNG_TRANSFORM_STRIP_16 |
    // PNG_TRANSFORM_PACKING  forces 8 bit
    // PNG_TRANSFORM_EXPAND forces to
    //  expand a palette into RGB
    //
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);

    png_uint_32 width, height;
    int bit_depth;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
    _width = width;
    _height = height;
    _textureWidth = next_p2(_width);
    _textureHeight = next_p2(_height);

    size_t row_bytes = _textureWidth * 4;
    _data = (unsigned char*)malloc(_textureWidth * _textureHeight * 4);

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    // PNG is ordered top to bottom, but OpenGL expect it bottom to top so the order or swapped
    for (int i = 0; i < _height; i++) {
        memcpy(_data + (row_bytes * (_height - 1 - i)), row_pointers[i], _width * 4);
    }

    // Clean up after the read, and free any memory allocated
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    // Close the file
    fclose(fp);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
SystemImages::SystemImages() {
    _starEmptyImage = std::make_shared<Image>("StarEmptySmall@2x.png");
    _starFilledImage = std::make_shared<Image>("StarFilledSmall@2x.png");
    _upArrowImage = std::make_shared<Image>("UpArrow@2x.png");
    _downArrowImage = std::make_shared<Image>("DownArrow@2x.png");
    _leftArrowImage = std::make_shared<Image>("LeftArrow@2x.png");
    _rightArrowImage = std::make_shared<Image>("RightArrow@2x.png");
    _sliderThumbImage = std::make_shared<Image>("SliderThumb@2x.png");
    _checkMarkImage = std::make_shared<Image>("CheckMark@2x.png");
    _radioButtonImage = std::make_shared<Image>("RadioButton@2x.png");
    _crossCircleImage = std::make_shared<Image>("CrossCircle@2x.png");
}

// ---------------------------------------------------------------------------------------------------------------------
SystemImages* SystemImages::sharedInstance() {
    static SystemImages instance;
    return &instance;
}
