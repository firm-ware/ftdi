cmake_minimum_required(VERSION 3.21)
project(firm_ftdi VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-std=gnu++20 ${CMAKE_C_FLAGS}")

set(FTDI_IMPLEMENTATION "libftdi" CACHE STRING "The FTDI implementation to use (libftdi or ftd2xx)")

include(vendor.cmake)
if (FTDI_IMPLEMENTATION STREQUAL "libftdi")
    include(vendor-libftdi.cmake)
    set(FTDI_INCLUDES ${libftdi_SOURCE_DIR}/src) # I don't understand libftdi's CMakeLists.txt
    set(FTDI_LIBRARIES ftdi1)
    set(TARGET_DEFINITION LIBFTDI)
elseif (FTDI_IMPLEMENTATION STREQUAL "ftd2xx")
    include(vendor-ftd2xx.cmake)
    set(FTDI_LIBRARIES ftd2xx)
    set(TARGET_DEFINITION FTD2XX)
else()
    message(FATAL_ERROR "Unknown FTDI implementation: ${FTDI_IMPLEMENTATION}")
endif()

add_library(firm_ftdi ${CMAKE_CURRENT_SOURCE_DIR}/src/uart.cpp)
target_include_directories(firm_ftdi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${FTDI_INCLUDES}
)
target_link_libraries(firm_ftdi
    firm_uart
    firm_timing
    firm_logging
    ${FTDI_LIBRARIES}
)
target_compile_definitions(firm_ftdi PUBLIC ${TARGET_DEFINITION})

add_executable(break-test ${CMAKE_CURRENT_SOURCE_DIR}/src/break-test.cpp)
target_include_directories(break-test PUBLIC
    ${FTDI_INCLUDES}
)
target_link_libraries(break-test ${FTDI_LIBRARIES})
target_compile_definitions(break-test PUBLIC ${TARGET_DEFINITION})
