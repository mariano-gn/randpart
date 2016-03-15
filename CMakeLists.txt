cmake_minimum_required(VERSION 2.8)
set(TARGET_NAME randpart)
project(${TARGET_NAME})

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
   message(FATAL_ERROR "In-source builds are not allowed. Create a directory, and build within it.")
endif()

#Add simple-* stuff
set(SIMPLE_BASE "./3rdParty/simples")
include_directories(${SIMPLE_BASE})
include_directories(${SIMPLE_BASE}/patterns)
set(SIMPLE_SOURCES
    ${SIMPLE_BASE}/logger.cpp
)

#Add GLM-0.9.7.1
include_directories("./3rdParty/glm-0.9.7.1")

#Add GLFW3
set(GLFW3_BASE "./3rdParty/glfw")
add_subdirectory(${GLFW3_BASE})
include_directories(${GLFW3_BASE}/include)

#Add glLoadGen
include_directories("./3rdParty/glLoadGen")
set(GLLOADGEN_SOURCES
    "./3rdParty/glLoadGen/gl_core_3_3_noext_pcpp.cpp")

#Project files
include_directories("./src")
set(RANDPART_SOURCES
	"src/main.cpp"
	"src/window.cpp"
)

#Add up all sources
set(SOURCES
    ${SIMPLE_SOURCES}
    ${GLLOADGEN_SOURCES}
    ${RANDPART_SOURCES}
)

#Definitions
add_definitions(-D_LOG)
# Force C++11 on GLM
add_definitions(-DGLM_FORCE_CXX11)

add_executable(${TARGET_NAME} ${SOURCES})
target_link_libraries(${TARGET_NAME} glfw ${GLFW_LIBRARIES})

# c++11
if (${CMAKE_VERSION} VERSION_GREATER 3.1.0)
    set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
else()
    message(STATUS "Using an older version of CMAKE")
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # If on Apple, use libc++
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif()

# Set up warnings/errors and other needed flags.
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")

    # Per configuration flags.
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
else() # Assumes windows
    message(STATUS "Assuming MSVC compiler")
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
    # Also Warnings as errors.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
    # Specify unwind sematics (fix C4577 and related)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif()