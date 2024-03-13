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

message(STATUS "Fetching libftdi...")
FetchContent_Declare(
  libftdi
  GIT_REPOSITORY "git://developer.intra2net.com/libftdi"
  GIT_TAG "de9f01ec"
)
FetchContent_MakeAvailable(libftdi)
