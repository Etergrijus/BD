cmake_minimum_required(VERSION 3.24)
project(Database_1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Database_1 main.cpp)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(Lab6 Lab6.cpp mainwindow.cpp mainwindow.h mainwindow.ui DataBaseHandler.cpp DataBaseHandler.h)

add_subdirectory(libpqxx-7.9.2 build-pqxx)

target_link_libraries(Lab6 Qt6::Core Qt6::Gui Qt6::Widgets pqxx)

target_link_libraries(Database_1 PRIVATE pqxx)


