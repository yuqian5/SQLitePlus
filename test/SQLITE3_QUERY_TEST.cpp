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

#include "SQLITE3_QUERY.hpp"

#include <cassert>

int main () {
    SQLITE3_QUERY query;

    // check initialization
    assert(query.query_template.empty());

    // check
    query.set_query_template("? ? ?");

    // check add_binding and bind
    char* s1 =  "abc";
    query.add_binding(s1);
    query.add_binding("def");
    query.add_binding(std::string("ghi"));
    assert(query.bind().bound_query == "'abc' 'def' 'ghi'");

    // check copy construct
    SQLITE3_QUERY query2 = query;
    assert(query.bind().bound_query == query2.bind().bound_query);

    // check copy assignment
    SQLITE3_QUERY query3 = query;
    assert(query.bind().bound_query == query3.bind().bound_query);

    // check reset binding, and pack binding
    query3.reset_binding();
    query3.add_binding("abc", "def", "ghi");
    assert(query.bind().bound_query == query3.bind().bound_query);
}