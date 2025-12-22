cmake_minimum_required(VERSION 3.10)

project(counters VERSION 1.0.3 LANGUAGES CXX)

option(COUNTERS_INSTALL "Enable install" ON)

# Default to Release for single-configuration generators when no build type
# was provided by the user.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()
option(COUNTERS_BUILD_TESTS "Build tests" OFF)

if(COUNTERS_INSTALL)
  include(GNUInstallDirs)
endif()

if(COUNTERS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

add_library(counters INTERFACE)



add_library(counters::counters ALIAS counters)
target_include_directories(
  counters
  INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


if(COUNTERS_INSTALL)
  include(CMakePackageConfigHelpers)

  set(COUNTERS_VERSION_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/module/CountersConfigVersion.cmake")
  set(COUNTERS_PROJECT_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/module/CountersConfig.cmake")
  set(COUNTERS_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_DATAROOTDIR}/cmake/Counters")

  if(${CMAKE_VERSION} VERSION_LESS "3.14")
    write_basic_package_version_file("${COUNTERS_VERSION_CONFIG}" VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
  else()
    write_basic_package_version_file("${COUNTERS_VERSION_CONFIG}" VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
  endif()
  configure_package_config_file("cmake/config.cmake.in"
                                "${COUNTERS_PROJECT_CONFIG}"
                                INSTALL_DESTINATION "${COUNTERS_CONFIG_INSTALL_DIR}")

  install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/counters" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
  install(FILES "${COUNTERS_PROJECT_CONFIG}" "${COUNTERS_VERSION_CONFIG}" DESTINATION "${COUNTERS_CONFIG_INSTALL_DIR}")
  install(EXPORT ${PROJECT_NAME}-targets NAMESPACE Counters:: DESTINATION "${COUNTERS_CONFIG_INSTALL_DIR}")

  install(TARGETS counters
          EXPORT ${PROJECT_NAME}-targets
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
endif()
