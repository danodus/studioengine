//
//  splitviewmultih.h
//  MDStudio
//
//  Created by Daniel Cliche on 2017-11-18.
//  Copyright (c) 2017-2020 Daniel Cliche. All rights reserved.
//

#ifndef SPLITVIEWMULTIH_H
#define SPLITVIEWMULTIH_H

#include <functional>
#include <memory>

#include "view.h"

#define SPLIT_VIEW_MULTI_H_SPLITTER_WIDTH 6

namespace MDStudio {

class SplitViewMultiH : public View {
   public:
    typedef std::function<void(SplitViewMultiH* sender, std::vector<std::pair<float, bool>> pos)> PosChangedFnType;

   private:
    std::vector<std::pair<float, bool>> _splitPos;
    bool _isPosRelativeToRight;

    void draw() override;
    bool handleEvent(const UIEvent* event) override;

    bool _isCaptured;
    size_t _capturedPosIndex;

    std::vector<std::shared_ptr<View>> _panes;

    PosChangedFnType _posChangedFn;

    void updateFrames();

    bool isPointInAnyRect(Point pt, const std::vector<Rect>& rects, size_t* index);

   public:
    SplitViewMultiH(const std::string& name, void* owner, const std::vector<std::shared_ptr<View>>& panes,
                    const std::vector<std::pair<float, bool>>& splitPos);
    ~SplitViewMultiH();

    void setFrame(Rect rect) override;

    std::vector<std::pair<float, bool>> splitPos() { return _splitPos; }
    void setSplitPos(const std::vector<std::pair<float, bool>>& splitPos);

    void setPosChangedFn(PosChangedFnType posChangedFn) { _posChangedFn = posChangedFn; }
};

}  // namespace MDStudio

#endif  // SPLITVIEWMULTIH_H
