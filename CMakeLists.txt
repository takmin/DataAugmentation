cmake_minimum_required(VERSION 3.5)
project(DataAugmentation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main.cpp
    DataAugmentation.cpp
    Util.cpp
    RandomRotation.cpp
)

find_package(OpenCV REQUIRED)

add_executable(DataAugmentation ${SOURCES})
target_link_libraries(DataAugmentation PRIVATE ${OpenCV_LIBS})
target_include_directories(DataAugmentation PRIVATE ${OpenCV_INCLUDE_DIRS})

