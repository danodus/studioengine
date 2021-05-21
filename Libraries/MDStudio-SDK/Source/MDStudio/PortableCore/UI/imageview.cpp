//
//  imageview.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-15.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "imageview.h"

#include <math.h>

#include "draw.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
ImageView::ImageView(const std::string& name, void* owner, std::shared_ptr<Image> image, bool isStretched)
    : _image(image), _isStretched(isStretched), View(name, owner) {
    _color = whiteColor;
}

// ---------------------------------------------------------------------------------------------------------------------
ImageView::~ImageView() {}

// ---------------------------------------------------------------------------------------------------------------------
void ImageView::draw() {
    DrawContext* dc = drawContext();

    if (_image) {
        Rect r = _isStretched
                     ? bounds()
                     : makeCenteredRectInRect(bounds(), floorf(_image->size().width), floorf(_image->size().height));
        dc->drawImage(r, _image, _color);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void ImageView::setImage(std::shared_ptr<Image> image) {
    _image = image;
    setDirty();
}
