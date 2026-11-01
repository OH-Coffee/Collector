
set(g_exe_name "BPF-Loader")

include_directories(
    ${CMAKE_SOURCE_DIR}/src/base/header/event
    ${CMAKE_SOURCE_DIR}/src/base/L1/
    ${CMAKE_SOURCE_DIR}/src/base/L1/jsoncpp/
    ${CMAKE_SOURCE_DIR}/src/base/L2/
    ${CMAKE_SOURCE_DIR}/src/third/libbpf/src
)

file(GLOB_RECURSE g_sources CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/base/L1/*.cpp
    ${CMAKE_SOURCE_DIR}/src/base/L2/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

link_directories(
    ${CMAKE_SOURCE_DIR}/src/third/libbpf/src
)

add_executable(${g_exe_name} ${g_sources})

target_compile_definitions(${g_exe_name} PRIVATE OS_LINUX)
target_compile_options(${g_exe_name} PRIVATE
    -Wno-unused-value
    -g
    PRIVATE -Wno-sign-compare
    PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-x;c++>"
)
target_link_libraries(${g_exe_name} PRIVATE pthread stdc++fs libbpf.a elf z rt)

set_target_properties(${g_exe_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/collector/bin"
)
