cmake_minimum_required(VERSION 3.16)

project(Clone VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========================================
# 👇 Manually add QScintilla include/lib
# ========================================
include_directories("C:/Qt/6.9.1/mingw_64/include")
link_directories("C:/Qt/6.9.1/mingw_64/lib")

# ========================================
# Qt Required Packages
# ========================================
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# ========================================
# Source Files
# ========================================
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

# qt5_add_resources(RESOURCES resources.qrc)
# target_sources(your_target_name PRIVATE ${RESOURCES})

# ========================================
# Executable
# ========================================
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Clone
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        CustomFileSystemModel.h
        resources.qrc
        CustomFileSystemModel.cpp

    )
else()
    if(ANDROID)
        add_library(Clone SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(Clone
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# ========================================
# 🔗 Link QScintilla
# ========================================
target_link_libraries(Clone
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    qscintilla2_qt6
)

# ========================================
# App Properties
# ========================================
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Clone)
endif()

set_target_properties(Clone PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# ========================================
# Install Settings
# ========================================
include(GNUInstallDirs)
install(TARGETS Clone
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Clone)
endif()
