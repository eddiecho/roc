cmake_minimum_required(VERSION 3.12)

configure_file(
  "${PROJECT_SOURCE_DIR}/include/roc_config.h.in"
  "${PROJECT_BINARY_DIR}/include/roc_config.h"
)
