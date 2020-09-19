//
// Created by Kerry Cao on 2020-09-18.
//

#include "../include/SQLITE3.hpp"

SQLITE3::SQLITE3(const std::string &db_name) {
    this->err_msg = nullptr;
    this->stmt = nullptr;
    this->db = nullptr;

    // open database if name is provided
    if (!db_name.empty()) {
        int rc = sqlite3_open(db_name.c_str(), &db);
        if (rc != SQLITE_OK) { // check for error
            error_no = 1; // set error code

            sqlite3_close(db);
            throw std::runtime_error("Unable to open database");
        }
        start_transaction();
    }

    // initialize result vector
    result = std::unique_ptr<std::vector<SQLITE_ROW_VECTOR>>(new std::vector<SQLITE_ROW_VECTOR>);
}

SQLITE3::~SQLITE3() {
    if (db) {
        sqlite3_close(db);
    }
    if (err_msg) {
        sqlite3_free(err_msg);
    }
    if (stmt) {
        sqlite3_finalize(stmt);
    }
}

int SQLITE3::open(std::string &db_name) {
    if (db) { // can only bind to 1 database
        error_no = 2;
        return 1;
    } else {
        int rc = sqlite3_open(db_name.c_str(), &db);

        if (rc != SQLITE_OK) { // check for error
            error_no = 1; // set error code

            sqlite3_close(db);
            return 1;
        }

        start_transaction();
        return 0; // all good
    }
}

int SQLITE3::start_transaction() {
    int rc = sqlite3_exec(db, "BEGIN;", nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) { // check for error
        error_no = 127;
        return 1;
    }
    return 0;
}

int SQLITE3::commit() {
    int rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) { // check for error
        error_no = 127;
        return 1;
    }
    start_transaction();
    return 0;
}

void SQLITE3::perror() const {
    switch (error_no) {
        case 0:
            break;
        case 1:
            std::cerr << "SQLITE DATABASE OPEN FAILURE\n";
            break;
        case 2:
            std::cerr << "SQLITE DATABASE ALREADY OPENED, CREATE NEW OBJECT FOR NEW DATABASE\n";
            break;
        case 3:
            std::cerr << "Query Binding Failed\n";
            break;
        case 4:
            std::cerr << "No database connected\n";
            break;
        case 127:
            std::cerr << err_msg << std::endl;
            break;
    }
}

int SQLITE3::execute(SQLITE3_QUERY &query) {
    // check if database connection is open
    if (!db) {
        error_no = 4;
        return 1;
    }

    // get query from SQLITE3_QUERY
    std::string prepared_query;
    try {
        prepared_query = query.bind().bound_query;
    } catch (std::out_of_range &e) {
        error_no = 3;
        return 1;
    }

    // clear result vector
    result->clear();

    // run query
    int rc = sqlite3_exec(db, prepared_query.c_str(), &exec_callback, &result, &err_msg);
    if (rc != SQLITE_OK) { // check for error
        error_no = 127;
        return 1;
    }
    return 0;
}

int SQLITE3::execute(std::string &query) {
    // check if database connection is open
    if (!db) {
        error_no = 4;
        return 1;
    }

    // clear result vector
    result->clear();

    // run query
    int rc = sqlite3_exec(db, query.c_str(), &exec_callback, &result, &err_msg);
    if (rc != SQLITE_OK) { // check for error
        error_no = 127;
        return 1;
    }
    return 0;
}

int SQLITE3::exec_callback(void *ptr, int argc, char **argv, char **col_name) {
    auto *result = reinterpret_cast<std::unique_ptr<std::vector<SQLITE_ROW_VECTOR>> *>(ptr);

    // get result
    SQLITE_ROW_VECTOR row;
    for(int i = 0; i < argc; i++) {
         row.push_back(std::string(argv[i] ? argv[i] : "NULL"));
    }

    // push SQLITE_ROW_VECTOR to result vector
    result->get()->push_back(row);

    return 0;
}

std::vector<SQLITE_ROW_VECTOR> *SQLITE3::get_result() {
    return result.get();
}












