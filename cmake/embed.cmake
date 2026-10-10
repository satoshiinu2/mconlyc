# 内容が違うときだけ書き込む(毎回書くと全再ビルドになる)
function(_write_if_different path content)
    set(old "")
    if(EXISTS "${path}")
        file(READ "${path}" old)
    endif()
    if(NOT "${content}" STREQUAL "${old}")
        file(WRITE "${path}" "${content}")
    endif()
endfunction()

# embed_assets(<出力変数> <アセットディレクトリ>)
#   生成物 (CMAKE_BINARY_DIR/embedded/):
#     asset_ids.h    : enum AssetId (ASSET_TERRAIN_PNG など)
#     asset_index.c  : assets[ASSET_COUNT]
#     asset_*.c      : 各ファイルのバイト配列
function(embed_assets out_var asset_dir)
    file(GLOB_RECURSE files CONFIGURE_DEPENDS RELATIVE "${asset_dir}" "${asset_dir}/*")
    list(SORT files)

    if(NOT files)
        message(FATAL_ERROR "embed_assets: no files in ${asset_dir}")
    endif()

    set(gen_dir "${CMAKE_BINARY_DIR}/embedded")
    file(MAKE_DIRECTORY "${gen_dir}")

    set(sources "")
    set(decls "")
    set(entries "")
    set(enum_entries "")
    set(seen "")

    foreach(rel ${files})
        string(MAKE_C_IDENTIFIER "asset_${rel}" sym)

        # "a-b.png" と "a_b.png" は同じ識別子になるので検出する
        if(sym IN_LIST seen)
            message(FATAL_ERROR "embed_assets: name collision: ${rel} -> ${sym}")
        endif()
        list(APPEND seen ${sym})

        string(TOUPPER "${sym}" enum_name)
        string(APPEND enum_entries "    ${enum_name},\n")

        set(in "${asset_dir}/${rel}")
        set(out "${gen_dir}/${sym}.c")

        file(READ "${in}" hex HEX)
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${hex}")
        file(SIZE "${in}" size)

        if(size EQUAL 0)
            message(FATAL_ERROR "embed_assets: empty file: ${rel}")
        endif()

        # 配列にサイズを明示する(_size 変数は C では定数式にならないので使わない)
        string(CONCAT content
            "const unsigned char ${sym}[${size}] = {${bytes}};\n")
        _write_if_different("${out}" "${content}")

        # アセット本体が変わったら CMake を再実行させる
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${in}")

        list(APPEND sources "${out}")

        # index 側も同じサイズで extern 宣言し、sizeof で取る
        string(APPEND decls
            "extern const unsigned char ${sym}[${size}];\n")
        string(APPEND entries
            "    [${enum_name}] = {\"${rel}\", ${sym}, sizeof(${sym})},\n")
    endforeach()

    # asset_ids.h
    string(CONCAT ids_content
        "#pragma once\n"
        "typedef enum {\n"
        "${enum_entries}"
        "    ASSET_COUNT,\n"
        "} AssetId;\n")
    _write_if_different("${gen_dir}/asset_ids.h" "${ids_content}")

    # asset_index.c
    string(CONCAT index_content
        "#include \"core/asset.h\"\n"
        "${decls}"
        "const Asset assets[ASSET_COUNT] = {\n"
        "${entries}"
        "};\n")
    _write_if_different("${gen_dir}/asset_index.c" "${index_content}")
    list(APPEND sources "${gen_dir}/asset_index.c")

    set(${out_var} ${sources} PARENT_SCOPE)
endfunction()

# omg i used claude