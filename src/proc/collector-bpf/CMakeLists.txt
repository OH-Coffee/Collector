cmake_minimum_required(VERSION 3.22)

set(g_exe_name "BPF-Loader")

set(g_clang               "clang")
set(g_opt                 "opt")
set(g_llvm_dis            "llvm-dis")
set(g_llc                 "llc" )
find_program(g_clang_exe       NAMES ${g_clang}      REQUIRED)
find_program(g_opt_exe         NAMES ${g_opt}        REQUIRED)
find_program(g_llvm_dis_exe    NAMES ${g_llvm_dis}   REQUIRED)
find_program(g_llc_exe         NAMES ${g_llc}        REQUIRED)

message(STATUS "${g_exe_name}: ${g_clang_exe}")
message(STATUS "${g_exe_name}: ${g_opt_exe}")
message(STATUS "${g_exe_name}: ${g_llvm_dis_exe}")
message(STATUS "${g_exe_name}: ${g_llc_exe}")


set(g_output_dir          ${CMAKE_BINARY_DIR}/collector/bin/BPF)
set(g_intermediate_dir    ${g_output_dir}/intermediates)


include(${CMAKE_SOURCE_DIR}/cmake/get_arch.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/get_kernel_build_dir.cmake)

get_arch(g_arch)
get_kernel_build_dir(g_kernel_build_dir)

message(STATUS "${g_exe_name}: ${g_arch}")
message(STATUS "${g_exe_name}: ${g_kernel_build_dir}")

if(NOT IS_DIRECTORY "${g_kernel_build_dir}")
    message(FATAL_ERROR "No kernel build directory")
endif()


set(g_inclde_dirs
    ${g_kernel_build_dir}/arch/${g_arch}/include
    ${g_kernel_build_dir}/arch/${g_arch}/include/generated
    ${g_kernel_build_dir}/include
    ${CMAKE_SOURCE_DIR}/src/base/
    ${CMAKE_SOURCE_DIR}/src/third/libbpf/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/progs
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers

    ${g_kernel_build_dir}/arch/${g_arch}/include/uapi
    ${g_kernel_build_dir}/arch/${g_arch}/include/generated/uapi
    ${g_kernel_build_dir}/include/uapi
    ${g_kernel_build_dir}/include/generated/uapi
)
set(g_include_options)
foreach(g_include_dir IN LISTS g_inclde_dirs)
    list(APPEND g_include_options "-I${g_include_dir}")
endforeach()


set(g_compile_options
    -DBPF_PROGRAM_COMPILE
    -DOS_LINUX
    -DCC_USING_FENTRY
    -D__KERNEL__
    -D__BPF_TRACING__
    "-D__TARGET_ARCH_${BPF_ARCH}"
    -fno-asynchronous-unwind-tables
    -fno-stack-protector
    -O
    -g
    -emit-llvm
    -Xclang
    -disable-llvm-passes
    -Wno-unused-value
    -Wno-pointer-sign
    -Wno-compare-distinct-pointer-types
    -Wno-gnu-variable-sized-type-not-at-end
    -Wno-address-of-packed-member
    -Wno-tautological-compare
    -Wno-unknown-warning-option
    -Wno-int-to-void-pointer-cast
    -Wno-incompatible-library-redeclaration
    -Wno-unknown-attributes
    -include
    ${g_kernel_build_dir}/include/linux/kconfig.h
)

if(g_arch STREQUAL "x86")
    list(APPEND g_compile_options -DARCHITECTURE_X86)
elseif(g_arch STREQUAL "arm")
    list(APPEND g_compile_options -DARCHITECTURE_ARM)
endif()


execute_process(
    COMMAND ${g_clang_exe} -dM -E -x c /dev/null
    OUTPUT_VARIABLE g_clang_definitions
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE g_clang_version_result
)
if(NOT g_clang_version_result EQUAL 0)
    message(FATAL_ERROR "Failed to query the version of ${g_clang_exe}")
endif()
string(REGEX MATCH "#define __clang_major__ ([0-9]+)" _ "${g_clang_definitions}")
if(CMAKE_MATCH_1 AND CMAKE_MATCH_1 LESS 11)
    list(APPEND g_compile_options -include asm_goto_workaround.h)
endif()


file(GLOB_RECURSE g_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.bpf.c"
)
file(GLOB_RECURSE g_headers CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.inl"
)
if(NOT g_sources OR NOT g_headers)
    message(FATAL_ERROR "No *.bpf.c or header files")
endif()

file(MAKE_DIRECTORY ${g_output_dir})
file(MAKE_DIRECTORY ${g_intermediate_dir})

set(g_outputs)
set(g_intermediate_files)

foreach(g_source IN LISTS g_sources)
    file(RELATIVE_PATH g_relative_source ${CMAKE_CURRENT_SOURCE_DIR} ${g_source})

    get_filename_component(g_source_dir  ${g_relative_source} DIRECTORY)
    get_filename_component(g_source_name ${g_relative_source} NAME)

    string(REGEX REPLACE "\\.c$" ".o" g_object_name "${g_source_name}")
    
    set(g_output_subdir "${g_output_dir}/${g_source_dir}")
    file(MAKE_DIRECTORY "${g_output_subdir}")

    string(MAKE_C_IDENTIFIER "${g_relative_source}" g_source_id)

    # 获取文件的哈希值，确保每个源文件对应唯一的输出文件名
    string(SHA256 g_source_hash "${g_relative_source}")
    string(SUBSTRING "${g_source_hash}" 0 12 g_source_hash)
    string(APPEND g_source_id "_${g_source_hash}")

    set(g_bpf_raw_bitcode ${g_intermediate_dir}/${g_source_id}.bc)
    set(g_bpf_opt_bitcode ${g_intermediate_dir}/${g_source_id}.opt.bc)
    set(g_bpf_llvm_ir     ${g_intermediate_dir}/${g_source_id}.ll)
    set(g_bpf_output      ${g_output_subdir}/${g_object_name})

    list(APPEND g_intermediate_files
        "${g_bpf_raw_bitcode}"
        "${g_bpf_opt_bitcode}"
        "${g_bpf_llvm_ir}"
    )

    add_custom_command(
        OUTPUT "${g_bpf_output}"
        COMMAND "${g_clang_exe}"
            ${g_compile_options}
            ${g_include_options}
            -c "${g_source}" -o "${g_bpf_raw_bitcode}"
        COMMAND "${g_opt_exe}"
            -O2 -mtriple=bpf-pc-linux
            "${g_bpf_raw_bitcode}" -o "${g_bpf_opt_bitcode}"
        COMMAND "${g_llvm_dis_exe}"
            "${g_bpf_opt_bitcode}" -o "${g_bpf_llvm_ir}"
        COMMAND "${g_llc_exe}"
            -march=bpf -filetype=obj "${g_bpf_llvm_ir}" -o "${g_bpf_output}"
        DEPENDS "${g_sources}" ${g_headers}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Compiling BPF program ${g_relative_source}"
        VERBATIM
    )

    list(APPEND g_outputs "${g_bpf_output}")
endforeach()

add_custom_target(BPF ALL DEPENDS ${g_outputs})
add_custom_target(clean_bpf
    COMMAND "${CMAKE_COMMAND}" -E rm -f ${g_outputs}
    COMMENT "Removing generated BPF object files"
)
add_custom_target(clean_bpf_intermediates
    COMMAND "${CMAKE_COMMAND}" -E rm -f ${g_intermediate_files}
    COMMENT "Removing generated BPF intermediate files"
)
add_custom_target(info
    COMMAND "${CMAKE_COMMAND}" -E echo "Sources: ${g_sources}"
    COMMAND "${CMAKE_COMMAND}" -E echo "Outputs: ${g_outputs}"
    COMMAND "${CMAKE_COMMAND}" -E echo "Intermediates: ${g_intermediate_dir}"
    COMMAND "${CMAKE_COMMAND}" -E echo "Keep intermediates: ${g_keep_intermediates}"
    COMMAND "${CMAKE_COMMAND}" -E echo "Output directory: ${g_bpf_output_dir}"
    VERBATIM
)

message(STATUS "Arch: ${g_arch}")
message(STATUS "Kernel build directory: ${g_kernel_build_dir}")
message(STATUS "BPF Output: ${g_bpf_output_dir}")
