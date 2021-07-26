###############################################################################
# Policy adjustments
###############################################################################
cmake_minimum_required(VERSION 3.8.0)
cmake_policy(VERSION 3.8.0)
cmake_policy(SET CMP0087 NEW)
###############################################################################
# Options
###############################################################################
option(${ROOT_PROJECT_NAME}_BUILD_TOOLS "Toggle building of biogears command line tools" ON)
if(${ROOT_PROJECT_NAME}_BUILD_TOOLS)

###############################################################################
# Requirments
###############################################################################
if (${PREFIX}_BUILD_STATIC)
  add_definitions("-D${PREFIX}_BUILT_AS_STATIC")
  add_definitions("-DARACORE_BUILT_AS_STATIC")
endif()

###############################################################################
#Sorce and Header Defines
###############################################################################
message(STATUS "")
message(STATUS "Configuring ${PROJECT_NAME}")

#Scenario Driver
add_source_files(HEADERS LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/include REGEX "*.h"  SOURCE_GROUP "Headers\\Public\\")
add_source_files(SOURCES LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/ REGEX "*.cpp" SOURCE_GROUP "Sources\\")

##########################
#  Biogears Command Line #
##########################

add_executable(bg-mltest ${HEADERS} ${SOURCES} ${CONTROLLER_SOURCES})
set_target_properties(bg-mltest
  PROPERTIES
  FOLDER "Binaries"
  OUTPUT_NAME "bg-mltest"
  COMPILE_PDB_NAME "bg-mltest"
  PROJECT_LABEL "bg-mltest"
  DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
  CXX_STANDARD 14
  VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/runtime
  VS_DEBUGGER_ENVIRONMENT "${VS_DEBUGGER_ENVIRONMENT_LIST}"
  )
target_compile_definitions(bg-mltest PRIVATE DEBUG_POSTFIX=$<$<CONFIG:Debug>:"${CMAKE_DEBUG_POSTFIX}">)
target_compile_options(bg-mltest PRIVATE $<$<PLATFORM_ID:Windows>:/bigobj> PRIVATE $<$<PLATFORM_ID:Windows>:/MP${${ROOT_PROJECT_NAME}_BUILD_PARALLEL_LEVEL}>)
target_include_directories(bg-mltest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src/utils/ PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/)


target_link_libraries(bg-mltest 
      Biogears::libbiogears
      minizip::lib
)

if(TARGET Biogears::libbiogears_io)
  target_link_libraries(bg-mltest Biogears::libbiogears_io)
  target_compile_definitions(bg-mltest PRIVATE "BIOGEARS_IO_PRESENT")
endif()

install(TARGETS bg-mltest bg-scenario
     RUNTIME DESTINATION bin
     LIBRARY DESTINATION ${LIBRARY_INSTALL_DIR}
     ARCHIVE DESTINATION lib
)
set_property(INSTALL "$<TARGET_FILE_NAME:bg-mltest>"
  PROPERTY CPACK_START_MENU_SHORTCUTS "BioGears Commnad Line"
)

 
###
message(STATUS "")
endif()
