include(ExternalProject)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # message(STATUS "Fetching ftd2xx for darwin/arm64...")
  # ExternalProject_Add(ftd2xx
  #   URL "https://ftdichip.com/wp-content/uploads/2021/05/D2XX1.4.24.zip"
  #   URL_HASH "SHA256=f59d18c11ecf5dedf0fcbdef24f18823c122ff24189a8e204479f9c408af7704"
  # )
  add_subdirectory(${PROJECT_SOURCE_DIR}/ftd2xx)
else ()
  message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} currently not supported")
endif ()
