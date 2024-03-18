include(FetchContent)
message(STATUS "Fetching firm...")
FetchContent_Declare(
  firm
  GIT_REPOSITORY "https://github.com/firm-ware/firm"
  GIT_TAG "v4.2.1"
)
FetchContent_MakeAvailable(firm)

message(STATUS "Fetching libusb...")
FetchContent_Declare(
    LIBUSB
    GIT_REPOSITORY "https://github.com/libusb/libusb-cmake.git"
    GIT_TAG "main"
  )
FetchContent_MakeAvailable(LIBUSB)
set(LIBUSB_INCLUDE_DIR ${LIBUSB_SOURCE_DIR}/libusb/libusb)
set(LIBUSB_LIBRARIES "usb-1.0")

