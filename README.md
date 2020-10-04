# SQLitePlus ![build](https://github.com/yuqian5/SQLitePlus/workflows/CMake/badge.svg?branch=master)

A modern C++ OO header only SQLite3 wrapper.

### About
This header only library aims to provide an simple and intuitive interface for the SQLite3 C library.
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
* [SQLite3](https://github.com/sqlite/sqlite)
* C++11

### Documentation
See [Doxygen](https://yuqian5.github.io/SQLitePlus/html/annotated.html)
    
### Contributing
1. Fork Repo
2. Make Changes and add new tests if needed
3. Submit pull request with detail description on changes and new features
4. Wait for approval
* Please keep in mind that this project aims to create a simple and intuitive interface for the SQLite3 C library, so please refrain from adding anything to the dependency tree.
    
### Feature Request
Submite a feature request under the issues tab.

### Demo
    cmake CMakeLists.txt
    make
    ./SQLitePlusDemo
    
### Tutorial
* [SQLITE3](./docs/tutorial/tutorial-SQLITE3.md)
* [SQLITE3_QUERY](./docs/tutorial/tutorial-SQLITE3_QUERY.md)

