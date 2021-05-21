//
//  image.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-15.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <string>

#include "size.h"

namespace MDStudio {

class Image {
    bool _textureLoaded = false;
    unsigned char* _data;
    int _width, _height;
    int _textureWidth, _textureHeight;
    uint32_t _textureID;
    float _scale;

    bool loadPNGImage(const char* name);

   public:
    Image(const std::string& path, bool isExternal = false);
    ~Image();

    Size size() { return makeSize(static_cast<float>(_width / _scale), static_cast<float>(_height / _scale)); }

    Size internalSize() { return makeSize(static_cast<float>(_width), static_cast<float>(_height)); }
    Size internalTextureSize() {
        return makeSize(static_cast<float>(_textureWidth), static_cast<float>(_textureHeight));
    }

    void bindTexture();
};

class SystemImages {
    std::shared_ptr<Image> _starEmptyImage;
    std::shared_ptr<Image> _starFilledImage;
    std::shared_ptr<Image> _upArrowImage;
    std::shared_ptr<Image> _downArrowImage;
    std::shared_ptr<Image> _leftArrowImage;
    std::shared_ptr<Image> _rightArrowImage;
    std::shared_ptr<Image> _sliderThumbImage;
    std::shared_ptr<Image> _checkMarkImage;
    std::shared_ptr<Image> _radioButtonImage;
    std::shared_ptr<Image> _crossCircleImage;

   public:
    SystemImages();

    static SystemImages* sharedInstance();

    std::shared_ptr<Image> starEmptyImage() { return _starEmptyImage; }
    std::shared_ptr<Image> starFilledImage() { return _starFilledImage; }
    std::shared_ptr<Image> upArrowImage() { return _upArrowImage; }
    std::shared_ptr<Image> downArrowImage() { return _downArrowImage; }
    std::shared_ptr<Image> leftArrowImage() { return _leftArrowImage; }
    std::shared_ptr<Image> rightArrowImage() { return _rightArrowImage; }
    std::shared_ptr<Image> sliderThumbImage() { return _sliderThumbImage; }
    std::shared_ptr<Image> checkMarkImage() { return _checkMarkImage; }
    std::shared_ptr<Image> radioButtonImage() { return _radioButtonImage; }
    std::shared_ptr<Image> crossCircleImage() { return _crossCircleImage; }
};

}  // namespace MDStudio

#endif  // IMAGE_H
