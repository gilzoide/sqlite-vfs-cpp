add_library(sqlite3 STATIC sqlite-amalgamation/sqlite3.c)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_SHARED_LIBRARY_PREFIX "")
include_directories(".." "sqlite-amalgamation")

add_library(logiovfs SHARED "logiovfs.cpp")
add_executable(logiovfs-sample "logiovfs-main.cpp")
target_link_libraries(logiovfs-sample sqlite3 logiovfs)