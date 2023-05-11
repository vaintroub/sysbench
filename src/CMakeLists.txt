function(set_allowed_string_values var_name values)
  set_property(CACHE ${var_name} PROPERTY STRINGS ${values})
  set(val "${${var_name}}")
  if (NOT ("${val}" IN_LIST values))
   message(FATAL_ERROR
   "Invalid value '${val}' for variable ${var_name}."
   "These values are accepted: ${values}")
  endif()
endfunction()

add_executable(sysbench
  db_driver.c
  db_driver.h
  sb_barrier.c
  sb_barrier.h
  sb_ck_pr.h
  sb_counter.c
  sb_counter.h
  sb_global.h
  sb_histogram.c
  sb_histogram.h
  sb_list.h
  sb_logger.c
  sb_logger.h
  sb_lua.c
  sb_lua.h
  sb_options.c
  sb_options.h
  sb_rand.c
  sb_rand.h
  sb_thread.c
  sb_thread.h
  sb_timer.c
  sb_timer.h
  sb_util.c
  sb_util.h
  sysbench.c
  sysbench.h
  tests/sb_cpu.h
  tests/cpu/sb_cpu.c
  tests/sb_fileio.h
  tests/fileio/sb_fileio.c
  tests/fileio/crc32.c
  tests/fileio/crc32.h
  tests/fileio/crc32tbl.h
  tests/sb_memory.h
  tests/memory/sb_memory.c
  tests/sb_mutex.h
  tests/mutex/sb_mutex.c
  tests/sb_threads.h
  tests/threads/sb_threads.c
  xoroshiro128plus.h
  config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)
set_source_files_properties(
  ${CMAKE_CURRENT_BINARY_DIR}/config.h PROPERTIES GENERATED 1)
if (WIN32)
  target_sources(sysbench PRIVATE win/pthread_data.c win/pthread.h win/win_ck.h)
  target_include_directories(sysbench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/win)
endif()

target_include_directories(sysbench PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(sysbench PRIVATE -DHAVE_CONFIG_H)
add_subdirectory(lua/internal)

# Export all symbols from sysbench executable
# (some of those symbols are used in ffi). Note, that exporting all symbols
# this way precludes LTO with MSVC(technicality, object file format for -GL compiled
# files is undocumented. Regardless, perhaps we can think about picking symbols to export
# rather than exporting everything blindly.
set_target_properties(sysbench PROPERTIES ENABLE_EXPORTS 1 WINDOWS_EXPORT_ALL_SYMBOLS 1)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
target_link_libraries(sysbench PRIVATE Threads::Threads)
set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
# System checks
include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckFunctionExists)
check_function_exists(clock_gettime HAVE_CLOCK_GETTIME)
check_include_file(alarm.h HAVE_ALARM)
check_function_exists(directio HAVE_DIRECTIO)
check_include_file(errno.h HAVE_ERRNO_H)
check_include_file(fcntl.h HAVE_FCNTL_H)
check_function_exists(fdatasync HAVE_FDATASYNC)
if(CMAKE_COMPILER_IS_GNUCC)
 set(HAVE_FUNC_ATTRIBUTE_FORMAT 1)
 set(HAVE_FUNC_ATTRIBUTE_UNUSED 1)
endif()
check_function_exists(isatty HAVE_ISATTY)
find_library(aio HAVE_LIBAIO)
check_include_file(libgen.h HAVE_LIBGEN_H)
check_include_file(limits.h HAVE_LIMITS_H)
check_include_file(math.h HAVE_MATH_H)
#check_symbol_exists(mysql.h MYSQL_OPT_SSL_MODE HAVE_MYSQL_OPT_SSL_MODE)
check_symbol_exists(SHM_HUGETLB sys/shm.h HAVE_LARGE_PAGES)
check_function_exists(posix_memalign HAVE_POSIX_MEMALIGN)
if(WIN32)
  set(HAVE_PTHREAD_CANCEL 1)
  set(HAVE_PTHREAD_H 1)
  set(HAVE_PTHREAD_YIELD 1)
else()
  check_include_file(pthread.h HAVE_PTHREAD_H)
  check_symbol_exists(pthread_cancel pthread.h HAVE_PTHREAD_CANCEL)
  check_symbol_exists(pthread_yield pthread.h HAVE_PTHREAD_YIELD)
endif()
check_include_file(sched.h HAVE_SCHED_H)
check_function_exists(setvbuf HAVE_SETVBUF)
check_include_file(signal.h HAVE_SIGNAL_H)
check_function_exists(strerror_r HAVE_STRERROR_R)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(sys/ipc.h HAVE_SYS_IPC_H)
check_include_file(sys/mman.h HAVE_SYS_MMAN_H)
check_include_file(sys/shm.h HAVE_SYS_SHM_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(thread.h HAVE_THREAD_H)
check_function_exists(thr_setconcurrency HAVE_THR_SETCONCURRENCY)
check_include_file(unistd.h HAVE_UNISTD_H)
check_function_exists(fdatasync HAVE_FDATASYNC)
check_function_exists(memalign HAVE_MEMALIGN)
set(CMAKE_REQUIRED_INCLUDES stdbool.h)
check_type_size(bool SIZEOF_BOOL  LANGUAGE C)
set(CMAKE_REQUIRED_INCLUDES)
check_type_size(size_t SIZEOF_SIZE_T LANGUAGE C)
check_function_exists(mmap HAVE_MMAP)
check_function_exists(valloc HAVE_VALLOC)
set(PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
set(PACKAGE ${CMAKE_PROJECT_NAME})
include(GNUInstallDirs)
set(DATADIR_REL ${CMAKE_INSTALL_DATADIR})
set(DATADIR ${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME} CACHE STRING "full path to pkgdatadir")
set(LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/src/cmake")

# Use LuaJit library, either as system library or build from bundled source
set(WITH_LUAJIT "auto" CACHE STRING
 "Which luajit library will be used. Options are: auto, system, bundled")
set_allowed_string_values(WITH_LUAJIT "auto;system;bundled")
if (NOT WITH_LUAJIT MATCHES "bundled")
  # Find system library
  find_package(LuaJit)
  if(NOT LuaJit_FOUND AND (WITH_LUAJIT MATCHES "system"))
    message(FATAL_ERROR "could not find system luajit library")
  endif()
  set(SB_WITH_LUAJIT "system")
endif()
if (NOT LuaJit_FOUND AND NOT TARGET luajit::libluajit)
  # Build from bundled source
  include(BuildLuaJit)
  set(SB_WITH_LUAJIT "bundled")
endif()
target_link_libraries(sysbench PRIVATE luajit::libluajit)

# MySQL driver : auto,system,libmariadb,OFF
# Default on *nix - lookup system mariadb/mysql client library,
#  and build mariadb from git, if not found
# Default on Windows - build mariadb from git
set(WITH_MYSQL "auto" CACHE STRING
"Which mysql library will be used. Options are: system,libmariadb,auto,OFF
If you chose libmariadb, it will be built from its repository source.")
set_allowed_string_values(WITH_MYSQL "auto;system;libmariadb;OFF")

if (WITH_MYSQL)
  target_compile_definitions(sysbench PRIVATE -DUSE_MYSQL)
  target_sources(sysbench PRIVATE drivers/mysql/drv_mysql.c)
endif()
# On Windows, system(i.e vcpkg provided) mysql and mariadb clients are of a poor quality.
# So default is to build libmariadb from source.
# Elsewhere, default is to use system provided MySQL or MariaDB
if (WITH_MYSQL MATCHES "system" OR (WITH_MYSQL MATCHES "auto" AND (NOT WIN32)))
  find_package(MySQL)
  if (WITH_MYSQL MATCHES "system" AND (NOT MySQL_FOUND))
    message(FATAL_ERROR "system MariaDB/MySQL client library was not found")
  endif()
  if(MySQL_FOUND)
    target_link_libraries(sysbench PRIVATE MySQL::MySQL)
  endif()
endif()
if(WITH_MYSQL AND NOT(TARGET  MySQL::MySQL))
  include(BuildMariaDBClient)
  target_link_libraries(sysbench PRIVATE mariadbclient)
endif()


# Postgres driver (auto, ON, OFF) Auto means : use if found.
set(WITH_PGSQL "auto" CACHE STRING "Whether to use postgres driver. Options are: auto,ON,OFF")
set_allowed_string_values(WITH_PGSQL "auto;ON;OFF")

if (WITH_PGSQL)
  # workaround CMake issue # 17223 (non-essential header lookup)
  set(PostgreSQL_TYPE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
  find_package(PostgreSQL)
  if (PostgreSQL_FOUND)
    target_sources(sysbench PRIVATE drivers/pgsql/drv_pgsql.c)
    target_compile_definitions(sysbench PRIVATE -DUSE_PGSQL)
    target_link_libraries(sysbench PRIVATE PostgreSQL::PostgreSQL)
  else()
    if(MATCHES "ON")
      message(FATAL_ERROR "Postgres client library was not found")
    endif()
  endif()
endif()

if(NOT WIN32)
# Concurrency kit options (auto, system, bundled). Auto means - use system one found, else bundled
set(WITH_CONCURRENCY_KIT "auto" CACHE STRING
"Which concurrency kit library will be used. Options are: auto,system,bundled")
set_allowed_string_values(WITH_CONCURRENCY_KIT "auto;system;bundled")

if(WITH_CONCURRENCY_KIT MATCHES "system|auto")
  find_package(ConcurrencyKit)
  if((WITH_CONCURRENCY_KIT MATCHES "system") AND (NOT ConcurrencyKit_FOUND))
    message(FATAL_ERROR "Can't find system concurrency kit library")
  endif()
endif()
if(NOT ConcurrencyKit_FOUND)
 include(BuildConcurrencyKit)
endif()
add_dependencies(sysbench ConcurrencyKit)
target_link_libraries(sysbench PRIVATE ConcurrencyKit)
endif()

if(WIN32)
 #pthread.h on Windows
 target_include_directories(sysbench SYSTEM BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/win)
 if(MSVC)
   # Switch nagging irrelevant warnings off.
   target_compile_definitions(sysbench PRIVATE
     -DWIN32_MEAN_AND_LEAN
     -DNOGDI
     -DNOMINMAX
     -D_CRT_NONSTDC_NO_WARNINGS
     -D_CRT_SECURE_NO_WARNINGS
   )
 endif()
 add_custom_command(TARGET sysbench
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:luajit::libluajit> $<TARGET_FILE_DIR:sysbench>
  COMMENT "Copy luajit dll"
)
endif()

# Find git hash, if built from git
include(githash)
GitHash(SB_GIT_SHA)
if(SB_GIT_SHA)
  set(SB_GIT_SHA "-${SB_GIT_SHA}")
endif()

configure_file(config.h.in config.h @ONLY)

# Installation related stuff
if(NOT DISABLE_INSTALL)
install (FILES
  lua/bulk_insert.lua
  lua/empty-test.lua
  lua/oltp_common.lua
  lua/oltp_delete.lua
  lua/oltp_insert.lua
  lua/oltp_point_select.lua
  lua/oltp_read_only.lua
  lua/oltp_read_write.lua
  lua/oltp_update_index.lua
  lua/oltp_update_non_index.lua
  lua/oltp_write_only.lua
  lua/prime-test.lua
  lua/select_random_points.lua
  lua/select_random_ranges.lua
  DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
)

if(NOT WIN32)
# Install tests
if(WITH_MYSQL)
 set(USE_MYSQL 1)
endif()
if(WITH_PGSQL)
 set(USE_MYSQL 1)
endif()

configure_file(
  ${PROJECT_SOURCE_DIR}/tests/include/config.sh.in
  ${PROJECT_BINARY_DIR}/tests/include/config.sh @ONLY)
install(FILES
  ${PROJECT_BINARY_DIR}/tests/include/config.sh
  DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/tests/include
)

install(DIRECTORY
  ${PROJECT_SOURCE_DIR}/tests
  DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING
  PATTERN "*.t"
  PATTERN "*.sh"
  PATTERN "*.md"
  PATTERN "*.lua"
)
if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
  # make test is broken on out-of-source
  add_custom_target(test
    COMMAND sh -c ./test_run.sh
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests)
endif()
endif()

if(WIN32)
  # for timerGetPeriod
  target_link_libraries(sysbench PRIVATE winmm)

  if(TARGET MySQL::MySQL)
    set(mysqlclientlib  MySQL::MySQL)
  elseif(TARGET mariadbclient)
    set(mysqlclientlib mariadbclient)
  endif()
  # Install .dll dependencies next to the executable
  install(TARGETS
    sysbench
    EXPORT sysbenchTargets
    RUNTIME_DEPENDENCIES
    PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-" # magic to exclude MS .dlls
    POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
    DIRECTORIES
    $<TARGET_FILE_DIR:sysbench>
    $<TARGET_FILE_DIR:luajit::libluajit>
    $<$<BOOL:${mysqlclientlib}>:$<TARGET_FILE_DIR:${mysqlclientlib}>>
    ${PostgreSQL_LIBRARY_DIRS}
    $<$<BOOL:${PostgreSQL_LIBRARY_DIR}>:${PostgreSQL_LIBRARY_DIR}/../bin>
  )
else()
  install(TARGETS sysbench
    EXPORT sysbenchTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

install(EXPORT sysbenchTargets
  FILE sysbenchTargets.cmake
  NAMESPACE sysbench::
  DESTINATION share/sysbench/cmake
)

include(CMakePackageConfigHelpers)
# generate the config file that includes the exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/sysbenchConfig.cmake"
  INSTALL_DESTINATION "share/sysbench/cmake"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/sysbenchConfigVersion.cmake"
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/sysbenchConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/sysbenchConfigVersion.cmake
  DESTINATION share/sysbench/cmake
)
if (WIN32)
  # A more reasonable choice of default package on Windows
  # than NSIS
  set(CPACK_GENERATOR ZIP)
endif()
include(CPack)
endif()


