cmake_minimum_required(VERSION 3.0.0)
project(shader_story VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CTest)
enable_testing()

add_subdirectory(third_party)
add_subdirectory(engine)

add_executable(shader_story main.cpp)
target_link_libraries(shader_story PUBLIC Engine)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})


include(CPack)
