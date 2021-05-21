//
//  db.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2014-06-15.
//  Copyright (c) 2014-2020 Daniel Cliche. All rights reserved.
//

#include "db.h"

#include <string.h>

#include <iostream>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
static int callback(void* sender, int argc, char** argv, char** azColName) {
    DB* db = static_cast<DB*>(sender);
    int i;

    std::vector<std::pair<std::string, std::string>> columns;

    for (i = 0; i < argc; i++) {
        columns.push_back(std::make_pair(std::string(azColName[i]), std::string(argv[i] ? argv[i] : "")));
    }
    db->addRow(columns);
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
bool DB::open(const char* path) {
    int rc = sqlite3_open(path, &_db);
    if (rc) {
        sqlite3_close(_db);
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool DB::exec(const char* command, bool rollbackOnError) {
    char* zErrMsg = nullptr;
    int rc = sqlite3_exec(_db, command, callback, this, &zErrMsg);
    if (rc != SQLITE_OK) {
        if (zErrMsg) {
            std::cout << "SQL error: " << zErrMsg << std::endl;
            std::cout << "  Command: " << command << std::endl;
            sqlite3_free(zErrMsg);
        }
        if (rollbackOnError) exec("ROLLBACK;", false);
    }
    return rc == SQLITE_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
bool DB::readBlob(const char* command, char** blob, size_t* size, bool rollbackOnError) {
    sqlite3_stmt* stmt;
    int rc;

    // In case there is no table entry for key zKey or an error occurs,
    // set *pzBlob and *pnBlob to 0 now.
    *blob = 0;
    *size = 0;

    // Compile the SELECT statement into a virtual machine.
    rc = sqlite3_prepare_v2(_db, command, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        return false;
    }

    // Run the virtual machine. We can tell by the SQL statement that
    // at most 1 row will be returned. So call sqlite3_step() once
    // only. Normally, we would keep calling sqlite3_step until it
    // returned something other than SQLITE_ROW.
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        // The pointer returned by sqlite3_column_blob() points to memory
        // that is owned by the statement handle (pStmt). It is only good
        // until the next call to an sqlite3_XXX() function (e.g. the
        // sqlite3_finalize() below) that involves the statement handle.
        // So we need to make a copy of the blob into memory obtained from
        // malloc() to return to the caller.
        *size = sqlite3_column_bytes(stmt, 0);
        *blob = (char*)malloc(*size);
        memcpy(*blob, sqlite3_column_blob(stmt, 0), *size);
    }

    // Finalize the statement (this releases resources allocated by
    // sqlite3_prepare() ).
    rc = sqlite3_finalize(stmt);

    if (rc != SQLITE_OK) {
        const char* zErrMsg = sqlite3_errmsg(_db);
        if (zErrMsg) {
            std::cout << "SQL error: " << zErrMsg << std::endl;
        }
        if (rollbackOnError) exec("ROLLBACK;", false);
    }

    return rc == SQLITE_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
bool DB::writeBlob(const char* command, char* blob, size_t size, bool rollbackOnError) {
    sqlite3_stmt* stmt;
    int rc;

    // Compile the INSERT statement into a virtual machine.
    rc = sqlite3_prepare_v2(_db, command, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        return false;
    }

    // Bind blob
    sqlite3_bind_blob(stmt, 1, blob, static_cast<int>(size), SQLITE_STATIC);

    // Call sqlite3_step() to run the virtual machine. Since the SQL being
    // executed is not a SELECT statement, we assume no data will be returned.
    sqlite3_step(stmt);

    // Finalize the virtual machine. This releases all memory and other
    // resources allocated by the sqlite3_prepare() call above.
    rc = sqlite3_finalize(stmt);

    if (rc != SQLITE_OK) {
        const char* zErrMsg = sqlite3_errmsg(_db);
        if (zErrMsg) {
            std::cout << "SQL error: " << zErrMsg << std::endl;
        }
        if (rollbackOnError) exec("ROLLBACK;", false);
    }

    return rc == SQLITE_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
sqlite3_int64 DB::lastInsertID() { return sqlite3_last_insert_rowid(_db); }

// ---------------------------------------------------------------------------------------------------------------------
void DB::close() { sqlite3_close(_db); }

// ---------------------------------------------------------------------------------------------------------------------
void DB::printResults() {
    for (auto row : _rows) {
        std::cout << "Row: ";
        for (auto column : row) {
            std::cout << column.first << " = " << column.second << ", ";
        }
        std::cout << std::endl;
    }
}
