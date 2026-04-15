cmake_minimum_required(VERSION 3.16)

project(Agentia VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CPP_SOURCES
    cpp/Agent.cpp
    cpp/NetWorkManager.cpp
    "cpp/ redisworker.cpp"
    cpp/redismessager.cpp
)
set(CPP_HEADERS
    cpp/Agent.h
    cpp/NetWorkManager.h
    "cpp/ redisworker.h"
    cpp/redismessager.h
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

set(PROJECT_SOURCES
        qt/main.cpp
        qt/mainwindow.cpp
        qt/mainwindow.h
        qt/mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Agentia
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        qt/datadisplay.h qt/datadisplay.cpp
        #qt/Resource.qrc
        qt/gridworld.h qt/gridworld.cpp
        ${CPP_SOURCES}
        ${CPP_HEADERS}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Agentia APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Agentia SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Agentia
            ${PROJECT_SOURCES}
        )
    endif()
endif()

set(HIREDIS_ROOT "D:/Qt/externalLibs/hiredis")
target_include_directories(Agentia PRIVATE
    ${HIREDIS_ROOT}
    qt cpp
)

target_include_directories(Agentia PRIVATE qt cpp)
target_link_libraries(Agentia PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Core
    ${HIREDIS_ROOT}/build/Release/hiredis.lib
    ws2_32
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Agentia)
endif()
set_target_properties(Agentia PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Agentia
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Agentia)
endif()

#hiredis要依赖
#if(WIN32)
#    target_link_libraries(${PROJECT_NAME} ws2_32)
#endif()

#外部文件，不参与Qt编译环境的构建
file(GLOB_RECURSE EXTRA_FILES CONFIGURE_DEPENDS cpp/* py/*)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "External" FILES ${EXTRA_FILES})