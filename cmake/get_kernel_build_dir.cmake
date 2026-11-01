
function(get_kernel_build_dir o_dir)
    execute_process(
        COMMAND uname -r
        OUTPUT_VARIABLE v_kernel_version
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE v_uname_output
    )
    if(NOT v_uname_output EQUAL 0)
        message(FATAL_ERROR "Unable to determine the kernel version with uname -r")
    endif()

    set(${o_dir} "/lib/modules/${v_kernel_version}/build" PARENT_SCOPE)
endfunction()
