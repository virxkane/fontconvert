project(fontconvert C)

cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_BINARY_DIR})

find_package(Freetype REQUIRED)

include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories(${CMAKE_BINARY_DIR})

set(SRC_LIST
	fontconvert.c
)

set(LDADD_LIBS)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${FREETYPE_LIBRARIES} ${LDADD_LIBS})

configure_file(mk_sample.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/mk_sample.sh)
