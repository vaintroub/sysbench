if(WIN32)
  message(FATAL_ERROR "concurrency kit can't be built on Windows")
endif()

# Compile concurrency kit from source, not fancy flags
# As a matter of fact, we do not even need to link the library
# we only use functionality from headers only

include(ExternalProject)
ExternalProject_Add(buildConcurrencyKit
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/third_party/concurrency_kit/ck
  BINARY_DIR ${PROJECT_SOURCE_DIR}/third_party/concurrency_kit/ck
  CONFIGURE_COMMAND ./configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/ck-install
  BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/ck-install/lib/libck.a
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_ALWAYS FALSE)

add_library(ConcurrencyKit STATIC IMPORTED)
add_dependencies(ConcurrencyKit buildConcurrencyKit)
make_directory(${CMAKE_CURRENT_BINARY_DIR}/ck-install/include)
set_target_properties(ConcurrencyKit PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/ck-install/lib/libck.a
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/ck-install/include)

