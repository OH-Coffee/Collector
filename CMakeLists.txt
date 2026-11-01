cmake_minimum_required(VERSION 3.22)
project("Collector")

# 变量设置
set(g_root_dir      ${CMAKE_CURRENT_SOURCE_DIR})
set(g_build_type    ${CMAKE_BUILD_TYPE})
set(g_arch_type     ${CMAKE_SYSTEM_PROCESSOR})

# 编译器
set(CMAKE_C_COMPILER            clang)
set(CMAKE_CXX_COMPILER          clang++)
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 编译选项
add_definitions(-DOS_LINUX)
add_compile_options(
    -g
    -D_FORTIFY_SOURCE=2
    -fstack-protector-all
)
add_link_options(
    -Wl,--no-undefined
    -Wl,--exclude-libs,ALL
    -Wl,-z,now
    -Wl,-z,relro
)

# 编译类型
if(${g_build_type} STREQUAL "debug")
    add_definitions(-D_DEBUG -DDEBUG)
    add_compile_options(-O0)
else()
    add_definitions(-D_RELEASE -DRELEASE)
    add_compile_options(-O2)
endif()
add_compile_options(
    -Wall
    -Wextra
    -Wno-unused-value
    -Wno-unused-variable
    -Wno-unused-function
    -Wno-unused-parameter
    -Wno-delete-non-virtual-dtor
    -Werror
)
message(STATUS ">> CMAKE_BUILD_TYPE = " ${g_build_type})

# 库文件搜索路径
link_directories(
    ${CMAKE_BINARY_DIR}/src/third/libbpf/src
)

# 头文件搜索路径
include_directories(
    ${g_root_dir}/src/base/
    ${g_root_dir}/src/base/header/
)

# 子项目
add_subdirectory(${g_root_dir}/src/proc/api-tester)
add_subdirectory(${g_root_dir}/src/proc/collector)
add_subdirectory(${g_root_dir}/src/proc/collector-bpf)
add_subdirectory(${g_root_dir}/src/proc/collector-bpf-loader)
