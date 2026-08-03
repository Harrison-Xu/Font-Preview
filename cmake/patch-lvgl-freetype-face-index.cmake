# SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
# SPDX-License-Identifier: MIT

function(font_preview_patch_lvgl_freetype_face_index lvgl_source_dir)
    set(freetype_source "${lvgl_source_dir}/src/libs/freetype/lv_freetype.c")
    if(NOT EXISTS "${freetype_source}")
        message(FATAL_ERROR "LVGL FreeType source not found: ${freetype_source}")
    endif()

    file(READ "${freetype_source}" source_text)
    set(patch_marker "Font Preview TTC face selector")
    if(source_text MATCHES "${patch_marker}")
        return()
    endif()

    set(original [=[    FT_Error error = FT_New_Face(ctx->library, node->pathname, 0, &face);]=])
    set(replacement [=[    /* Font Preview TTC face selector: a trailing #0..#9 selects a collection face. */
    const char * face_pathname = node->pathname;
    char * allocated_pathname = NULL;
    FT_Long face_index = 0;
    const size_t pathname_len = lv_strlen(node->pathname);
    if(pathname_len >= 2 && node->pathname[pathname_len - 2] == '#' &&
       node->pathname[pathname_len - 1] >= '0' && node->pathname[pathname_len - 1] <= '9') {
        const size_t file_path_len = pathname_len - 2;
        allocated_pathname = lv_malloc(file_path_len + 1);
        if(!allocated_pathname) {
            LV_LOG_ERROR("failed to allocate TTC pathname");
            return false;
        }
        lv_memcpy(allocated_pathname, node->pathname, file_path_len);
        allocated_pathname[file_path_len] = '\0';
        face_pathname = allocated_pathname;
        face_index = node->pathname[pathname_len - 1] - '0';
    }

    FT_Error error = FT_New_Face(ctx->library, face_pathname, face_index, &face);
    if(allocated_pathname) lv_free(allocated_pathname);]=])

    string(FIND "${source_text}" "${original}" original_position)
    if(original_position EQUAL -1)
        message(FATAL_ERROR "LVGL FreeType loader changed; cannot apply TTC face-index patch")
    endif()

    string(REPLACE "${original}" "${replacement}" patched_text "${source_text}")
    file(WRITE "${freetype_source}" "${patched_text}")
    message(STATUS "Patched LVGL FreeType loader with TTC face-index support")
endfunction()
