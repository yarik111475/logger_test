cmake_minimum_required(VERSION 3.5)

project(logger_test VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

#boost
if(WIN32)
    set(BOOST_ROOT "C:/boost_1_76_0")
    set(BOOST_INCLUDEDIR "C:/boost_1_76_0/boost")
    set(BOOST_LIBRARYDIR "C:/boost_1_76_0/stage/lib")
endif()

if(LINUX)
    set(BOOST_ROOT "/usr")
    set(BOOST_INCLUDEDIR "/usr/include")
    set(BOOST_LIBRARYDIR "/usr/lib")
endif()

set(Boost_DEBUG 1)
set(Boost_USE_STATIC_LIBS ON)

add_definitions(-DHAS_BOOST)
add_definitions(-DBOOST_NO_CXX11_SCOPED_ENUMS)

find_package(Boost 1.76.0 EXACT COMPONENTS
    chrono
    thread
    date_time
    system
    filesystem
    program_options
)

find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        gui/MainWidget.h
        gui/MainWidget.cpp
        #logger/Controller.h
        #logger/Controller.cpp
        logger/LogController.h
        logger/LogController.cpp
        settings/Settings.h
        settings/Settings.cpp
        #settings/AppSettings.h
        #settings/AppSettings.cpp
)

add_executable(logger_test
    ${PROJECT_SOURCES}
)

target_include_directories(logger_test PRIVATE
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(logger_test PRIVATE
    Qt5::Core
    Qt5::Widgets
    ${Boost_LIBRARIES}
)


install(TARGETS logger_test
    RUNTIME DESTINATION ${CMAKE_BINARY_DIR}
)


