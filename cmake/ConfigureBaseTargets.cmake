add_library(lendy-compile-option-interface INTERFACE)

set(CXX_EXTENSIONS OFF)

add_library(lendy-feature-interface INTERFACE)

target_compile_features(lendy-feature-interface
  INTERFACE
    cxx_alias_templates
    cxx_auto_type
    cxx_constexpr
    cxx_decltype
 
    cxx_final
    cxx_lambdas
    
    cxx_variadic_templates
    cxx_defaulted_functions
    cxx_nullptr
    cxx_trailing_return_types

    #cxx_decltype_auto
    #cxx_generic_lambdas
    #cxx_return_type_deduction
    )


add_library(lendy-warning-interface INTERFACE)

add_library(lendy-default-interface INTERFACE)
target_link_libraries(lendy-default-interface
  INTERFACE
    lendy-compile-option-interface
    lendy-feature-interface)

add_library(lendy-no-warning-interface INTERFACE)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(lendy-no-warning-interface
    INTERFACE
      /W0)
else()
  target_compile_options(lendy-no-warning-interface
    INTERFACE
      -w)
  target_link_libraries(lendy-default-interface
    INTERFACE
	  dl
	  pthread)
endif()

add_library(lendy-hidden-symbols-interface INTERFACE)

add_library(lendy-dependency-interface INTERFACE)
target_link_libraries(lendy-dependency-interface
  INTERFACE
    lendy-default-interface
    lendy-no-warning-interface
    lendy-hidden-symbols-interface)

add_library(lendy-core-interface INTERFACE)
target_link_libraries(lendy-core-interface
  INTERFACE
    lendy-default-interface
    lendy-warning-interface)
