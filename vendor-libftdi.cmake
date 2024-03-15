message(STATUS "Fetching libftdi...")
FetchContent_Declare(
  libftdi
  GIT_REPOSITORY "https://github.com/sehrgutesoftware/libftdi"
  GIT_TAG "e8c49df"
)
FetchContent_MakeAvailable(libftdi)
