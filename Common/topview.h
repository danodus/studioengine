//
//  topview.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-07-12.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#ifndef TOPVIEW_H
#define TOPVIEW_H

#include <boxview.h>
#include <button.h>
#include <imageview.h>
#include <sheet.h>
#include <splitviewh.h>
#include <ui.h>

#include <functional>
#include <memory>
#include <vector>

#include "dbview.h"
#include "sequenceview.h"
#include "studiocontroller.h"
#include "studioview.h"

class TopView : public MDStudio::View {
   public:
    typedef std::function<bool(TopView* sender, const MDStudio::UIEvent* event)> HandleEventFnType;

   private:
    MDStudio::UI _ui;
    MDStudio::UI* _controlsViewUI;

    std::shared_ptr<DBView> _dbView;
    std::shared_ptr<MDStudio::View> _controlsView;
    std::shared_ptr<SequenceView> _sequenceView;
    std::shared_ptr<StudioView> _studioView;
    std::shared_ptr<MDStudio::SplitViewH> _horizSplitView;
    std::shared_ptr<MDStudio::View> _aboutView;
    std::shared_ptr<MDStudio::View> _preferencesView;
    std::shared_ptr<MDStudio::View> _confirmCleanupView;
    std::shared_ptr<MDStudio::View> _confirmEmptyTrashView;
    std::shared_ptr<MDStudio::View> _confirmSetAsPlayedAllView;
    std::shared_ptr<MDStudio::View> _acknowledgementsView;
    std::shared_ptr<MDStudio::View> _audioExportView;
    std::shared_ptr<MDStudio::View> _midiImportView;
    std::shared_ptr<MDStudio::View> _newSequenceView;

    std::shared_ptr<MDStudio::Image> _playingImage;
    std::shared_ptr<MDStudio::Image> _recordingImage;
    std::shared_ptr<MDStudio::Image> _metronomeLearningImage;
    std::shared_ptr<MDStudio::Image> _learnTempoImage;

    std::shared_ptr<MDStudio::Sheet> _splitSheet;
    std::shared_ptr<MDStudio::Sheet> _learnTempoSheet;
    std::shared_ptr<MDStudio::LabelView> _splitLabelView0, _splitLabelView1;
    std::shared_ptr<MDStudio::Button> _cancelButton;

    std::shared_ptr<MDStudio::ImageView> _learnTempoImageView;
    std::shared_ptr<MDStudio::LabelView> _learnTempoLabelView;

    std::shared_ptr<MDStudio::BoxView> _loadingBoxView;

    HandleEventFnType _handleEventFn;

    std::string _untitledStr;
    std::string _learnTempoStr[3];

   public:
    TopView(std::string name, void* owner, MDStudio::Studio* studio, double eventTickWidth, float eventHeight);
    ~TopView();

    std::shared_ptr<SequenceView> sequenceView() { return _sequenceView; }
    std::shared_ptr<StudioView> studioView() { return _studioView; }
    std::shared_ptr<DBView> dbView() { return _dbView; }
    std::shared_ptr<MDStudio::BoxView> loadingBoxView() { return _loadingBoxView; }
    std::shared_ptr<MDStudio::Button> rewindButton() {
        return dynamic_pointer_cast<MDStudio::Button>(_controlsViewUI->findView("rewindButton"));
    }
    std::shared_ptr<MDStudio::Button> playButton() {
        return dynamic_pointer_cast<MDStudio::Button>(_controlsViewUI->findView("playButton"));
    }
    std::shared_ptr<MDStudio::Button> recordButton() {
        return dynamic_pointer_cast<MDStudio::Button>(_controlsViewUI->findView("recordButton"));
    }
    std::shared_ptr<MDStudio::Button> convertSequenceButton() {
        return dynamic_pointer_cast<MDStudio::Button>(_controlsViewUI->findView("convertSequenceButton"));
    }
    std::shared_ptr<MDStudio::SplitViewH> horizSplitView() { return _horizSplitView; }
    std::shared_ptr<MDStudio::ImageView> statusImageView() {
        return dynamic_pointer_cast<MDStudio::ImageView>(_controlsViewUI->findView("statusImageView"));
    }

    std::shared_ptr<MDStudio::Image> playImage() { return _controlsViewUI->findImage("playImage"); }
    std::shared_ptr<MDStudio::Image> pauseImage() { return _controlsViewUI->findImage("pauseImage"); }
    std::shared_ptr<MDStudio::Image> playingImage() { return _playingImage; }
    std::shared_ptr<MDStudio::Image> recordingImage() { return _recordingImage; }
    std::shared_ptr<MDStudio::Image> metronomeLearningImage() { return _metronomeLearningImage; }
    std::shared_ptr<MDStudio::Image> convertToMTImage() { return _controlsViewUI->findImage("convertToMTImage"); }
    std::shared_ptr<MDStudio::Image> convertToSTImage() { return _controlsViewUI->findImage("convertToSTImage"); }

    bool handleEvent(const MDStudio::UIEvent* event) override;
    void setFrame(MDStudio::Rect rect) override;

    void setHandleEventFn(HandleEventFnType handleEvent) { _handleEventFn = handleEvent; }

    std::shared_ptr<MDStudio::Sheet> splitSheet() { return _splitSheet; }
    std::shared_ptr<MDStudio::Sheet> learnTempoSheet() { return _learnTempoSheet; }
    std::shared_ptr<MDStudio::LabelView> learnTempoLabelView() { return _learnTempoLabelView; }
    std::shared_ptr<MDStudio::Button> cancelButton() { return _cancelButton; }
    std::shared_ptr<MDStudio::ImageView> learnTempoImageView() { return _learnTempoImageView; }
    std::shared_ptr<MDStudio::View> aboutView() { return _aboutView; }
    std::shared_ptr<MDStudio::View> confirmCleanupView() { return _confirmCleanupView; }
    std::shared_ptr<MDStudio::View> confirmEmptyTrashView() { return _confirmEmptyTrashView; }
    std::shared_ptr<MDStudio::View> confirmSetAsPlayedAllView() { return _confirmSetAsPlayedAllView; }
    std::shared_ptr<MDStudio::View> acknowledgementsView() { return _acknowledgementsView; }
    std::shared_ptr<MDStudio::View> audioExportView() { return _audioExportView; }
    std::shared_ptr<MDStudio::View> midiImportView() { return _midiImportView; }
    std::shared_ptr<MDStudio::View> preferencesView() { return _preferencesView; }
    std::shared_ptr<MDStudio::View> newSequenceView() { return _newSequenceView; }

    std::string untitledStr() { return _untitledStr; }
    std::string learnTempoStr(int index) { return _learnTempoStr[index]; }

    MDStudio::UI* controlsViewUI() { return _controlsViewUI; }
};

#endif  // TOPVIEW_H
