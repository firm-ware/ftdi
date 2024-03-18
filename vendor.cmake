include(FetchContent)
message(STATUS "Fetching firm...")
FetchContent_Declare(
  firm
  GIT_REPOSITORY "https://github.com/firm-ware/firm"
  GIT_TAG "v4.2.1"
)
message(STATUS "Fetching libusb...")
FetchContent_Declare(
  libusb
  GIT_REPOSITORY "https://github.com/libusb/libusb-cmake.git"
  GIT_TAG "main"
)
FetchContent_MakeAvailable(firm libusb)

