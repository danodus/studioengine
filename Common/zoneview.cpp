//
//  zoneview.cpp
//  MelobaseStation
//
//  Created by Daniel Cliche on 2016-02-08.
//  Copyright (c) 2016-2021 Daniel Cliche. All rights reserved.
//

#include "zoneview.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
ZoneView::ZoneView(std::string name, void* owner, int zone) : View(name, owner) {
    _channel = zone;

    std::vector<Any> items;
    for (int i = 0; i < STUDIO_MAX_CHANNELS; ++i) items.push_back(std::to_string(i + 1));

    _channelSegmentedControl = std::make_shared<MDStudio::SegmentedControl>("channelSegmentedControl", this, items);
    _channelSegmentedControl->setFont(SystemFonts::sharedInstance()->semiboldFontSmall());
    addSubview(_channelSegmentedControl);

    _channelSegmentedControl->setSelectedSegment(_channel);

    _transposeLabelImage = std::make_shared<Image>("TransposeLabel@2x.png");
    _transposeLabelImageView = std::make_shared<ImageView>("transposeLabelImageView", this, _transposeLabelImage);

    _transposeBoxView = std::make_shared<BoxView>("transposeBoxView", this);
    _transposeLabelView = std::make_shared<LabelView>("transposeLabelView", this, "0");
    _transposeStepper = std::make_shared<Stepper>("transposeStepper", this, 12, -48, 48, 0);

    _studioChannelView = std::make_shared<StudioChannelView>("studioChannelView", this, _channel);

    addSubview(_studioChannelView);
    addSubview(_transposeBoxView);
    addSubview(_transposeLabelView);
    addSubview(_transposeStepper);
    addSubview(_transposeLabelImageView);
}

// ---------------------------------------------------------------------------------------------------------------------
ZoneView::~ZoneView() {}

// ---------------------------------------------------------------------------------------------------------------------
void ZoneView::setFrame(Rect aRect) {
    View::setFrame(aRect);

    MDStudio::Rect r = bounds();

    r.origin.x = 60.0f;
    r.origin.y = r.size.height - 18.0f;
    r.size.height = 16.0f;
    r.size.width -= 60.0f;
    r = MDStudio::makeCenteredRectInRect(r, STUDIO_MAX_CHANNELS * 20.0f, 16.0f);
    _channelSegmentedControl->setFrame(r);

    MDStudio::Rect r2 = MDStudio::makeCenteredRectInRect(
        makeRect(0.0f, 0.0f, bounds().size.width, bounds().size.height - r.size.height), 440.0f, 140.0f);

    auto r3 = r2;
    if (r3.origin.x < 0.0f) r3.origin.x = 0.0f;
    _transposeStepper->setFrame(makeRect(r3.origin.x + 0.0f, r3.origin.y + 40.0f, 20.0f, 30.0f));
    _transposeBoxView->setFrame(makeRect(r3.origin.x + 20.0f, r3.origin.y + 40.0f, 40.0f, 30.0f));
    _transposeLabelView->setFrame(makeInsetRect(_transposeBoxView->frame(), 5, 5));
    _transposeLabelImageView->setFrame(makeRect(r3.origin.x, r3.origin.y + 30.0f + 40.0f, 60.0f, 20.0f));

    _studioChannelView->setFrame(makeRect(r2.origin.x + 60.0f, r2.origin.y, r2.size.width - 60.0f, r2.size.height));
}
