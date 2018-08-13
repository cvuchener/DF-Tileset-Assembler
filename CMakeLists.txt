cmake_minimum_required(VERSION 3.1)
project(Tileset-Assembler)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED Gui Widgets)

add_executable(Tileset-Assembler WIN32
	src/ConfigurationWidget.cpp
	src/ConfigurationWidget.h
	src/CP437.cpp
	src/CP437.h
	src/FileLineReader.cpp
	src/FileLineReader.h
	src/LayerComboBox.cpp
	src/LayerComboBox.h
	src/LogWindow.cpp
	src/LogWindow.h
	src/LogWindow.ui
	src/main.cpp
	src/MainWindow.cpp
	src/MainWindow.h
	src/MainWindow.ui
	src/Palette.cpp
	src/Palette.h
	src/ParseError.cpp
	src/ParseError.h
	src/PreviewWidget.cpp
	src/PreviewWidget.h
	src/TilemapInfo.cpp
	src/TilemapInfo.h
	src/Tileset.cpp
	src/Tileset.h
	src/TileSubset.cpp
	src/TileSubset.h
	resources.qrc
)
target_link_libraries(Tileset-Assembler Qt5::Widgets)

install(TARGETS Tileset-Assembler
	RUNTIME DESTINATION /)
