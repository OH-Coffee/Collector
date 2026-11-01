set(g_exe_name "Collector")


include_directories(
    ${CMAKE_SOURCE_DIR}/src/base/L1/
    ${CMAKE_SOURCE_DIR}/src/base/L1/jsoncpp/
    ${CMAKE_SOURCE_DIR}/src/base/L2/
    ${CMAKE_SOURCE_DIR}/src/base/L3/
    ${CMAKE_SOURCE_DIR}/src/base/L4/

    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/main/
    ${CMAKE_CURRENT_SOURCE_DIR}/plugins-api/
    ${CMAKE_CURRENT_SOURCE_DIR}/plugins-func/
    ${CMAKE_CURRENT_SOURCE_DIR}/plugins-data/

    ${CMAKE_SOURCE_DIR}/src/third/libbpf/src

)

file(GLOB_RECURSE g_sources CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/base/L1/*.cpp
    ${CMAKE_SOURCE_DIR}/src/base/L2/*.cpp
    ${CMAKE_SOURCE_DIR}/src/base/L3/*.cpp
    ${CMAKE_SOURCE_DIR}/src/base/L4/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

foreach (regex IN LISTS ITEMS
    # ".*plugins.*"
)
    list(FILTER g_source EXCLUDE REGEX ${regex})
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

add_custom_command(TARGET ${g_exe_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/config"
        "${CMAKE_BINARY_DIR}/collector/config"
    VERBATIM
)