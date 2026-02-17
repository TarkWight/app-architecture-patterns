# cmake/CodeQuality.cmake

function(enable_code_quality TARGET_NAME)
  # Compiler warnings -> errors
  target_compile_options(${TARGET_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall -Wextra -Wpedantic -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  )

  # clang-tidy: if CLANG_TIDY_PATH set (presets/user presets)
  if (ENABLE_CLANG_TIDY AND CLANG_TIDY_PATH)
    set_target_properties(${TARGET_NAME} PROPERTIES
      CXX_CLANG_TIDY "${CLANG_TIDY_PATH}"
    )
  endif()

  # clang-format check: if CLANG_FORMAT_PATH set
  if (ENABLE_CLANG_FORMAT_CHECK AND CLANG_FORMAT_PATH)
    get_target_property(_srcs ${TARGET_NAME} SOURCES)

    set(_fmt_files "")
    foreach(f IN LISTS _srcs)
      if (NOT IS_ABSOLUTE "${f}")
        set(abs "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
      else()
        set(abs "${f}")
      endif()
      if (abs MATCHES ".*\\.(c|cc|cpp|cxx|h|hh|hpp|hxx)$")
        list(APPEND _fmt_files "${abs}")
      endif()
    endforeach()

    add_custom_target(${TARGET_NAME}-format-check
      COMMAND "${CLANG_FORMAT_PATH}" --dry-run --Werror ${_fmt_files}
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      COMMENT "Checking code style with clang-format"
      VERBATIM
    )
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}-format-check)
  endif()
endfunction()
