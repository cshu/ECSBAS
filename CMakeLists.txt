project(libecsbas)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

include_directories(. "$ENV{envvarpath_sdl}/include/" "$ENV{envvarpath_sqlite}/include/" $ENV{envvarpath_cpprs})
link_directories("$ENV{envvarpath_sdl}/lib/x64/" $ENV{envvarpath_sqlite})

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} SDL2 sqlite3)
if(WIN32)
	target_compile_options(${PROJECT_NAME} PUBLIC /wd4996 /wd4244 /wd4267)
endif(WIN32)

