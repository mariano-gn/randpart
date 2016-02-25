cmake_minimum_required(VERSION 2.8)
set(TARGET_NAME randpart)
project(${TARGET_NAME})

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
   message(FATAL_ERROR "In-source builds are not allowed. Create a directory, and build within it.")
endif()

#Add GLM-0.9.7.1
include_directories("./3rdParty/glm-0.9.7.1")

#Add GLFW3
set(GLFW3_BASE "./3rdParty/glfw")
add_subdirectory(${GLFW3_BASE})
include_directories(${GLFW3_BASE}/include)

#Add glLoadGen
set(GLLOADGEN_SOURCES 
    "./3rdParty/glLoadGen/gl_core_3_3_noext_pcpp.cpp")
include_directories("./3rdParty/glLoadGen")

#Project files
include_directories("./src")
set(SOURCES 
	"src/main.cpp"
)

add_executable(${TARGET_NAME} ${GLLOADGEN_SOURCES} ${SOURCES})
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
