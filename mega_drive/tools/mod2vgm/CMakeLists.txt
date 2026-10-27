cmake_minimum_required(VERSION 3.10)
project(mod2vgm CXX C)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mod2vgm
	main.cpp
	mod_parser.cpp
	vgm_writer.cpp
	fm_patch.cpp
	converter_engine.cpp
	replayer.cpp
	sample_analysis.cpp
)

add_executable(mod_render
	mod_render.cpp
	mod_parser.cpp
	replayer.cpp
	sample_analysis.cpp
)

add_executable(vgm_render
	vgm_render.cpp
	ym3438.c
)

add_executable(fit_voices
	fit_voices.cpp
	fm_patch.cpp
	mod_parser.cpp
	replayer.cpp
	sample_analysis.cpp
	ym3438.c
)
find_package(Threads REQUIRED)
target_link_libraries(fit_voices Threads::Threads)
