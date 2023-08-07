cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(3514 LANGUAGES C)
# Specify the minimum version you require.
find_package (mongoc-1.0 1.7 REQUIRED)

add_executable (main.out ../main.c)
target_link_libraries (main.out PRIVATE mongo::mongoc_shared)