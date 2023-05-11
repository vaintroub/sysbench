if(WIN32)
 # new-ish cmake features, install(runtime_dependencies etc)
 cmake_minimum_required(VERSION 3.21)
else()
 cmake_minimum_required(VERSION 3.3)
endif()

# CMP0091 MSVC runtime libraries can be controlled via flag
# CMP0069 CMAKE_INTERPROCEDURAL_OPTIMIZATION works for more than
#         Intel compiler
# CMP0075 Include check macros honor CMAKE_REQUIRED_LIBRARIES
# CMP0114 Xcode "new build system" support
foreach(pol CMP0091 CMP0069 CMP0075 CMP0114)
 if(POLICY ${pol})
   cmake_policy(SET ${pol} NEW)
 endif()
endforeach()

# Add homebrew search paths on Mac
if(APPLE)
  foreach(pkg openssl mysql-client mariadb-connector-c libpq)
    set(pkgpath /usr/local/opt/${pkg})
    if(EXISTS ${pkgpath})
     list(APPEND CMAKE_PREFIX_PATH ${pkg})
    endif()
  endforeach()
endif()

project(sysbench VERSION "1.1.0" LANGUAGES C)
set(CMAKE_C_STANDARD 11)

add_subdirectory(src)
