cmake_minimum_required(VERSION 3.5)
project(flush+reload)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory (libs)
add_compile_options(-mprefetchwt1)

add_executable (sender sender.c)
#add_executable (sender_pre sender_pre.c)
find_package(Threads)
target_link_libraries (sender LINK_PUBLIC  ${CMAKE_THREAD_LIBS_INIT} util)
#target_link_libraries (sender_pre LINK_PUBLIC  ${CMAKE_THREAD_LIBS_INIT} util)

#add_executable (receiver_pre receiver_pre.c)
#add_executable (receiver_load receiver_load.c)
add_executable (receiver_pre_pre receiver_pre_pre.c)

#target_link_libraries (receiver_pre LINK_PUBLIC  ${CMAKE_THREAD_LIBS_INIT} util)
#target_link_libraries (receiver_load LINK_PUBLIC  ${CMAKE_THREAD_LIBS_INIT} util)
target_link_libraries (receiver_pre_pre LINK_PUBLIC  ${CMAKE_THREAD_LIBS_INIT} util)

set(CMAKE_CXX_FLAGS "-pthread")
