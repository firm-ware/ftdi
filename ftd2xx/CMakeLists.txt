add_library(ftd2xx SHARED IMPORTED GLOBAL)
target_include_directories(ftd2xx INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET ftd2xx PROPERTY
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/build/libftd2xx.a")

find_library(CF CoreFoundation)
find_library(IOK IOKit)
target_link_libraries(ftd2xx INTERFACE ${CF} ${IOK})
