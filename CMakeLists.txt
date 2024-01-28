cmake_minimum_required(VERSION 3.10)

project(mruv VERSION 1.0)

add_executable(mruv src/main.c)
configure_file(config.rb config.rb COPYONLY)

add_library(picohttpparser STATIC "${PROJECT_SOURCE_DIR}/lib/picohttpparser/picohttpparser.c")
target_include_directories(picohttpparser INTERFACE "${PROJECT_SOURCE_DIR}/lib/picohttpparser")

add_library(mruby STATIC IMPORTED)
set_target_properties(mruby PROPERTIES IMPORTED_LOCATION /usr/local/lib/libmruby.a)
set_target_properties(mruby PROPERTIES INTERFACE_INCLUDE_DIRECTORIES /usr/local/include)

add_library(libuv STATIC IMPORTED)
set_target_properties(libuv PROPERTIES IMPORTED_LOCATION /usr/local/lib/libuv.a)
set_target_properties(libuv PROPERTIES INTERFACE_INCLUDE_DIRECTORIES /usr/local/include)

target_compile_options(mruv PUBLIC "-pthread")
target_link_libraries(mruv PRIVATE libuv mruby m dl pthread picohttpparser)
target_include_directories(mruv PRIVATE "${PROJECT_BINARY_DIR}")
