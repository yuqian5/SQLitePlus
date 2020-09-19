# SQLitePlus
A modern headers only SQLite3 wrapper implemented in C++

### About
This headers only library aims to provide an simple and intuitive interface for the C SQLite3 library.
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
See ./docs/html/index.html

### Demo
    cmake CMakeLists.txt
    make
    ./SQLitePlusDemo
    


    
