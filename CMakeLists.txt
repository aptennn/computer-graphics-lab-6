cmake_minimum_required(VERSION 4.3)
project(computer_graphics6)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(computer_graphics6 main.cpp)
target_link_libraries(computer_graphics6
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

