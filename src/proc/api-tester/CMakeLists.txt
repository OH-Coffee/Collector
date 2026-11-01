set(g_exe_name "API-Tester")


include_directories(
    ${g_root_dir}/src/base/L1/
    ${g_root_dir}/src/base/L1/jsoncpp/
    ${g_root_dir}/src/base/L2/
    ${g_root_dir}/src/base/L3/
    ${g_root_dir}/src/base/L4/

    ${g_root_dir}/src/proc/api-tester/
    ${g_root_dir}/src/third/googletest/googletest/
    ${g_root_dir}/src/third/googletest/googletest/include/

    ${CMAKE_SOURCE_DIR}/src/third/libbpf/src
)

file(GLOB_RECURSE g_sources CONFIGURE_DEPENDS
    ${g_root_dir}/src/base/L1/*.cpp
    ${g_root_dir}/src/base/L2/*.cpp
    ${g_root_dir}/src/base/L3/*.cpp
    ${g_root_dir}/src/base/L4/*.cpp

    ${g_root_dir}/src/proc/api-tester/*.cpp
    ${g_root_dir}/src/third/googletest/googletest/src/*.cc
)

foreach (regex IN LISTS ITEMS
    "gtest-all.cc$"
    "gtest_main.cc$"
)
    list(FILTER g_sources EXCLUDE REGEX ${regex})
endforeach ()

link_directories(
    ${CMAKE_SOURCE_DIR}/src/third/libbpf/src
)

# 生成目标文件
add_executable(${g_exe_name} ${g_sources})
target_link_libraries(${g_exe_name} pthread stdc++fs dl libbpf.a elf z rt)
target_compile_options(${g_exe_name}
    PRIVATE -Wno-sign-compare
    PRIVATE -Wno-gnu-variable-sized-type-not-at-end
    PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-x;c++>"
)
set_target_properties(${g_exe_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/collector/bin"
)