cmake_minimum_required(VERSION 3.25.0)

project(Orderbook_Benchmarks LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)

include(${PARENT_DIR}/cmake/CompilerWarnings.cmake)
include(${PARENT_DIR}/cmake/Sanitizers.cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/orderbooks)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${PARENT_DIR}/submodules)

find_package(benchmark REQUIRED)

# Main EXE
add_executable(${PROJECT_NAME} google_benchmark_main.cpp)

set_project_warnings(${PROJECT_NAME} TRUE "X" "" "" "X")
enable_sanitizers(${PROJECT_NAME} TRUE TRUE TRUE FALSE FALSE)

target_link_libraries(${PROJECT_NAME} PRIVATE benchmark::benchmark)
