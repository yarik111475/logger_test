cmake_minimum_required(VERSION 3.5)

project(logger_test VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        gui/MainWidget.h
        gui/MainWidget.cpp
        logger/LogController.h
        logger/LogController.cpp
)

add_executable(logger_test
    ${PROJECT_SOURCES}
)

target_link_libraries(logger_test PRIVATE
    Qt5::Core
    Qt5::Widgets
)


install(TARGETS logger_test
    RUNTIME DESTINATION ${CMAKE_BINARY_DIR}
)


