function(get_arch o_arch)
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE v_machine
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE v_uname_output
    )
    if(NOT v_uname_output EQUAL 0)
        message(FATAL_ERROR "Unable to determine the host architecture with uname -m")
    endif()

    if(v_machine MATCHES "^(i.86|x86_64)$")
        set(v_detected_arch x86)
    elseif(v_machine MATCHES "^(aarch64|arm64)$")
        set(v_detected_arch arm64)
    elseif(v_machine MATCHES "^arm")
        set(v_detected_arch arm)
    elseif(v_machine STREQUAL "s390x")
        set(v_detected_arch s390)
    elseif(v_machine MATCHES "^ppc")
        set(v_detected_arch powerpc)
    elseif(v_machine MATCHES "^mips")
        set(v_detected_arch mips)
    elseif(v_machine MATCHES "^sh[234]")
        set(v_detected_arch sh)
    elseif(v_machine STREQUAL "parisc64")
        set(v_detected_arch parisc)
    elseif(v_machine STREQUAL "sun4u" OR v_machine STREQUAL "sparc64")
        set(v_detected_arch sparc)
    else()
        set(v_detected_arch "${v_machine}")
    endif()
    
    set(${o_arch} "${v_detected_arch}" PARENT_SCOPE)
endfunction()