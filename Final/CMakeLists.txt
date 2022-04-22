cmake_minimum_required(VERSION 3.4)
include("${CMAKE_SOURCE_DIR}/FindGfxLibs.cmake")

project(Exp)

# Enforce use of C++14
set(CMAKE_CXX_STANDARD_REQUIRED 14)
set(CMAKE_CXX_STANDARD 14)

# Use glob to get the list of all source files.
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" 
			  "${CMAKE_SOURCE_DIR}/ext/*/*.cpp" 
			  "${CMAKE_SOURCE_DIR}/ext/glad/src/*.c" 
			  "${CMAKE_SOURCE_DIR}/ext/glad/src/*.c"
			  "${CMAKE_SOURCE_DIR}/ext/imgui/*.cpp"
			  "${CMAKE_SOURCE_DIR}/ext/imgui/backends/imgui_impl_glfw.cpp"
			  "${CMAKE_SOURCE_DIR}/ext/imgui/backends/imgui_impl_opengl3.cpp")

list(FILTER SOURCES EXCLUDE REGEX "${CMAKE_SOURCE_DIR}/ext/imgui/misc/.*")

# We don't really need to include header and resource files to build, but it's
# nice to have them show up in IDEs.
file(GLOB_RECURSE HEADERS "src/*.h" "ext/*/*.h" "ext/*/*/*.h" "ext/glad/*/*.h")
file(GLOB_RECURSE GLSL "shaders/*.glsl")

include_directories("ext" "ext/glad/include" "ext/imgui" "ext/imgui/backends")

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

findGLFW3(${CMAKE_PROJECT_NAME})
findGLM(${CMAKE_PROJECT_NAME})
findFreetype(${CMAKE_PROJECT_NAME})
findAssimp(${CMAKE_PROJECT_NAME})

# OS specific options and libraries
if(NOT WIN32)
  message(STATUS "Adding GCC style compiler flags")
  add_compile_options("-g" "-Wall" "-pedantic" "-Werror=return-type")

  # TODO: The following links may be uneeded.
  if(APPLE)
    # Add required frameworks for GLFW.
    target_link_libraries(${CMAKE_PROJECT_NAME} "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
  else()
    #Link the Linux OpenGL library
    target_link_libraries(${CMAKE_PROJECT_NAME} "GL" "dl")
  endif()

else()
  # Link OpenGL on Windows
  target_link_libraries(${CMAKE_PROJECT_NAME} opengl32.lib)

endif()
