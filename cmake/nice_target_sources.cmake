function(nice_target_sources target_name src_loc)
    set(current_scope "")
    set(public_sources "")
    set(private_sources "")
    set(interface_sources "")

    foreach(entry ${ARGN})
        if(${entry} STREQUAL "PRIVATE" OR ${entry} STREQUAL "PUBLIC" OR ${entry} STREQUAL "INTERFACE")
            set(current_scope ${entry})
            continue()
        endif()

        set(full_name ${src_loc}/${entry})
        if("${current_scope}" STREQUAL "PRIVATE")
            list(APPEND private_sources ${full_name})
        elseif("${current_scope}" STREQUAL "PUBLIC")
            list(APPEND public_sources ${full_name})
        elseif("${current_scope}" STREQUAL "INTERFACE")
            list(APPEND interface_sources ${full_name})
        else()
            message(FATAL_ERROR "Unknown sources scope for target ${target_name}")
        endif()
    endforeach()

    if(NOT "${public_sources}" STREQUAL "")
        target_sources(${target_name} PUBLIC ${public_sources})
    endif()
    if(NOT "${private_sources}" STREQUAL "")
        target_sources(${target_name} PRIVATE ${private_sources})
    endif()
    if(NOT "${interface_sources}" STREQUAL "")
        target_sources(${target_name} INTERFACE ${interface_sources})
    endif()
endfunction()
