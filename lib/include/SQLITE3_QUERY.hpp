//
// Created by Kerry Cao on 2020-09-18.
// SQLitePlus
//    Copyright (C) <2020>  <Yuqian Cao> (kcyq98@gmail.com)
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
//    USA
//

#ifndef SQLITEPLUS_SQLITE3_QUERY_HPP
#define SQLITEPLUS_SQLITE3_QUERY_HPP

#include <iostream>
#include <vector>

class SQLITE3_QUERY {
public:
    /**
     * constructor
     * @param query_template
     */
    explicit SQLITE3_QUERY(std::string query_template = "") {
        this->query_template = std::move(query_template);
    }

    /**
     * Copy Constructor
     * @param rhs
     */
    SQLITE3_QUERY(const SQLITE3_QUERY &rhs) {
        query_template = rhs.query_template;
        binding = rhs.binding;
        bound_query = rhs.bound_query;
    }

    /**
     * Copy Assignment
     * @param rhs
     * @return SQLITE3_QUERY
     */
    SQLITE3_QUERY &operator=(const SQLITE3_QUERY &rhs) {
        if (this == &rhs) { // self assignment guard
            return *this;
        }
        // copy the values
        query_template = rhs.query_template;
        binding = rhs.binding;
        bound_query = rhs.bound_query;
        return *this;
    }

    /**
     * Destructor
     */
    ~SQLITE3_QUERY() = default;

    /**
     * \private
     */
    void add_binding() {

    }

    /**
     * Add a new string to the binding vector
     * @param str
     */
    void add_binding(const std::string &str) {
        binding.push_back(str);
    }

    /**
     * Add a new string to the binding vector
     * @param str
     */
    void add_binding(const char * str) {
        binding.emplace_back(str);
    }

    /**
     * Add a new string to the binding vector
     * @param str
     */
    void add_binding(char * str) {
        binding.emplace_back(str);
    }

    /**
     * Add all new string to the binding vector
     * @tparam STRING string
     * @tparam STRINGS string pack
     * @param b binding
     * @param bs binding pack
     */
    template<typename STRING, typename ... STRINGS>
    void add_binding(STRING b, STRINGS ... bs) {
        // check if arguments provided is string type
        if (typeid(b) == typeid(std::string) || typeid(b) == typeid(const char *) || typeid(b) == typeid(char *)) {
            binding.push_back(b);
            add_binding(bs...);
        }
    }

    /**
     * Replace all ? in query_template with corresponding strings in binding
     * @return constructed query
     * @throw std::out_of_range
     * @return SQLITE3_QUERY
     */
    SQLITE3_QUERY &bind() {
        int index = 0;
        bound_query.clear();
        for (char &c : query_template) {
            if (c == '?') {
                if (index == (int) binding.size()) {
                    throw std::out_of_range("query_template have more argument than binding provided");
                }
                bound_query += "'" + binding.at(index) + "'";
                index += 1;
            } else {
                bound_query += c;
            }
        }
        return *this;
    }

    /**
     * Delete all elements from binding
     * @return SQLITE3_QUERY
     */
    SQLITE3_QUERY &reset_binding() {
        binding.clear();
        return *this;
    }

    /**
     * Clear query_template
     * @param q_template query template
     * @return SQLITE3_QUERY
     */
    SQLITE3_QUERY &set_query_template(std::string q_template) {
        query_template = std::move(q_template);
        return *this;
    }

    std::string query_template;
    std::string bound_query;
    std::vector<std::string> binding;
};


#endif //SQLITEPLUS_SQLITE3_QUERY_HPP
