//
//  column.h
//  MDStudio
//
//  Created by Daniel Cliche on 2017-02-13.
//  Copyright (c) 2017-2020 Daniel Cliche. All rights reserved.
//

#ifndef COLUMN_H
#define COLUMN_H

#include <font.h>

#include <string>

namespace MDStudio {

class Column {
   public:
    typedef enum { Ascending, Descending } SortDirection;

   private:
    std::string _title;
    float _width;
    bool _isResizable;
    SortDirection _sortDirection;
    bool _isSortAvailable;
    bool _isSortEnabled;
    MultiDPIFont* _font;

   public:
    Column(std::string title, float width, bool isResizable, bool isSortAvailable, bool isSortEnabled,
           SortDirection sortDirection);

    std::string title() { return _title; }
    void setTitle(std::string title) { _title = title; }

    bool isSortAvailable() { return _isSortAvailable; }

    bool isSortEnabled() { return _isSortEnabled; }
    void setIsSortEnabled(bool isSortEnabled) { _isSortEnabled = isSortEnabled; }

    SortDirection sortDirection() { return _sortDirection; }
    void setSortDirection(SortDirection sortDirection) { _sortDirection = sortDirection; }

    float width() { return _width; }
    void setWidth(float width) { _width = width; }

    bool isResizable() { return _isResizable; }
    void setIsResizable(bool isResizable) { _isResizable = isResizable; }

    MultiDPIFont* font() { return _font; }
    void setFont(MultiDPIFont* font) { _font = font; }
};
}  // namespace MDStudio

#endif  // COLUMN_H
