//
//  db.h
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-15.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#ifndef DB_H
#define DB_H

#include <sqlite3.h>

#include <string>
#include <utility>
#include <vector>

namespace MDStudio {
class DB {
    sqlite3* _db;
    std::vector<std::vector<std::pair<std::string, std::string>>> _rows;

   public:
    bool open(const char* path);
    bool exec(const char* command, bool rollbackOnError = false);
    bool readBlob(const char* command, char** blob, size_t* size, bool rollbackOnError = false);
    bool writeBlob(const char* command, char* blob, size_t size, bool rollbackOnError = false);

    sqlite3_int64 lastInsertID();

    void close();

    void clearResults() { _rows.clear(); }
    void addRow(std::vector<std::pair<std::string, std::string>> columns) { _rows.push_back(columns); }
    std::vector<std::vector<std::pair<std::string, std::string>>> rows() { return _rows; }

    void printResults();
};
}  // namespace MDStudio

#endif  // DB_H
