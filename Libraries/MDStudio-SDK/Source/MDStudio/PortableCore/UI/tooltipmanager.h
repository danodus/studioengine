//
//  tooltipmanager.h
//  MDStudio
//
//  Created by Daniel Cliche on 2018-09-07.
//  Copyright (c) 2018-2021 Daniel Cliche. All rights reserved.
//

#ifndef TOOLTIPMANAGER_H
#define TOOLTIPMANAGER_H

#include "tooltip.h"

namespace MDStudio {

class TooltipManager {
    View* _topView;
    std::vector<View*> _views;
    View* _currentTooltipView;
    Tooltip* _tooltip;

    void hideTooltip();

    void viewMouseDidEnter(View* sender, View* view);
    void viewMouseDidLeave(View* sender, View* view);
    void viewMouseDidDetectButtonActivity(View* sender, View* view);

   public:
    TooltipManager(View* topView);
    ~TooltipManager();
    void addTooltip(View* view);
    void removeTooltip(View* view);
    void removeAllTooltips();
};

}  // namespace MDStudio

#endif  // TOOLTIP_MANAGER
