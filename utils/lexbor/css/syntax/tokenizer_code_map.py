import os, sys

ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../../".format(ABS_PATH))

import lexbor.res

def make_ascii_map():
    entries = []
    ascii_map = lexbor.res.ascii_names_map

    for code in range(0, 128):
        if ascii_map[code][1]:
            entries.append("NULL, /* 0x%s; '%s'; %s */" 
                % (format(code, '02X'), *ascii_map[code]))
        else:
            entries.append("NULL, /* 0x%s; '%s' */" 
                % (format(code, '02X'), ascii_map[code][0]))

    for code in range(128, 255):
        entries.append("NULL, /* 0x%s */" % format(code, 'X'))

    entries.append("NULL  /* 0x%s */" % format(code + 1, 'X'))

    map_list = []

    map_list.append("static const lxb_css_syntax_tokenizer_state_f")
    map_list.append("lxb_css_syntax_state_res_map[256] =")
    map_list.append("{")
    map_list.append('    ' + '\n    '.join(entries))
    map_list.append("};")

    return '\n'.join(map_list)

if __name__ == "__main__":
    print(make_ascii_map())