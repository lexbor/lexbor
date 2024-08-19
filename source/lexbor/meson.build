
lexbor_src = []

if sys_windows == true
  lexbor_src += [
    'ports/windows_nt/lexbor/fs.c',
    'ports/windows_nt/lexbor/memory.c'
  ]
  if get_option('perf')
    lexbor_src += [ 'ports/windows_nt/lexbor/perf.c' ]
  endif
else
  lexbor_src += [
    'ports/posix/lexbor/fs.c',
    'ports/posix/lexbor/memory.c'
  ]
  if get_option('perf')
    lexbor_src += [ 'ports/posix/lexbor/perf.c' ]
  endif
endif

lexbor_src += [
  'core/array.c',
  'core/array_obj.c',
  'core/avl.c',
  'core/bst.c',
  'core/bst_map.c',
  'core/conv.c',
  'core/diyfp.c',
  'core/dobject.c',
  'core/dtoa.c',
  'core/in.c',
  'core/mem.c',
  'core/mraw.c',
  'core/plog.c',
  'core/shbst.c',
  'core/shs.c',
  'core/str.c',
  'core/strtod.c',
  'core/utils.c'
]

lexbor_src += [
  'css/syntax/consume.c',
  'css/syntax/state.c',
  'css/syntax/token.c',
  'css/syntax/tokenizer.c',
  'css/syntax/tokenizer/error.c'
]

lexbor_src += [
  'dom/collection.c',
  'dom/exception.c',
  'dom/interface.c',
  'dom/qualified_name.c',
  'dom/interfaces/attr.c',
  'dom/interfaces/cdata_section.c',
  'dom/interfaces/character_data.c',
  'dom/interfaces/comment.c',
  'dom/interfaces/document.c',
  'dom/interfaces/document_fragment.c',
  'dom/interfaces/document_type.c',
  'dom/interfaces/element.c',
  'dom/interfaces/event_target.c',
  'dom/interfaces/node.c',
  'dom/interfaces/processing_instruction.c',
  'dom/interfaces/shadow_root.c',
  'dom/interfaces/text.c'
]

lexbor_src += [
  'html/html.c',
  'html/in.c',
  'html/interface.c',
  'html/parser.c',
  'html/parser_char.c',
  'html/parser_char_ref.c',
  'html/serialize.c',
  'html/token.c',
  'html/token_attr.c',
  'html/tokenizer.c',
  'html/tree.c',
  'html/interfaces/anchor_element.c',
  'html/interfaces/area_element.c',
  'html/interfaces/audio_element.c',
  'html/interfaces/base_element.c',
  'html/interfaces/body_element.c',
  'html/interfaces/br_element.c',
  'html/interfaces/button_element.c',
  'html/interfaces/canvas_element.c',
  'html/interfaces/d_list_element.c',
  'html/interfaces/data_element.c',
  'html/interfaces/data_list_element.c',
  'html/interfaces/details_element.c',
  'html/interfaces/dialog_element.c',
  'html/interfaces/directory_element.c',
  'html/interfaces/div_element.c',
  'html/interfaces/document.c',
  'html/interfaces/element.c',
  'html/interfaces/embed_element.c',
  'html/interfaces/field_set_element.c',
  'html/interfaces/font_element.c',
  'html/interfaces/form_element.c',
  'html/interfaces/frame_element.c',
  'html/interfaces/frame_set_element.c',
  'html/interfaces/head_element.c',
  'html/interfaces/heading_element.c',
  'html/interfaces/hr_element.c',
  'html/interfaces/html_element.c',
  'html/interfaces/iframe_element.c',
  'html/interfaces/image_element.c',
  'html/interfaces/input_element.c',
  'html/interfaces/label_element.c',
  'html/interfaces/legend_element.c',
  'html/interfaces/li_element.c',
  'html/interfaces/link_element.c',
  'html/interfaces/map_element.c',
  'html/interfaces/marquee_element.c',
  'html/interfaces/media_element.c',
  'html/interfaces/menu_element.c',
  'html/interfaces/meta_element.c',
  'html/interfaces/meter_element.c',
  'html/interfaces/mod_element.c',
  'html/interfaces/o_list_element.c',
  'html/interfaces/object_element.c',
  'html/interfaces/opt_group_element.c',
  'html/interfaces/option_element.c',
  'html/interfaces/output_element.c',
  'html/interfaces/paragraph_element.c',
  'html/interfaces/param_element.c',
  'html/interfaces/picture_element.c',
  'html/interfaces/pre_element.c',
  'html/interfaces/progress_element.c',
  'html/interfaces/quote_element.c',
  'html/interfaces/script_element.c',
  'html/interfaces/select_element.c',
  'html/interfaces/slot_element.c',
  'html/interfaces/source_element.c',
  'html/interfaces/span_element.c',
  'html/interfaces/style_element.c',
  'html/interfaces/table_caption_element.c',
  'html/interfaces/table_cell_element.c',
  'html/interfaces/table_col_element.c',
  'html/interfaces/table_element.c',
  'html/interfaces/table_row_element.c',
  'html/interfaces/table_section_element.c',
  'html/interfaces/template_element.c',
  'html/interfaces/text_area_element.c',
  'html/interfaces/time_element.c',
  'html/interfaces/title_element.c',
  'html/interfaces/track_element.c',
  'html/interfaces/u_list_element.c',
  'html/interfaces/unknown_element.c',
  'html/interfaces/video_element.c',
  'html/interfaces/window.c',
  'html/tokenizer/error.c',
  'html/tokenizer/state.c',
  'html/tokenizer/state_comment.c',
  'html/tokenizer/state_doctype.c',
  'html/tokenizer/state_rawtext.c',
  'html/tokenizer/state_rcdata.c',
  'html/tokenizer/state_script.c',
  'html/tree/active_formatting.c',
  'html/tree/error.c',
  'html/tree/open_elements.c',
  'html/tree/template_insertion.c',
  'html/tree/insertion_mode/after_after_body.c',
  'html/tree/insertion_mode/after_after_frameset.c',
  'html/tree/insertion_mode/after_body.c',
  'html/tree/insertion_mode/after_frameset.c',
  'html/tree/insertion_mode/after_head.c',
  'html/tree/insertion_mode/before_head.c',
  'html/tree/insertion_mode/before_html.c',
  'html/tree/insertion_mode/foreign_content.c',
  'html/tree/insertion_mode/in_body.c',
  'html/tree/insertion_mode/in_caption.c',
  'html/tree/insertion_mode/in_cell.c',
  'html/tree/insertion_mode/in_column_group.c',
  'html/tree/insertion_mode/in_frameset.c',
  'html/tree/insertion_mode/in_head.c',
  'html/tree/insertion_mode/in_head_noscript.c',
  'html/tree/insertion_mode/in_row.c',
  'html/tree/insertion_mode/in_select.c',
  'html/tree/insertion_mode/in_select_in_table.c',
  'html/tree/insertion_mode/in_table.c',
  'html/tree/insertion_mode/in_table_body.c',
  'html/tree/insertion_mode/in_table_text.c',
  'html/tree/insertion_mode/in_template.c',
  'html/tree/insertion_mode/initial.c',
  'html/tree/insertion_mode/text.c'
]

lexbor_src += [
  'ns/ns.c'
]

lexbor_src += [
  'tag/tag.c'
]

lexbor_lib = library('lexbor', lexbor_src,
  c_args : lexbor_args,
  include_directories : config_dir,
  install : true,
  version : meson.project_version()
)

lexbor = declare_dependency(
  include_directories : config_dir,
  link_with : lexbor_lib
)
