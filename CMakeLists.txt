cmake_minimum_required(VERSION 4.2)
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

# Егор: подключение модели многогранника и каркасной отрисовки к приложению.
add_executable(computer_graphics6
        main.cpp
        geometry.cpp geometry.h
        wireframe_renderer.cpp wireframe_renderer.h
        math4.cpp math4.h
        mainwindow.cpp mainwindow.h)
target_link_libraries(computer_graphics6
        Qt::Core
        Qt::Gui
        Qt::Widgets
)
