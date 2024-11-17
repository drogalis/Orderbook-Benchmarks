cmake_minimum_required(VERSION 3.25.0)

project(Coinbase_Orderbook_Benchmark LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(BOOST_VERSION 1.83.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)

include(FetchContent)
include(${PARENT_DIR}/cmake/CompilerWarnings.cmake)
include(${PARENT_DIR}/cmake/Sanitizers.cmake)

include_directories(${PARENT_DIR}/benchmarks/orderbooks)
include_directories(${PARENT_DIR}/submodules)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Boost ${BOOST_VERSION} REQUIRED program_options)
find_package(OpenSSL REQUIRED)

# Fast JSON Parsing, better than SIMDjson
FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG main
  GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(glaze)

# Main EXE
add_executable(${PROJECT_NAME} coinbase_validation_main.cpp)

set_project_warnings(${PROJECT_NAME} TRUE "X" "" "" "X")
enable_sanitizers(${PROJECT_NAME} TRUE TRUE TRUE FALSE FALSE)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto
                                              ${Boost_LIBRARIES} glaze::glaze)
