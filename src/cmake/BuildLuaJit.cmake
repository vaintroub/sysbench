# Build the bundled luajit library and create imported library.
# luajit::libluajit.
#
# This follows the instructions on the luajit page
# Both Unix and MSVC build are done in-source, with "make" rsp batch file
# On *Nix, luajit::libluajit is a static library,on Windows its a  DLL (this has to be so)
include(ExternalProject)
if(NOT WIN32)
  ExternalProject_Add(buildLuaJit
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/third_party/luajit/luajit
    BINARY_DIR ${PROJECT_SOURCE_DIR}/third_party/luajit/luajit
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libluajit-install
    INSTALL_COMMAND make install PREFIX=${CMAKE_CURRENT_BINARY_DIR}/libluajit-install
    BUILD_BYPRODUCTS  ${CMAKE_CURRENT_BINARY_DIR}/libluajit-install/lib/libluajit-5.1.a
    BUILD_ALWAYS FALSE
  )
  add_library(luajit::libluajit STATIC IMPORTED)
  add_dependencies(luajit::libluajit buildLuaJit)
  make_directory(${CMAKE_CURRENT_BINARY_DIR}/libluajit-install/include/luajit-2.1)
  set_target_properties(luajit::libluajit PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/libluajit-install/lib/libluajit-5.1.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/libluajit-install/include/luajit-2.1/
    INTERFACE_LINK_LIBRARIES "m;dl")
else()
  ExternalProject_Add(buildLuaJit
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/third_party/luajit/luajit/src
    BINARY_DIR ${PROJECT_SOURCE_DIR}/third_party/luajit/luajit/src
    CONFIGURE_COMMAND ""
    BUILD_COMMAND cmd.exe /c msvcbuild.bat debug # emit debug symbols
    INSTALL_COMMAND ""
    BUILD_ALWAYS FALSE
  )
  add_library(luajit::libluajit SHARED IMPORTED)
  add_dependencies(luajit::libluajit buildLuaJit)
  set_target_properties(luajit::libluajit PROPERTIES
    IMPORTED_IMPLIB ${PROJECT_SOURCE_DIR}/third_party/luajit/luajit/src/lua51.lib
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/third_party/luajit/luajit/src/lua51.dll
    INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/third_party/luajit/luajit/src)
endif()
