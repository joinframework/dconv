set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "lib${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${DCONV_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${DCONV_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${DCONV_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${DCONV_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "Mathieu Rabine")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Header-only C++ library for printing and parsing floating point numbers")
set(CPACK_COMPONENT_DEV_DESCRIPTION
"dconv converts doubles to and from text, from a buffer or a stream.
This package contains the headers and CMake configuration.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/joinframework/dconv")

set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL dev)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

set(CPACK_DEBIAN_DEV_PACKAGE_NAME "lib${PROJECT_NAME}-dev")
set(CPACK_DEBIAN_DEV_PACKAGE_SECTION "libdevel")
set(CPACK_DEBIAN_DEV_PACKAGE_ARCHITECTURE "all")

include(CPack)
