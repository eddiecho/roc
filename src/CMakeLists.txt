file(GLOB_RECURSE HDRFILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SRCFILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header"
  FILES ${HDRFILES}
)

add_executable("${EXECUTABLE_NAME}" ${SRCFILES})
target_include_directories("${EXECUTABLE_NAME}" PUBLIC
  "${PROJECT_BINARY_DIR}/include"
  "${PROJECT_SOURCE_DIR}/include")

target_link_libraries(
  "${EXECUTABLE_NAME}"
  ${THIRD_PARTY_LIB}
)
