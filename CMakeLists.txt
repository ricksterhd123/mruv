cmake_minimum_required(VERSION 3.10)

project(mruv VERSION 1.0)

configure_file(src/includes/mruv_config.h.in src/includes/mruv_config.h)

add_executable(mruv src/main.c)
configure_file(config.rb config.rb COPYONLY)

add_library(mruby STATIC IMPORTED)
set_target_properties(mruby PROPERTIES IMPORTED_LOCATION /usr/local/lib/libmruby.a)
set_target_properties(mruby PROPERTIES INTERFACE_INCLUDE_DIRECTORIES /usr/local/include)

add_library(libuv STATIC IMPORTED)
set_target_properties(libuv PROPERTIES IMPORTED_LOCATION /usr/local/lib/libuv.a)
set_target_properties(libuv PROPERTIES INTERFACE_INCLUDE_DIRECTORIES /usr/local/include)

target_link_libraries(mruv PRIVATE libuv mruby m)
target_include_directories(mruv PRIVATE "${PROJECT_BINARY_DIR}")
