cmake_minimum_required(VERSION 3.19)
project(FaceFynd VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenCV_DIR "D:/opencv-build/install/")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Gui Concurrent sql)
find_package(OpenCV REQUIRED)

qt_standard_project_setup()

qt_add_executable(FaceFynd
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(FaceFynd
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Gui
        Qt::Concurrent
        Qt::Sql
        ${OpenCV_LIBS}
)

include(GNUInstallDirs)

install(TARGETS FaceFynd
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET FaceFynd
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
