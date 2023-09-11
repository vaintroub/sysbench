target_sources(sysbench PRIVATE
 ${CMAKE_CURRENT_SOURCE_DIR}/sb_win_posix.h
 ${CMAKE_CURRENT_SOURCE_DIR}/sb_win_posix.c)
option(WITH_SEGMENT_HEAP "Use Windows segment heap, for better performance" ON)
if(WITH_SEGMENT_HEAP)
  target_sources(sysbench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/segment_heap.manifest)
endif()
target_include_directories(sysbench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
