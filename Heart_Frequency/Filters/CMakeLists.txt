cmake_minimum_required(VERSION 3.0.0)
project(HeartRatePPG VERSION 1.0.1 LANGUAGES CXX)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include_directories(eigen3 eigen3/unsupported eigen3/EigenRand)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
file(GLOB_RECURSE _HDRS "*.hpp")
file(GLOB_RECURSE _SRCS "*.cpp")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(HeartRatePPG SHARED ${_HDRS} ${_SRCS})
target_link_libraries(HeartRatePPG PRIVATE ${OpenCV_LIBS})
generate_export_header(HeartRatePPG)
target_include_directories(${PROJECT_NAME} PUBLIC 
							"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>" 
							"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>" )

							install(TARGETS HeartRatePPG
	EXPORT HeartRatePPGTargets
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	INCLUDES DESTINATION include)
install(FILES ${_HDRS} DESTINATION include)
install(FILES ${PROJECT_BINARY_DIR}/HeartRatePPG_export.h DESTINATION include)
install(EXPORT HeartRatePPGTargets
        FILE HeartRatePPGTargets.cmake
        DESTINATION lib/cmake/HeartRatePPG)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/HeartRatePPGConfig.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/HeartRatePPGConfig.cmake"
	INSTALL_DESTINATION lib/cmake/HeartRatePPG)
install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/HeartRatePPGConfig.cmake"
	DESTINATION lib/cmake/HeartRatePPG)
set(CPACK_GENERATOR NSIS)
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "CMake.org")
set(CPACK_PACKAGE_VERSION "1.0.1")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CPACK_GENERATOR "ZIP")
SET(CPACK_NSIS_MODIFY_PATH ON)
INCLUDE(CPack)