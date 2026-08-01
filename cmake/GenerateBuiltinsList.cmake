function(v8_generate_builtins_list target-dir)
  set(directory ${target-dir}/builtins-generated)
  set(output ${directory}/bytecodes-builtins-list.h)
  # 在 configure 阶段直接创建目录，而非用 add_custom_command 以目录作为 OUTPUT
  # 后者会导致 MSBuild MSB8065 警告，破坏增量编译
  file(MAKE_DIRECTORY ${directory})
  add_custom_command(
    COMMAND bytecode_builtins_list_generator ${output}
    OUTPUT ${output}
    COMMENT "Generating ${output}"
    VERBATIM)
  add_library(v8-bytecodes-builtin-list INTERFACE)
  target_include_directories(v8-bytecodes-builtin-list INTERFACE ${target-dir})
  target_sources(v8-bytecodes-builtin-list INTERFACE ${output})
endfunction()
