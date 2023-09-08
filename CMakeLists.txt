cmake_minimum_required(VERSION 2.8)
PROJECT(signal_quality_evaluation)
FILE(GLOB SOURCE "${CMAKE_SOURCE_DIR}/src/*.cpp")
FILE(GLOB INC "${CMAKE_SOURCE_DIR}/inc/*.h")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/inc/")

#Use OpenCV
find_package(OpenCV REQUIRED)

# Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# Add OpenCV headers location to your include paths
include_directories(${OpenCV_INCLUDE_DIRS})       #将openCV头文件包含进工程
include_directories("E:/armadillo-12.6.3/include")
include_directories("E:/sigpack-1.2.7/sigpack")

# 建立筛选器，添加头文件和源文件
source_group(inc FILES ${INC})
source_group(src FILES ${SOURCE})
ADD_EXECUTABLE(signal_quality_evaluation ${SOURCE} ${INC})
TARGET_LINK_LIBRARIES(signal_quality_evaluation ${OpenCV_LIBS})
TARGET_LINK_LIBRARIES(signal_quality_evaluation 
                      optimized E:/OpenBLAS-develop/build/lib/RELEASE
					  debug E:/OpenBLAS-develop/build/lib/DEBUG)


