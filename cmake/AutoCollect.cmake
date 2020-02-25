function(CollectSourceFiles current_dir variable)
    list(FIND ARGN "${current_dir}" IS_EXCLUDED)
    if(IS_EXCLUDED EQUAL -1)
        file(GLOB COLLECTED_SOURCES
            ${current_dir}/*.c
            ${current_dir}/*.cc
            ${current_dir}/*.cpp
            ${current_dir}/*.inl
            ${current_dir}/*.def
            ${current_dir}/*.h
            ${current_dir}/*.hh
            ${current_dir}/*.hpp)
        list(APPEND ${variable} ${COLLECTED_SOURCES})

        file(GLOB SUB_DIRECTORIES ${current_dir}/*)
        foreach(SUB_DIRECTORY ${SUB_DIRECTORIES})
            if (IS_DIRECTORY ${SUB_DIRECTORY})
                CollectSourceFiles("${SUB_DIRECTORY}" "${variable}" "${ARGN}")
            endif()
        endforeach()
        set(${variable} ${${variable}} PARENT_SCOPE)
    endif()
endfunction()

function(CollectIncludeDirectories current_dir variable)
    list(FIND ARGN "${current_dir}" IS_EXCLUDED)
    if(IS_EXCLUDED EQUAL -1)
        list(APPEND ${variable} ${current_dir})
        file(GLOB SUB_DIRECTORIES ${current_dir}/*)
        foreach(SUB_DIRECTORY ${SUB_DIRECTORIES})
            if (IS_DIRECTORY ${SUB_DIRECTORY})
            CollectIncludeDirectories("${SUB_DIRECTORY}" "${variable}" "${ARGN}")
        endif()
        endforeach()
        set(${variable} ${${variable}} PARENT_SCOPE)
    endif()
endfunction()
