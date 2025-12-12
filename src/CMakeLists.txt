file(GLOB_RECURSE HDR_FILES   CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE COMMON_SRCS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(FILTER COMMON_SRCS EXCLUDE REGEX ".*(win32|linux|darwin)/.*")

# I think this is only for Visual Studio?
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header"
  FILES ${HDR_FILES}
)

set(SRCFILES ${COMMON_SRCS})

if(WIN32)
  file(GLOB_RECURSE WIN32_SRCS CONFIGURE_DEPENDS "win32/*.cpp")
  list(APPEND SRCFILES ${WIN32_SRCS})
elseif(UNIX AND NOT APPLE)
  file(GLOB_RECURSE LINUX_SRCS CONFIGURE_DEPENDS "linux/*.cpp")
  list(APPEND SRCFILES ${LINUX_SRCS})
elseif(APPLE)
  file(GLOB_RECURSE DARWIN_SRCS CONFIGURE_DEPENDS "darwin/*.cpp")
  list(APPEND SRCFILES ${DARWIN_SRCS})
endif()

add_executable("${EXE_NAME}" ${SRCFILES})
set_compiler_flags("${EXE_NAME}")

set_target_properties("${EXE_NAME}" PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories("${EXE_NAME}" PUBLIC
  "${PROJECT_SOURCE_DIR}/include")

target_link_libraries(
  "${EXE_NAME}"
  ${THIRD_PARTY_LIB}
)
