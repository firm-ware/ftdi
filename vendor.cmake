include(FetchContent)
message(STATUS "Fetching firm...")
FetchContent_Declare(
  firm
  GIT_REPOSITORY "git@github.com:firm-ware/firm.git"
  GIT_TAG "v4.1.0"
)
FetchContent_MakeAvailable(firm)

find_package(PkgConfig REQUIRED)
pkg_check_modules(usb REQUIRED IMPORTED_TARGET libusb)
