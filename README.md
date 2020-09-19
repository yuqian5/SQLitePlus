# SQLitePlus ![CMake](https://github.com/yuqian5/SQLitePlus/workflows/CMake/badge.svg?branch=master)

A modern headers only SQLite3 wrapper implemented in C++

### About
This headers only library aims to provide an simple and intuitive interface for the SQLite3 C library.
Many of the methods took inspirations from the Python SQLite3 library. 

### Install
1. Download directory lib
2. Include headers
    * CMake Project:
        * simply include the lib/include directory in your project
        *     INCLUDE_DIRECTORIES(./lib/include)
    * Others:
        * include SQLITE3.hpp and SQLITE3_QUERY.hpp in your project
        
### Dependency
SQLite3

### Documentation
See [Link](https://yuqian5.github.io/SQLitePlus/html/annotated.html)

### Demo
    cmake CMakeLists.txt
    make
    ./SQLitePlusDemo
    


    
