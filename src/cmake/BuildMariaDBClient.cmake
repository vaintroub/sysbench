# Build libmariadb from source
# On *Nix, this is done when mariadb/mysql client system library
# is missing, and USE_MYSQL is not set to OFF
#
# On Windows, we build it by default, "thanks" to lousy quality of vcpkg
# libs
include(ExternalProject)
if(WIN32)
  set(mariadbclient_LIBRARY_NAME libmariadb.dll)
else()
  set(mariadbclient_LIBRARY_NAME libmariadbclient.a)
endif()

set(mariadbclient_IMPORTED_LOCATION
  ${CMAKE_CURRENT_BINARY_DIR}/libmariadb-install/lib/mariadb/${mariadbclient_LIBRARY_NAME})
if(WIN32)
  set(STATIC_PVIO_NPIPE -DCLIENT_PLUGIN_PVIO_NPIPE=STATIC)
  set(mariadbclient_IMPORTED_IMPLIB
    ${CMAKE_CURRENT_BINARY_DIR}/libmariadb-install/lib/mariadb/libmariadb.lib)
endif()
include(ProcessorCount)
ProcessorCount(N)
set(parallel_build_flags)
if(NOT N EQUAL 0)
  set(parallel_build_flags -j${N})
endif()
ExternalProject_Add(
  buildLibmariadb
  GIT_REPOSITORY https://github.com/mariadb-corporation/mariadb-connector-c
  GIT_TAG v3.3.4
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libmariadb-src
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/libmariadb-build
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  BUILD_ALWAYS FALSE
  CMAKE_ARGS
  -G${CMAKE_GENERATOR}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
  ${STATIC_PVIO_NPIPE}
  -DCLIENT_PLUGIN_CACHING_SHA2_PASSWORD=STATIC
  -DSKIP_TESTS=1
  -DCMAKE_DISABLE_FIND_PACKAGE_CURL=1
  -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libmariadb-install
  -Wno-dev
  BUILD_BYPRODUCTS ${mariadbclient_IMPORTED_LOCATION}
)
make_directory(${CMAKE_CURRENT_BINARY_DIR}/libmariadb-install/include/mariadb)
if(WIN32)
  add_library(mariadbclient SHARED IMPORTED GLOBAL)
  set_target_properties(mariadbclient PROPERTIES
    IMPORTED_LOCATION ${mariadbclient_IMPORTED_LOCATION}
    IMPORTED_IMPLIB ${mariadbclient_IMPORTED_IMPLIB}
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/libmariadb-install/include/mariadb
    INTERFACE_LINK_LIBRARIES "${mariadbclient_INTERFACE_LINK_LIBRARIES}")
else()
  find_package(OpenSSL)
  if (NOT OpenSSL_FOUND)
    message(FATAL_ERROR "Could not find OpenSSL, which is required to build libmariadb from source")
  endif()
  add_library(mariadbclient STATIC IMPORTED GLOBAL)
  set(mariadbclient_INTERFACE_LINK_LIBRARIES "m;dl;OpenSSL::SSL")
  set_target_properties(mariadbclient PROPERTIES
  IMPORTED_LOCATION ${mariadbclient_IMPORTED_LOCATION}
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/libmariadb-install/include/mariadb
  INTERFACE_LINK_LIBRARIES "${mariadbclient_INTERFACE_LINK_LIBRARIES}")
endif()
add_dependencies(mariadbclient buildLibmariadb)
