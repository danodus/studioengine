//
//  imageview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-15.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <string>

#include "image.h"
#include "view.h"

namespace MDStudio {

class ImageView : public View {
    bool _isStretched;

    std::shared_ptr<Image> _image;
    Color _color;

    void draw() override;

   public:
    ImageView(const std::string& name, void* owner, std::shared_ptr<Image> image, bool isStretched = false);
    ~ImageView();

    void setImage(std::shared_ptr<Image> image);

    void setColor(Color color) {
        _color = color;
        setDirty();
    }
    Color color() { return _color; }

    bool isStretched() { return _isStretched; }
    void setIsStretched(bool isStretched) {
        _isStretched = isStretched;
        setDirty();
    }
};

}  // namespace MDStudio

#endif  // IMAGEVIEW_H
