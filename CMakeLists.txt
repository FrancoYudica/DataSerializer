cmake_minimum_required(VERSION 3.0)

project(DataSerializer)
add_executable(
    ${CMAKE_PROJECT_NAME}
    main.cpp
    data_file.hpp
    data_file.cpp
    )

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 17)
