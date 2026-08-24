cmake_minimum_required(VERSION 3.20)
project(MyClient LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)

add_executable(client src/main.cpp)
target_link_libraries(client PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
)