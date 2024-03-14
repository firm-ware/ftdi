cmake_minimum_required(VERSION 3.21)
project(firm_ftdi VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-std=gnu++20 ${CMAKE_C_FLAGS}")

include(vendor.cmake)

list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/src/uart.cpp)
add_library(firm_ftdi ${sources})
target_include_directories(firm_ftdi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libftdi/src # I don't understand libftdi's CMakeLists.txt
)
target_link_libraries(firm_ftdi
    firm_uart
    firm_timing
    firm_logging
    ftdi1
)
