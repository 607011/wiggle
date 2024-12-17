cmake_minimum_required(VERSION 3.12)
project(wiggle CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX)
  set(CMAKE_CXX_FLAGS_PROFILING
    "${CMAKE_CXX_FLAGS_DEBUG} -pg"
    CACHE STRING "Flags used by the C++ compiler during profiling builds."
    FORCE)
  set(CMAKE_CXX_FLAGS "-pthread -Wno-deprecated-declarations")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g3 -ggdb -DDEBUG -pedantic -Wall -Wextra -Wreorder -Wuninitialized -Wtype-limits -Wno-pragmas")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -Wno-deprecated -Wsign-compare -Wall")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -Wno-deprecated -DDEBUG -ggdb")
endif(UNIX)

if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-security")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -glldb")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -Wno-deprecated -DDEBUG -glldb")
endif()

add_executable(wiggle 
    wiggle/main.cpp
)

find_library(CORE_GRAPHICS_LIBRARY CoreGraphics REQUIRED)
find_library(APPLICATION_SERVICES_LIBRARY ApplicationServices REQUIRED)
find_library(IOKIT_LIBRARY IOKit REQUIRED)

option(WITH_FADER "Enable display fading" ON)
if(WITH_FADER)
    add_definitions(-DWITH_FADER)
endif()

target_link_libraries(wiggle
    ${CORE_GRAPHICS_LIBRARY}
    ${APPLICATION_SERVICES_LIBRARY}
    ${IOKIT_LIBRARY}
)
