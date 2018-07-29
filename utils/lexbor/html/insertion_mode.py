
import json
import sys, re, os

# Find and append run script run dir to module search path
ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../lexbor/".format(ABS_PATH))

import LXB
import tags

class InsertionMode:
    prefix = "lxb_html_tree_insertion_mode"
    anything_else = "anything_else"
    anything_else_closed = "anything_else_closed"

    def __init__(self, json_tags, json_interface):
        self.tags = tags.Tags(json_tags, json_interface)

    def make(self, name, temp_file_h, temp_file_c, save_to, save_to_c):
        tags = self.tags.enum_list
        res = LXB.Res("lxb_html_tree_insertion_mode_f", "{}_{}_res".format(self.prefix, name), False)
        res_closed = LXB.Res("lxb_html_tree_insertion_mode_f", "{}_{}_closed_res".format(self.prefix, name), False)

        for entry in tags:
            res.append("{}_{}_{} /* {} */".format(self.prefix, name, self.anything_else, entry["c_name"]))
            res_closed.append("{}_{}_{} /* {} */".format(self.prefix, name, self.anything_else_closed, entry["c_name"]))

        res_data = res.create(1, True)
        res_closed_data = res_closed.create(1, True)

        lxb_temp = LXB.Temp(temp_file_h, save_to)

        lxb_temp.pattern_append("%%NAME%%", name.upper())
        lxb_temp.pattern_append("%%HASH%%", self.tags.enum_hash_ifdef())
        lxb_temp.pattern_append("%%BODY-OPEN%%", ''.join(res_data))
        lxb_temp.pattern_append("%%BODY-CLOSED%%", ''.join(res_closed_data))

        lxb_temp.build()
        lxb_temp.save()

        lxb_temp = LXB.Temp(temp_file_c, save_to_c)

        lxb_temp.pattern_append("%%NAME%%", name)

        lxb_temp.build()
        lxb_temp.save()

if __name__ == "__main__":
    imode = InsertionMode("data/tags.json", "data/interfaces.json")
    imode.make("in_body", "tmp/insertion_mode.h", "tmp/insertion_mode.c", "in_body_res.h", "in_body.c")
    imode.make("in_table", "tmp/insertion_mode.h", "tmp/insertion_mode.c", "in_table_res.h", "in_table.c")
    imode.make("in_caption", "tmp/insertion_mode.h", "tmp/insertion_mode.c", "in_caption_res.h", "in_caption.c")
    imode.make("in_column_group", "tmp/insertion_mode.h", "tmp/insertion_mode.c", "in_column_group_res.h", "in_column_group.c")
    imode.make("in_table_body", "tmp/insertion_mode.h", "tmp/insertion_mode.c", "in_table_body_res.h", "in_table_body.c")
    imode.make("in_row", "tmp/insertion_mode.h", "tmp/insertion_mode.c", "in_row_res.h", "in_row.c")
    imode.make("in_cell", "tmp/insertion_mode.h", "tmp/insertion_mode.c", "in_cell_res.h", "in_cell.c")
    imode.make("in_select", "tmp/insertion_mode.h", "tmp/insertion_mode.c", "in_select_res.h", "in_select.c")
    imode.make("in_select_in_table", "tmp/insertion_mode.h", "tmp/insertion_mode.c", "in_select_in_table_res.h", "in_select_in_table.c")
    imode.make("in_template", "tmp/insertion_mode.h", "tmp/insertion_mode.c", "in_template_res.h", "in_template.c")
    imode.make("foreign_content", "tmp/insertion_mode.h", "tmp/insertion_mode.c", "foreign_content_res.h", "foreign_content.c")
