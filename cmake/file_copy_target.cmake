function(file_copy_target target_name)
    set(one_value_args SRC_DIR DST_DIR MSG_NAME)
    set(multi_value_args GLOB)
    cmake_parse_arguments(ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    set(FILE_INPUT_LIST "")
    foreach(ITEM ${ARG_GLOB})
        file(GLOB_RECURSE SRC_LIST CONFIGURE_DEPENDS "${ARG_SRC_DIR}/${ITEM}")
        list(APPEND FILE_INPUT_LIST ${SRC_LIST})
    endforeach()
    
    set(FILE_OUTPUT_LIST "")
    foreach(ITEM ${FILE_INPUT_LIST})
        string(REPLACE "${ARG_SRC_DIR}" "${ARG_DST_DIR}" ITEM_TARGET ${ITEM})
        list(APPEND FILE_OUTPUT_LIST ${ITEM_TARGET})

        cmake_path(CONVERT ${ITEM} TO_NATIVE_PATH_LIST ITEM_INPUT)
        cmake_path(CONVERT ${ITEM_TARGET} TO_NATIVE_PATH_LIST ITEM_OUTPUT)

        add_custom_command(
            OUTPUT ${ITEM_TARGET}
            COMMAND copy ${ITEM_INPUT} ${ITEM_OUTPUT}
            DEPENDS ${ITEM}
            COMMENT "Copying ${ARG_MSG_NAME}: ${ITEM}"
            VERBATIM)
    endforeach()

    add_custom_target(${target_name}
        DEPENDS ${FILE_OUTPUT_LIST}
        COMMENT "Copy ${ARG_MSG_NAME}"
        VERBATIM)
endfunction()
