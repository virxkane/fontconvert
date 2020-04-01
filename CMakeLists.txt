project(fontconvert C)

cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_BINARY_DIR})

find_package(Freetype REQUIRED)

include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories(${CMAKE_BINARY_DIR})

set(SRC_LIST
	fontconvert.c
)

# add libgnugetopt sources
if(WIN32)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libgnugetopt-1.3)
	set(SRC_LIST ${SRC_LIST} libgnugetopt-1.3/getopt.c libgnugetopt-1.3/getopt1.c)
	add_definitions(-DHAVE_STRING_H=1)
endif(WIN32)

set(LDADD_LIBS)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${FREETYPE_LIBRARIES} ${LDADD_LIBS})
