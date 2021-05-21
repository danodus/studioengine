//
//  column.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2017-02-13.
//  Copyright (c) 2017-2020 Daniel Cliche. All rights reserved.
//

#include "column.h"

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
Column::Column(std::string title, float width, bool isResizable, bool isSortAvailable, bool isSortEnabled,
               SortDirection sortDirection)
    : _title(title),
      _width(width),
      _isResizable(isResizable),
      _isSortAvailable(isSortAvailable),
      _isSortEnabled(isSortEnabled),
      _sortDirection(sortDirection) {
    _font = SystemFonts::sharedInstance()->semiboldFont();
}
