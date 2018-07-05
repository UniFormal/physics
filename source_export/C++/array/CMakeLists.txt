

set(base_SOURCE
  ${base_SOURCE}
  ${CMAKE_CURRENT_SOURCE_DIR}/adjacency.cxx 
  ${CMAKE_CURRENT_SOURCE_DIR}/array.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/array_omp.cxx 
  ${CMAKE_CURRENT_SOURCE_DIR}/array_stack.cxx 
  ${CMAKE_CURRENT_SOURCE_DIR}/array_block.cxx 
  ${CMAKE_CURRENT_SOURCE_DIR}/block.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/vector.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/arrayxx.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/arrayprint.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/partitioning.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/arraytools.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/linalgdens.cxx
PARENT_SCOPE)



#install(FILES 
#	array0.h array.h
#	operator.h arrayprint.h
#	array_handler.h
#	partition.h vector.h DESTINATION ${build_dir} )


