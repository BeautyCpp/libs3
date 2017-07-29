################################################################################
# extlib/libs3/CMakeLists.txt
#
# Build libS3 library
################################################################################


cmake_minimum_required(VERSION 3.0)

include("cmake/Hunter/HunterGate.cmake")
HunterGate(
        URL "https://github.com/ruslo/hunter/archive/v0.16.24.tar.gz"
        SHA1 "00ab627d3d4fe5151e15b6ab2b1b1da17650984a"
)

set(CMAKE_CXX_STANDARD 14)

project(libs3)


add_definitions(-DLIBS3_VER_MAJOR="2")
add_definitions(-DLIBS3_VER_MINOR="0")
add_definitions(-DLIBS3_VER="2.0")

# find packages
hunter_add_package(CURL)
find_package(CURL REQUIRED)
include_directories(SYSTEM ${CURL_INCLUDE_DIRS})

hunter_add_package(OpenSSL)
find_package(OpenSSL REQUIRED)
include_directories(SYSTEM ${OPENSSL_INCLUDE_DIR})

hunter_add_package(libxml2)
find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})

if(MINGW)
  add_definitions(-DLIBXML_STATIC)
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/inc/")

set(LIBS3_SOURCES
  #src/acl.c
  src/bucket.c
  src/bucket_metadata.c
  src/error_parser.c
  src/general.c
  src/multipart.c
  src/object.c
  src/request.c
  src/request_context.c
  src/response_headers_handler.c
  src/s3.c
  src/service.c
  src/service_access_logging.c
  src/simplexml.c
  src/util.c
  # src/mingw_functions.c
  # src/mingw_s3_functions.c
  )

set(LIBS3_HDRS
  inc/error_parser.h
  inc/libs3.h
  inc/request.h
  inc/request_context.h
  inc/response_headers_handler.h
  inc/simplexml.h
  inc/string_buffer.h
  inc/util.h
)


add_library(s3 STATIC ${LIBS3_SOURCES})
target_link_libraries(s3
  ${LIBXML2_LIBRARIES} ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES})

####
# Installation (https://github.com/forexample/package-example)

# Layout. This works for all platforms:
#   * <prefix/lib/cmake/<PROJECT-NAME>
#   * <prefix>/lib/
#   * <prefix>/include/
set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(include_install_dir "include")
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Configuration
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

# Include module with fuction 'write_basic_package_version_file'
include(CMakePackageConfigHelpers)

# Configure '<PROJECT-NAME>ConfigVersion.cmake'
# Note: PROJECT_VERSION is used as a VERSION
# write_basic_package_version_file(
#     "${version_config}" COMPATIBILITY SameMajorVersion
# )

# Configure '<PROJECT-NAME>Config.cmake'
# Use variables:
#   * targets_export_name
#   * PROJECT_NAME
configure_package_config_file(
    "cmake/Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)

# Install the library and produce a CMake export script
install(TARGETS s3
	EXPORT "${targets_export_name}"
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	INCLUDES DESTINATION include
	FRAMEWORK DESTINATION Frameworks
)

if (APPLE AND BUILD_FRAMEWORK)
#	set_target_properties(s3 PROPERTIES OUTPUT_NAME libs3 FRAMEWORK ON)
#	set_source_files_properties(${ASYNCXX_INCLUDE} PROPERTIES MACOSX_PACKAGE_LOCATION Headers/async++)
#	set_source_files_properties(${PROJECT_SOURCE_DIR}/include/async++.h PROPERTIES MACOSX_PACKAGE_LOCATION Headers)
else()
	set_target_properties(s3 PROPERTIES OUTPUT_NAME s3)
	target_include_directories(s3 INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include>)
	install(FILES ${LIBS3_HDRS} DESTINATION include)
endif()


# Config
#   * <prefix>/lib/cmake/Async++/Async++Config.cmake
#   * <prefix>/lib/cmake/Async++/Async++ConfigVersion.cmake
install(
    FILES "${project_config}"
    DESTINATION "${config_install_dir}"
)

# Config
#   * <prefix>/cmake/Async++/FooTargets.cmake
install(
    EXPORT "${targets_export_name}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "none") #required

INCLUDE(CPack)

