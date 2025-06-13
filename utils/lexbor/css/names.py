
import sys, re, os, datetime

# Find and append run script run dir to module search path
ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../lexbor/".format(ABS_PATH))

import LXB

# CSS StyleSheet entries

at_rules = {
    "media": {"initial": "NULL"},
    "namespace": {"initial": "NULL"}
}

min_max = ["min-content", "max-content", "_length", "_percentage", "_number", "_angle"]
margin_padding = ["auto", "_length", "_percentage"]
border_values = ["thin", "medium", "thick",
                 "none", "hidden", "dotted", "dashed", "solid", "double",
                 "groove", "ridge", "inset", "outset", "_length"]

color_named = ["aliceblue", "antiquewhite", "aqua", "aquamarine", "azure",
               "beige", "bisque", "black", "blanchedalmond", "blue",
               "blueviolet", "brown", "burlywood", "cadetblue",
               "chartreuse", "chocolate", "coral", "cornflowerblue",
               "cornsilk", "crimson", "cyan", "darkblue", "darkcyan",
               "darkgoldenrod", "darkgray", "darkgreen", "darkgrey",
               "darkkhaki", "darkmagenta", "darkolivegreen", "darkorange",
               "darkorchid", "darkred", "darksalmon", "darkseagreen",
               "darkslateblue", "darkslategray", "darkslategrey",
               "darkturquoise", "darkviolet", "deeppink", "deepskyblue",
               "dimgray", "dimgrey", "dodgerblue", "firebrick", "floralwhite",
               "forestgreen", "fuchsia", "gainsboro", "ghostwhite", "gold",
               "goldenrod", "gray", "green", "greenyellow", "grey", "honeydew",
               "hotpink", "indianred", "indigo", "ivory", "khaki", "lavender",
               "lavenderblush", "lawngreen", "lemonchiffon", "lightblue",
               "lightcoral", "lightcyan", "lightgoldenrodyellow", "lightgray",
               "lightgreen", "lightgrey", "lightpink", "lightsalmon",
               "lightseagreen", "lightskyblue", "lightslategray",
               "lightslategrey", "lightsteelblue", "lightyellow", "lime",
               "limegreen", "linen", "magenta", "maroon", "mediumaquamarine",
               "mediumblue", "mediumorchid", "mediumpurple", "mediumseagreen",
               "mediumslateblue", "mediumspringgreen", "mediumturquoise",
               "mediumvioletred", "midnightblue", "mintcream", "mistyrose",
               "moccasin", "navajowhite", "navy", "oldlace", "olive",
               "olivedrab", "orange", "orangered", "orchid", "palegoldenrod",
               "palegreen", "paleturquoise", "palevioletred", "papayawhip",
               "peachpuff", "peru", "pink", "plum", "powderblue", "purple",
               "rebeccapurple", "red", "rosybrown", "royalblue", "saddlebrown",
               "salmon", "sandybrown", "seagreen", "seashell", "sienna",
               "silver", "skyblue", "slateblue", "slategray", "slategrey",
               "snow", "springgreen", "steelblue", "tan", "teal", "thistle",
               "tomato", "turquoise", "violet", "wheat", "white", "whitesmoke",
               "yellow", "yellowgreen"]

color_system = ["Canvas", "CanvasText", "LinkText", "VisitedText", "ActiveText",
                "ButtonFace", "ButtonText", "ButtonBorder", "Field", "FieldText",
                "Highlight", "HighlightText", "SelectedItem", "SelectedItemText",
                "Mark", "MarkText", "GrayText", "AccentColor", "AccentColorText"]

color_function = ["rgb", "rgba",
                  "hsl", "hsla", "hwb",
                  "lab", "lch", "oklab", "oklch",
                  "color"]

color_values = ["currentcolor", "transparent", "hex"] + color_named + color_system + color_function

length_0 = ".u.length = {.num = 0, .is_float = false}"
length_percentage_0 = "{.type = LXB_CSS_VALUE__LENGTH, %s}" % (length_0)
length_percentage_auto = "{.type = LXB_CSS_VALUE_AUTO, %s}" % (length_0)
border_init = "{.style = LXB_CSS_BORDER_NONE, .width = {.type = LXB_CSS_BORDER_MEDIUM}, .color = {.type = LXB_CSS_COLOR_CURRENTCOLOR}}"

styles = {
    # https://drafts.csswg.org/css-display/

    "display": {
        "values": [
            # <display-outside>
            "block", "inline", "run-in",
            # <display-inside>
            "flow", "flow-root", "table", "flex", "grid", "ruby",
            # <display-listitem>
            "list-item",
            # <display-internal>
            "table-row-group", "table-header-group", "table-footer-group",
            "table-row", "table-cell", "table-column-group", "table-column",
            "table-caption", "ruby-base", "ruby-text", "ruby-base-container",
            "ruby-text-container",
            # <display-box>
            "contents", "none",
            # <display-legacy>
            "inline-block", "inline-table", "inline-flex", "inline-grid"
        ],
        "initial": "&(lxb_css_property_display_t) {.a = LXB_CSS_DISPLAY_INLINE, .b = LXB_CSS_PROPERTY__UNDEF, .c = LXB_CSS_PROPERTY__UNDEF}"
    },

    "order": {"values": ["_integer"], "initial": "&(lxb_css_property_order_t) {.type = LXB_CSS_ORDER__INTEGER, .integer = {.num = 0}}"},
    "visibility": {"values": ["visible", "hidden", "collapse"], "initial": "&(lxb_css_property_visibility_t) {.type = LXB_CSS_VISIBILITY_VISIBLE}"},

    # https://drafts.csswg.org/css-sizing-3/

    "box-sizing": {
        "values": ["content-box", "border-box"],
        "initial": "&(lxb_css_property_box_sizing_t) {.type = LXB_CSS_BOX_SIZING_CONTENT_BOX}"
    },

    "width": {"values": ["auto"] + min_max, "initial": "&(lxb_css_property_width_t) {.type = LXB_CSS_WIDTH_AUTO, %s}" % (length_0)},
    "height": {"values": ["auto"] + min_max, "initial": "&(lxb_css_property_height_t) {.type = LXB_CSS_HEIGHT_AUTO, %s}" % (length_0)},
    "min-width": {"values": ["auto"] + min_max, "initial": "&(lxb_css_property_min_width_t) {.type = LXB_CSS_MIN_WIDTH_AUTO, %s}" % (length_0)},
    "min-height": {"values": ["auto"] + min_max, "initial": "&(lxb_css_property_min_height_t) {.type = LXB_CSS_MIN_HEIGHT_AUTO, %s}" % (length_0)},
    "max-width": {"values": ["none"] + min_max, "initial": "&(lxb_css_property_max_width_t) {.type = LXB_CSS_MAX_WIDTH_NONE, %s}" % (length_0)},
    "max-height": {"values": ["none"] + min_max, "initial": "&(lxb_css_property_max_height_t) {.type = LXB_CSS_MAX_HEIGHT_NONE, %s}" % (length_0)},

    # https://drafts.csswg.org/css-box-3/

    "margin": {"values": margin_padding, "initial": "&(lxb_css_property_margin_t) {.top = %s, .right = %s, .bottom = %s, .left = %s}" % (length_percentage_0, length_percentage_0, length_percentage_0, length_percentage_0)},
    "margin-top": {"values": margin_padding, "initial": "&(lxb_css_property_margin_top_t) " + length_percentage_0},
    "margin-right": {"values": margin_padding, "initial": "&(lxb_css_property_margin_right_t) " + length_percentage_0},
    "margin-bottom": {"values": margin_padding, "initial": "&(lxb_css_property_margin_bottom_t) " + length_percentage_0},
    "margin-left": {"values": margin_padding, "initial": "&(lxb_css_property_margin_left_t) " + length_percentage_0},

    "padding": {"values": margin_padding, "initial": "&(lxb_css_property_padding_t) {.top = %s, .right = %s, .bottom = %s, .left = %s}" % (length_percentage_0, length_percentage_0, length_percentage_0, length_percentage_0)},
    "padding-top": {"values": margin_padding, "initial": "&(lxb_css_property_padding_top_t) " + length_percentage_0},
    "padding-right": {"values": margin_padding, "initial": "&(lxb_css_property_padding_right_t) " + length_percentage_0},
    "padding-bottom": {"values": margin_padding, "initial": "&(lxb_css_property_padding_bottom_t) " + length_percentage_0},
    "padding-left": {"values": margin_padding, "initial": "&(lxb_css_property_padding_left_t) " + length_percentage_0},

    # https://drafts.csswg.org/css-backgrounds/

    "border": {"values": border_values, "initial": "&(lxb_css_property_border_t) " + border_init},
    "border-top": {"values": border_values, "initial": "&(lxb_css_property_border_top_t) " + border_init},
    "border-right": {"values": border_values, "initial": "&(lxb_css_property_border_right_t) " + border_init},
    "border-bottom": {"values": border_values, "initial": "&(lxb_css_property_border_bottom_t) " + border_init},
    "border-left": {"values": border_values, "initial": "&(lxb_css_property_border_left_t) " + border_init},

    "border-top-color": {"values": [], "initial": "&(lxb_css_property_border_top_color_t) {.type = LXB_CSS_COLOR_CURRENTCOLOR}"},
    "border-right-color": {"values": [], "initial": "&(lxb_css_property_border_right_color_t) {.type = LXB_CSS_COLOR_CURRENTCOLOR}"},
    "border-bottom-color": {"values": [], "initial": "&(lxb_css_property_border_bottom_color_t) {.type = LXB_CSS_COLOR_CURRENTCOLOR}"},
    "border-left-color": {"values": [], "initial": "&(lxb_css_property_border_left_color_t) {.type = LXB_CSS_COLOR_CURRENTCOLOR}"},

    "background-color": {"values": [], "initial": "&(lxb_css_property_background_color_t) {.type = LXB_CSS_COLOR_TRANSPARENT}"},

    # https://drafts.csswg.org/css-color/

    "color": {"values": color_values, "initial": "&(lxb_css_value_color_t) {.type = LXB_CSS_COLOR_CURRENTCOLOR}"},
    "opacity": {"values": ["_number", "_percentage"], "initial": "&(lxb_css_property_opacity_t) {.type = LXB_CSS_OPACITY__NUMBER, .u = {.number = {.num = 1, .is_float = false}}}"},

    # https://drafts.csswg.org/css-position-3/

    "position": {"values": ["static", "relative", "absolute", "sticky", "fixed"], "initial": "&(lxb_css_property_position_t) {.type = LXB_CSS_POSITION_STATIC}"},

    "top": {"values": margin_padding, "initial": "&(lxb_css_property_top_t) " + length_percentage_auto},
    "right": {"values": margin_padding, "initial": "&(lxb_css_property_right_t) " + length_percentage_auto},
    "bottom": {"values": margin_padding, "initial": "&(lxb_css_property_bottom_t) " + length_percentage_auto},
    "left": {"values": margin_padding, "initial": "&(lxb_css_property_left_t) " + length_percentage_auto},

    "inset-block-start": {"values": margin_padding, "initial": "&(lxb_css_property_inset_block_start_t) " + length_percentage_auto},
    "inset-inline-start": {"values": margin_padding, "initial": "&(lxb_css_property_inset_inline_start_t) " + length_percentage_auto},
    "inset-block-end": {"values": margin_padding, "initial": "&(lxb_css_property_inset_block_end_t) " + length_percentage_auto},
    "inset-inline-end": {"values": margin_padding, "initial": "&(lxb_css_property_inset_inline_end_t) " + length_percentage_auto},

    # https://drafts.csswg.org/css-text-3/

    "text-transform": {"values": ["none", "capitalize", "uppercase", "lowercase", "full-width", "full-size-kana"], "initial": "&(lxb_css_property_text_transform_t) {.type_case = LXB_CSS_TEXT_TRANSFORM_NONE, .full_width = LXB_CSS_PROPERTY__UNDEF, .full_size_kana = LXB_CSS_PROPERTY__UNDEF}"},
    "text-align": {"values": ["start", "end", "left", "right", "center", "justify", "match-parent", "justify-all"], "initial": "&(lxb_css_property_text_align_t) {.type = LXB_CSS_TEXT_ALIGN_START}"},
    "text-align-all": {"values": ["start", "end", "left", "right", "center", "justify", "match-parent"], "initial": "&(lxb_css_property_text_align_all_t) {.type = LXB_CSS_TEXT_ALIGN_ALL_START}"},
    "text-align-last": {"values": ["auto", "start", "end", "left", "right", "center", "justify", "match-parent"], "initial": "&(lxb_css_property_text_align_last_t) {.type = LXB_CSS_TEXT_ALIGN_LAST_AUTO}"},
    "text-justify": {"values": ["auto", "none", "inter-word", "inter-character"], "initial": "&(lxb_css_property_text_justify_t) {.type = LXB_CSS_TEXT_JUSTIFY_AUTO}"},
    "text-indent": {"values": ["_length", "_percentage", "hanging", "each-line"], "initial": "&(lxb_css_property_text_indent_t) {.length = {.type = LXB_CSS_VALUE__LENGTH, .u = {.length = {.num = 0, .is_float = false, .unit = LXB_CSS_UNIT__UNDEF}}}}"},
    "white-space": {"values": ["normal", "pre", "nowrap", "pre-wrap", "break-spaces", "pre-line"], "initial": "&(lxb_css_property_white_space_t) {.type = LXB_CSS_WHITE_SPACE_NORMAL}"},
    "tab-size": {"values": ["_number", "_length"], "initial": "&(lxb_css_property_tab_size_t) {.type = LXB_CSS_VALUE__NUMBER, .u = {.number = {.num = 8, .is_float = false}}}"},
    "word-break": {"values": ["normal", "keep-all", "break-all", "break-word"], "initial": "&(lxb_css_property_word_break_t) {.type = LXB_CSS_WORD_BREAK_NORMAL}"},
    "line-break": {"values": ["auto", "loose", "normal", "strict", "anywhere"], "initial": "&(lxb_css_property_line_break_t) {.type = LXB_CSS_LINE_BREAK_AUTO}"},
    "hyphens": {"values": ["none", "manual", "auto"], "initial": "&(lxb_css_property_hyphens_t) {.type = LXB_CSS_HYPHENS_MANUAL}"},
    "overflow-wrap": {"values": ["normal", "break-word", "anywhere"], "initial": "&(lxb_css_property_overflow_wrap_t) {.type = LXB_CSS_OVERFLOW_WRAP_NORMAL}"},
    "word-wrap": {"values": ["normal", "break-word", "anywhere"], "initial": "&(lxb_css_property_word_wrap_t) {.type = LXB_CSS_WORD_WRAP_NORMAL}"},
    "word-spacing": {"values": ["normal", "_length"], "initial": "&(lxb_css_property_word_spacing_t) {.type = LXB_CSS_WORD_SPACING_NORMAL}"},
    "letter-spacing": {"values": ["normal", "_length"], "initial": "&(lxb_css_property_letter_spacing_t) {.type = LXB_CSS_LETTER_SPACING_NORMAL}"},
    "hanging-punctuation": {"values": ["none", "first", "force-end", "allow-end", "last"], "initial": "&(lxb_css_property_hanging_punctuation_t) {.type_first = LXB_CSS_HANGING_PUNCTUATION_NONE}"},

    # https://drafts.csswg.org/css-fonts/

    "font-family": {"values": ["serif", "sans-serif", "cursive", "fantasy", "monospace", "system-ui", "emoji", "math", "fangsong", "ui-serif", "ui-sans-serif", "ui-monospace", "ui-rounded"], "initial": "NULL"},
    "font-weight": {"values": ["normal", "bold", "_number", "bolder", "lighter"], "initial": "&(lxb_css_property_font_weight_t) {.type = LXB_CSS_FONT_WEIGHT_NORMAL}"},
    "font-stretch": {"values": ["normal", "_percentage", "ultra-condensed", "extra-condensed", "condensed", "semi-condensed", "semi-expanded", "expanded", "extra-expanded", "ultra-expanded"], "initial": "&(lxb_css_property_font_stretch_t) {.type = LXB_CSS_FONT_STRETCH_NORMAL}"},
    "font-style": {"values": ["normal", "italic", "oblique"], "initial": "&(lxb_css_property_font_style_t) {.type = LXB_CSS_FONT_STYLE_NORMAL}"},
    "font-size": {"values": ["xx-small", "x-small", "small", "medium", "large", "x-large", "xx-large", "xxx-large", "larger", "smaller", "math", "_length"], "initial": "&(lxb_css_property_font_size_t) {.type = LXB_CSS_FONT_SIZE_MEDIUM}"},

    # https://drafts.csswg.org/css-page-floats/

    "float-reference": {"values": ["inline", "column", "region", "page"], "initial": "&(lxb_css_property_float_reference_t) {.type = LXB_CSS_FLOAT_REFERENCE_INLINE}"},
    "float": {"values": ["block-start", "block-end", "inline-start", "inline-end", "snap-block", "start", "end", "near", "snap-inline", "left", "right", "top", "bottom", "none"], "initial": "&(lxb_css_property_float_t) {.type = LXB_CSS_FLOAT_NONE}"},
    "clear": {"values": ["inline-start", "inline-end", "block-start", "block-end", "left", "right", "top", "bottom", "none"], "initial": "&(lxb_css_property_clear_t) {.type = LXB_CSS_CLEAR_NONE}"},
    "float-defer": {"values": ["_integer", "last", "none"], "initial": "&(lxb_css_property_float_defer_t) {.type = LXB_CSS_FLOAT_DEFER_NONE}"},
    "float-offset": {"values": ["_length", "_percentage"], "initial": "&(lxb_css_property_float_offset_t) {.type = LXB_CSS_VALUE__NUMBER, .u = {.length = {.num = 0, .is_float = false, .unit = LXB_CSS_UNIT__UNDEF}}}"},

    # https://drafts.csswg.org/css-exclusions/

    "wrap-flow": {"values": ["auto", "both", "start", "end", "minimum", "maximum", "clear"], "initial": "&(lxb_css_property_wrap_flow_t) {.type = LXB_CSS_WRAP_FLOW_AUTO}"},
    "wrap-through": {"values": ["wrap", "none"], "initial": "&(lxb_css_property_wrap_through_t) {.type = LXB_CSS_WRAP_THROUGH_WRAP}"},

    # https://drafts.csswg.org/css-flexbox/

    "flex-direction": {"values": ["row", "row-reverse", "column", "column-reverse"], "initial": "&(lxb_css_property_flex_direction_t) {.type = LXB_CSS_FLEX_DIRECTION_ROW}"},
    "flex-wrap": {"values": ["nowrap", "wrap", "wrap-reverse"], "initial": "&(lxb_css_property_flex_wrap_t) {.type = LXB_CSS_FLEX_WRAP_NOWRAP}"},
    "flex-flow": {"values": [], "initial": "&(lxb_css_property_flex_flow_t) {.type_direction = LXB_CSS_FLEX_DIRECTION_ROW, .wrap = LXB_CSS_FLEX_WRAP_NOWRAP}"},
    "flex": {"values": ["none"], "initial": "&(lxb_css_property_flex_t) {.type = LXB_CSS_VALUE__UNDEF, .grow = {.type = LXB_CSS_FLEX_GROW__NUMBER, .number = {.num = 0, .is_float = false}}, .shrink = {.type = LXB_CSS_FLEX_SHRINK__NUMBER, .number = {.num = 1, .is_float = false}}, .basis = {.type = LXB_CSS_WIDTH_AUTO, %s}}" % (length_0)},
    "flex-grow": {"values": ["_number"], "initial": "&(lxb_css_property_flex_grow_t) {.type = LXB_CSS_FLEX_GROW__NUMBER, .number = {.num = 0, .is_float = false}}"},
    "flex-shrink": {"values": ["_number"], "initial":  "&(lxb_css_property_flex_shrink_t) {.type = LXB_CSS_FLEX_SHRINK__NUMBER, .number = {.num = 1, .is_float = false}}"},
    "flex-basis": {"values": ["content"], "initial": "&(lxb_css_property_flex_basis_t) {.type = LXB_CSS_WIDTH_AUTO, %s}" % length_0},
    "justify-content": {"values": ["flex-start", "flex-end", "center", "space-between", "space-around"], "initial": "&(lxb_css_property_justify_content_t) {.type = LXB_CSS_JUSTIFY_CONTENT_FLEX_START}"},
    "align-items": {"values": ["flex-start", "flex-end", "center", "baseline", "stretch"], "initial": "&(lxb_css_property_align_items_t) {.type = LXB_CSS_ALIGN_ITEMS_STRETCH}"},
    "align-self": {"values": ["auto", "flex-start", "flex-end", "center", "baseline", "stretch"], "initial": "&(lxb_css_property_align_self_t) {.type = LXB_CSS_ALIGN_SELF_AUTO}"},
    "align-content": {"values": ["flex-start", "flex-end", "center", "space-between", "space-around", "stretch"], "initial": "&(lxb_css_property_align_content_t) {.type = LXB_CSS_ALIGN_CONTENT_STRETCH}"},

    # https://drafts.csswg.org/css-inline/

    "dominant-baseline": {"values": ["auto", "text-bottom", "alphabetic", "ideographic", "middle", "central", "mathematical", "hanging", "text-top"], "initial": "&(lxb_css_property_dominant_baseline_t) {.type = LXB_CSS_DOMINANT_BASELINE_AUTO}"},
    "vertical-align": {"values": ["first", "last"], "initial": "&(lxb_css_property_vertical_align_t) {.type = LXB_CSS_ALIGNMENT_BASELINE_BASELINE}"},
    "baseline-source": {"values": ["auto", "first", "last"], "initial": "&(lxb_css_property_baseline_source_t) {.type = LXB_CSS_BASELINE_SOURCE_AUTO}"},
    "alignment-baseline": {"values": ["baseline", "text-bottom", "alphabetic", "ideographic", "middle", "central", "mathematical", "text-top"], "initial": "&(lxb_css_property_alignment_baseline_t) {.type = LXB_CSS_ALIGNMENT_BASELINE_BASELINE}"},
    "baseline-shift": {"values": ["_length", "_percentage", "sub", "super", "top", "center", "bottom"], "initial": "&(lxb_css_property_baseline_shift_t) {.type = LXB_CSS_VALUE__NUMBER, .u = {.length = {.num = 0, .is_float = false, .unit = LXB_CSS_UNIT__UNDEF}}}"},
    "line-height": {"values": ["normal", "_number", "_length", "_percentage"], "initial": "&(lxb_css_property_line_height_t) {.type = LXB_CSS_LINE_HEIGHT_NORMAL}"},

    # https://drafts.csswg.org/css2/#z-index

    "z-index": {"values": ["auto", "_integer"], "initial": "&(lxb_css_property_z_index_t) {.type = LXB_CSS_Z_INDEX_AUTO}"},

    # https://drafts.csswg.org/css-writing-modes/

    "direction": {"values": ["ltr", "rtl"], "initial": "&(lxb_css_property_direction_t) {.type = LXB_CSS_DIRECTION_LTR}"},
    "unicode-bidi": {"values": ["normal", "embed", "isolate", "bidi-override", "isolate-override", "plaintext"], "initial": "&(lxb_css_property_unicode_bidi_t) {.type = LXB_CSS_UNICODE_BIDI_NORMAL}"},
    "writing-mode": {"values": ["horizontal-tb", "vertical-rl", "vertical-lr", "sideways-rl", "sideways-lr"], "initial": "&(lxb_css_property_writing_mode_t) {.type = LXB_CSS_WRITING_MODE_HORIZONTAL_TB}"},
    "text-orientation": {"values": ["mixed", "upright", "sideways"], "initial": "&(lxb_css_property_text_orientation_t) {.type = LXB_CSS_TEXT_ORIENTATION_MIXED}"},
    "text-combine-upright": {"values": ["none", "all", "digits"], "initial": "&(lxb_css_property_text_combine_upright_t) {.type = LXB_CSS_TEXT_COMBINE_UPRIGHT_NONE}"},

    # https://drafts.csswg.org/css-overflow/

    "overflow-x": {"values": ["visible", "hidden", "clip", "scroll", "auto"], "initial": "&(lxb_css_property_overflow_x_t) {.type = LXB_CSS_OVERFLOW_X_VISIBLE}"},
    "overflow-y": {"values": ["visible", "hidden", "clip", "scroll", "auto"], "initial": "&(lxb_css_property_overflow_y_t) {.type = LXB_CSS_OVERFLOW_Y_VISIBLE}"},
    "overflow-block": {"values": ["visible", "hidden", "clip", "scroll", "auto"], "initial": "&(lxb_css_property_overflow_block_t) {.type = LXB_CSS_OVERFLOW_BLOCK_VISIBLE}"},
    "overflow-inline": {"values": ["visible", "hidden", "clip", "scroll", "auto"], "initial": "&(lxb_css_property_overflow_inline_t) {.type = LXB_CSS_OVERFLOW_INLINE_VISIBLE}"},

    "text-overflow": {"values": ["clip", "ellipsis"], "initial": "&(lxb_css_property_text_overflow_t) {.type = LXB_CSS_TEXT_OVERFLOW_CLIP}"},

    # https://drafts.csswg.org/css-text-decor/

    "text-decoration-line": {"values": ["none", "underline", "overline", "line-through", "blink"], "initial": "&(lxb_css_property_text_decoration_line_t) {.type = LXB_CSS_TEXT_DECORATION_LINE_NONE}"},
    "text-decoration-style": {"values": ["solid", "double", "dotted", "dashed", "wavy"], "initial": "&(lxb_css_property_text_decoration_style_t) {.type = LXB_CSS_TEXT_DECORATION_STYLE_SOLID}"},
    "text-decoration-color": {"values": [], "initial": "&(lxb_css_property_text_decoration_color_t) {.type = LXB_CSS_COLOR_CURRENTCOLOR}"},
    "text-decoration": {"values": [], "initial": "&(lxb_css_property_text_decoration_t) {.line = {.type = LXB_CSS_TEXT_DECORATION_LINE_NONE}, .style = {.type = LXB_CSS_TEXT_DECORATION_STYLE_SOLID}, .color = {.type = LXB_CSS_COLOR_CURRENTCOLOR}}"}
}

compiles = [
    ["at_rule", at_rules, False],
    ["property", styles, True]
]

global_values = {
    "initial": [], "inherit": [], "unset": [], "revert": []
}

# Units

unit_absolute = {
    "cm": [], "mm": [], "Q": [], "in": [], "pc": [],
    "pt": [], "px": []
}

unit_relative = {
    "em": [], "ex": [], "cap": [], "ch": [], "ic": [],
    "rem": [], "lh": [], "rlh": [], "vw": [], "vh": [],
    "vi": [], "vb": [], "vmin": [], "vmax": []
}

unit_angle = {
    "deg": [],
    "grad": [],
    "rad": [],
    "turn": []
}

unit_frequency = {
    "Hz": [], "kHz": []
}

unit_resolution = {
    "dpi": [], "dpcm": [], "dppx": [], "x": []
}

unit_duration = {
    "s": [], "ms": []
}

units = [
    ["absolute", unit_absolute],
    ["relative", unit_relative],
    ["angel", unit_angle],
    ["frequency", unit_frequency],
    ["resolution", unit_resolution],
    ["duration", unit_duration]
]

class Pseudo:
    prefix = "LXB_CSS_"
    const_tmp = "tmp/const.h"
    res_tmp = "tmp/res.h"
    value_const_tmp = "tmp/value_const.h"
    value_res_tmp = "tmp/value_res.h"

    def make(self, prefix, values):
        idx = 0
        entries = []
        data_name = "lxb_css_{}_data".format(prefix)

        arr = list(values.keys())
        arr.sort()
        arr.insert(0, "_undef")
        arr.insert(1, "_custom")

        res = LXB.Res("lxb_css_entry_data_t", data_name, False,
                      self.make_enum_name(prefix, "_LAST_ENTRY"))

        frmt_enum = LXB.FormatEnum("{}{}_type_t".format(self.prefix.lower(),
                                                        self.make_name(prefix)))

        for name in arr:
            origin = name
            name = self.make_name(name)
            enum_name = self.make_enum_name(prefix, name)
            func_name = "lxb_css_{}_state_{}".format(prefix, name)

            if origin not in values or "hide" not in values[origin] or values[origin]["hide"] == False:
                frmt_enum.append(enum_name, "0x{0:04x}".format(idx))
                idx += 1

            create = "lxb_css_{}_{}_create".format(prefix, name)
            destroy = "lxb_css_{}_{}_destroy".format(prefix, name)
            serialize = "lxb_css_{}_{}_serialize".format(prefix, name)

            if name == "_undef":
                res.append("{{(lxb_char_t *) \"#undef\", 6, {}, {},\n"
                           "     {}, {}, {}, {}}}".format(
                               enum_name, func_name, create, destroy, serialize,
                               "(void *) (uintptr_t) " + enum_name
                            ))
                continue
            elif name == "_custom":
                res.append("{{(lxb_char_t *) \"#сustom\", 7, {}, {},\n"
                           "     {}, {}, {}, {}}}".format(
                               enum_name, func_name, create, destroy, serialize,
                               "(void *) (uintptr_t) " + enum_name
                            ))
                continue
            elif "hide" not in values[origin] or values[origin]["hide"] == False:
                res.append("{{(lxb_char_t *) \"{}\", {}, {}, {},\n"
                           "     {}, {}, {},\n"
                           "     {}}}".format(origin, len(name),
                                enum_name, func_name, create, destroy, serialize,
                                values[origin]["initial"]
                            ))

            if "hide" not in values[origin] or values[origin]["hide"] == False:
                entries.append({"key": origin,
                                "value": "(void *) &{}[{}]".format(data_name, enum_name)})

        frmt_enum.append(self.make_enum_name(prefix, "_LAST_ENTRY"),
                                             "0x{0:04x}".format(idx))

        shs = LXB.SHS(entries, 0, False)
        test = shs.make_test(5, 256)
        shs.table_size_set(test[0][2])

        result_enum = frmt_enum.build(typedef = "uintptr_t")
        result_res = res.create(rate = 1, is_const = True)
        result_sh = shs.create("lxb_css_{}_shs".format(prefix), rate = 1)

        print(self.shs_stat(shs))

        return [result_enum, result_res, result_sh]

    def make_value(self, values):
        vals_hash = []
        all_vals = {}
        results = []

        arr = list(values.keys())
        arr.sort()

        res = LXB.Res("lxb_css_data_t", "lxb_css_value_data", False,
                      self.make_enum_name("VALUE", "_LAST_ENTRY"))

        all_props_enum = LXB.FormatEnum("{}{}_type_t".format(self.prefix.lower(),
                                                             "value"))
        all_vals["_undef"] = 0
        values_idx = 1

        base_name = self.make_enum_name("value", "_undef")
        all_props_enum.append(base_name, "0x{0:04x}".format(all_vals["_undef"]))
        res.append("{{(lxb_char_t *) \"{}\", {}, {}}}".format("_undef", len("_undef"), base_name))

        for val in global_values:
            all_vals[val] = values_idx
            values_idx += 1

            base_name = self.make_enum_name("value", val)
            all_props_enum.append(base_name, "0x{0:04x}".format(all_vals[val]))
            vals_hash.append({"key": val.lower(), "value": "(void *) {}".format(base_name)})
            res.append("{{(lxb_char_t *) \"{}\", {}, {}}}".format(val, len(val), base_name))

        for name in arr:
            if "values" not in values[name]:
                continue

            vals = values[name]["values"]
            origin = name
            name = self.make_name(name)

            prop_enum = LXB.FormatEnum("{}{}_type_t".format(self.prefix.lower(),
                                                             self.make_name(name)))

            for val in vals:
                base_name = self.make_enum_name("value", val)

                if val not in all_vals:
                    all_vals[val] = values_idx
                    values_idx += 1

                    all_props_enum.append(base_name, "0x{0:04x}".format(all_vals[val]))
                    vals_hash.append({"key": val.lower(), "value": "(void *) {}".format(base_name)})

                    res.append("{{(lxb_char_t *) \"{}\", {}, {}}}".format(val, len(val), base_name))

                enum_name = self.make_enum_name(name, val)
                prop_enum.append(enum_name, base_name)

            result_enum = prop_enum.build(typedef = "unsigned int")
            results.append(result_enum)

            print("\n".join(result_enum))

        all_vals["_last_entry"] = values_idx
        base_name = self.make_enum_name("value", "_last_entry")
        all_props_enum.append(base_name, "0x{0:04x}".format(all_vals["_last_entry"]))

        result_all_enum = all_props_enum.build(typedef = "unsigned int")
        print("\n".join(result_all_enum))

        result_res = res.create(rate = 1, is_const = True)

        vals_shs = LXB.SHS(vals_hash, 0, False)
        test = vals_shs.make_test(5, 256)
        vals_shs.table_size_set(test[0][2])

        values_sh = vals_shs.create("lxb_css_{}_shs".format("value"), rate = 1)

        print(self.shs_stat(vals_shs))

        return [result_all_enum, results, values_sh, result_res]

    def make_units(self, prefix, values):
        result = []
        abre = []
        idx = 1
        data_name = "lxb_css_{}_data".format(prefix)
        enum_name = self.make_enum_name(prefix, "_undef")

        res = LXB.Res("lxb_css_data_t", data_name, False,
                      self.make_enum_name(prefix, "_LAST_ENTRY"))

        res.append("{{(lxb_char_t *) \"#undef\", 6, {}}}".format(enum_name))

        for (title, arr) in values:
            entries = []
            unit_name = "{}_{}".format(prefix, title)

            frmt_enum = LXB.FormatEnum("{}{}_{}_t".format(self.prefix.lower(),
                                                        self.make_name(prefix), title))
            arr = list(arr.keys())
            arr.sort()

            arr.insert(0,"_undef")

            for name in arr:
                enum_name = self.make_enum_name(prefix, name)

                if name == "_undef":
                    undef_name = self.make_enum_name("{}_{}".format(prefix, title), "_begin")
                    frmt_enum.append(undef_name, "0x{0:04x}".format(idx))

                    continue
                else:
                    frmt_enum.append(enum_name, "0x{0:04x}".format(idx))
                    idx += 1

                    res.append("{{(lxb_char_t *) \"{}\", {}, {}}}".format(name, len(name),
                                                                          enum_name))

                    entries.append({"key": name.lower(),
                                    "value": "(void *) &{}[{}]".format(data_name, enum_name)})

                    if title == "absolute" or title == "relative":
                        abre.append({"key": name.lower(),
                                     "value": "(void *) &{}[{}]".format(data_name, enum_name)})

            frmt_enum.append(self.make_enum_name(unit_name, "_LAST_ENTRY"),
                                                 "0x{0:04x}".format(idx))

            shs = LXB.SHS(entries, 0, False)
            test = shs.make_test(5, 256)
            shs.table_size_set(test[0][2])

            result_enum = frmt_enum.build()
            result_sh = shs.create("lxb_css_{}_{}_shs".format(prefix, title), rate = 1)

            result.append([result_enum, result_sh])

            print(self.shs_stat(shs))


        shs = LXB.SHS(abre, 0, False)
        test = shs.make_test(5, 256)
        shs.table_size_set(test[0][2])
        result_sh = shs.create("lxb_css_{}_{}_shs".format(prefix, "absolute_relative"), rate = 1)

        frmt_enum = LXB.FormatEnum("{}{}_t".format(self.prefix.lower(),
                                                   self.make_name(prefix)))
        name = self.make_enum_name(prefix, "_undef")
        frmt_enum.append(name, "0x{0:04x}".format(0))
        name = self.make_enum_name(prefix, "_LAST_ENTRY")
        frmt_enum.append(name, "0x{0:04x}".format(idx))

        result.insert(0, [frmt_enum.build(), []])

        result_res = res.create(rate = 1, is_const = True)

        result_enum = []
        result_sh = [''.join(result_sh)]

#        print(''.join(result_res));

        for arr in result:
            result_enum.append('\n'.join(arr[0]))
            result_enum.append('')
            result_sh.append(''.join(arr[1]))
            result_sh.append('\n\n')

#        print('\n'.join(result_enum))
#        print('\n'.join(result_sh))

        return [result_enum, result_res, result_sh]

    def make_enum_name(self, prefix, name):
        name = re.sub(r"[^a-zA-Z0-9_]", "_", name)
        prefix = re.sub(r"[^a-zA-Z0-9_]", "_", prefix)
        return "{}{}_{}".format(self.prefix, prefix.upper(), name.upper())

    def make_name(self, name):
        return re.sub("[^a-zA-Z0-9]", "_", name)

    def shs_stat(self, shs):
       return "Max deep {}; Used {} of {}".format(shs.max, shs.used, shs.table_size)

    def save(self, prefix, res, save_to, results = [], values = False):
        now = datetime.datetime.now()

        path = "{}{}".format(save_to, prefix)

        const_save_to = "{}/const.h".format(path)
        res_save_to = "{}/res.h".format(path)
        header = "lexbor/css/{}/const.h".format(prefix)

        if os.path.isdir(path) == False:
            os.mkdir(path, 0o755)

        body = ['\n'.join(res[0])]

        if values:
            for lines in results:
                body.append('\n'.join(lines))

        lxb_temp = LXB.Temp(self.const_tmp, const_save_to)

        lxb_temp.pattern_append("%%YEAR%%", str(now.year))
        lxb_temp.pattern_append("%%PREFIX%%", prefix.upper())
        lxb_temp.pattern_append("%%BODY%%", '\n\n'.join(body))

        lxb_temp.build()
        lxb_temp.save()

        print("Const saved to {}".format(const_save_to))

        # res
        lxb_temp = LXB.Temp(self.res_tmp, res_save_to)

        lxb_temp.pattern_append("%%YEAR%%", str(now.year))
        lxb_temp.pattern_append("%%PREFIX%%", prefix.upper())
        lxb_temp.pattern_append("%%HEADER%%", header.lower())
        lxb_temp.pattern_append("%%SHS_DATA%%", ''.join(res[1]))
        lxb_temp.pattern_append("%%SHS%%", ''.join(res[2]))

        lxb_temp.build()
        lxb_temp.save()

        print("Res saved to {}".format(res_save_to))

    def save_value(self, prefix, res, save_to):
        now = datetime.datetime.now()

        path = "{}{}".format(save_to, prefix)

        const_save_to = "{}/const.h".format(path)
        res_save_to = "{}/res.h".format(path)
        header = "lexbor/css/{}/const.h".format(prefix)

        if os.path.isdir(path) == False:
            os.mkdir(path, 0o755)

        body = ['\n'.join(res[0])]

        # for lines in res[1]:
        #     body.append('\n'.join(lines))

        lxb_temp = LXB.Temp(self.value_const_tmp, const_save_to)

        lxb_temp.pattern_append("%%YEAR%%", str(now.year))
        lxb_temp.pattern_append("%%PREFIX%%", prefix.upper())
        lxb_temp.pattern_append("%%BODY%%", '\n\n'.join(body))

        lxb_temp.build()
        lxb_temp.save()

        print("Const saved to {}".format(const_save_to))

        # res
        lxb_temp = LXB.Temp(self.value_res_tmp, res_save_to)

        lxb_temp.pattern_append("%%YEAR%%", str(now.year))
        lxb_temp.pattern_append("%%PREFIX%%", prefix.upper())
        lxb_temp.pattern_append("%%HEADER%%", header.lower())
        lxb_temp.pattern_append("%%SHS_DATA%%", ''.join(res[3]))
        lxb_temp.pattern_append("%%SHS%%", ''.join(res[2]))

        lxb_temp.build()
        lxb_temp.save()

        print("Res saved to {}".format(res_save_to))

    def print_cds(self, values):
        arr = list(values.keys())
        headers = []
        sources = []
        state_headers = []
        state_sources = []
        typedef = []

        for name in arr:
            origin = name.capitalize()
            name = self.make_name(name)

            headers.append("/* {}. */\n".format(origin))
            headers.append("LXB_API void *")
            headers.append("lxb_css_property_{}_create(lxb_css_memory_t *memory);\n".format(name))
            headers.append("LXB_API void *")
            headers.append("lxb_css_property_{}_destroy(lxb_css_memory_t *memory,".format(name))
            headers.append("                            void *style, bool self_destroy);\n")
            headers.append("LXB_API lxb_status_t")
            headers.append("lxb_css_property_{}_serialize(const void *style,".format(name))
            headers.append("                              lexbor_serialize_cb_f cb, void *ctx);\n")

            sources.append("/* {}. */\n".format(origin))
            sources.append("LXB_API void *")
            sources.append("lxb_css_property_{}_create(lxb_css_memory_t *memory)\n{{".format(name))
            sources.append("    return lexbor_mraw_calloc(memory->mraw, sizeof(lxb_css_property_{}_t));".format(name))
            sources.append("}\n")
            sources.append("LXB_API void *")
            sources.append("lxb_css_property_{}_destroy(lxb_css_memory_t *memory,".format(name))
            sources.append("                            void *style, bool self_destroy)\n{")
            sources.append("    return lxb_css_property__undef_destroy(memory, style, self_destroy);")
            sources.append("}\n")
            sources.append("LXB_API lxb_status_t")
            sources.append("lxb_css_property_{}_serialize(const void *style,".format(name))
            sources.append("                              lexbor_serialize_cb_f cb, void *ctx)\n{")
            sources.append("    return LXB_STATUS_ERROR;")
            sources.append("}\n")

            state_headers.append("LXB_API bool")
            state_headers.append("lxb_css_property_state_{}(lxb_css_parser_t *parser,".format(name))
            state_headers.append("                          const lxb_css_syntax_token_t *token, void *ctx);")

            state_sources.append("bool")
            state_sources.append("lxb_css_property_state_{}(lxb_css_parser_t *parser,".format(name))
            state_sources.append("                          const lxb_css_syntax_token_t *token, void *ctx)\n{")
            state_sources.append("    return lxb_css_parser_failed(parser);")
            state_sources.append("}\n")

            typedef.append("lxb_css_property_{}_t    *{};".format(name, name))

        print('\n'.join(headers))
        print("\n\n")
        print('\n'.join(sources))

        print("\n\n\n\n\n\n")

        print('\n'.join(state_headers))
        print("\n\n")
        print('\n'.join(state_sources))

        print("\n\n\n\n\n\n")

        print('\n'.join(typedef))

if __name__ == "__main__":
    ps = Pseudo()

    vals_res = ps.make_value(styles)
    ps.save_value("value", vals_res, "../../../source/lexbor/css/")

    for entry in compiles:
        print("Compile {}:".format(entry[0]))

        lw = entry[0].lower()
        res = ps.make(lw, entry[1])
        ps.save(lw, res, "../../../source/lexbor/css/", vals_res[1], entry[2])

    res = ps.make_units("unit", units)
    ps.save("unit", res, "../../../source/lexbor/css/")

    print("Done")

    # ps.print_cds(styles)
