cmake_minimum_required(VERSION 3.10)
project(mod2vgm CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mod2vgm
	main.cpp
	mod_parser.cpp
	vgm_writer.cpp
	fm_patch.cpp
	converter_engine.cpp
)

add_executable(vgm_render
	vgm_render.cpp
	ym3438.c
)
