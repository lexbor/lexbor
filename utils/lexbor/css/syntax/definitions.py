import os, sys

ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../../".format(ABS_PATH))

name_start_code_point = "0x01"
name_code_point = "0x02"

import lexbor.LXB

def_before = "#define LXB_CSS_SYNTAX_RES_NAME_START " + name_start_code_point

def name():
    res = lexbor.LXB.Res("lxb_char_t", "lxb_css_syntax_res_name_map", True, 256)

    for code in range(0, 128):
        # U+0061 LATIN SMALL LETTER A (a) and U+007A LATIN SMALL LETTER Z (z)
        if code >= 0x61 and code <= 0x7A:
            res.append(name_start_code_point)

        # U+0041 LATIN CAPITAL LETTER A (A) and U+005A LATIN CAPITAL LETTER Z (Z)
        elif code >= 0x41 and code <= 0x5A:
            res.append(name_start_code_point)

        # U+0030 DIGIT ZERO (0) and U+0039 DIGIT NINE (9)
        elif code >= 0x30 and code <= 0x39:
            res.append(name_code_point)

        # U+005F LOW LINE (_)
        elif code == 0x5F:
            res.append(name_start_code_point)

        # U+002D HYPHEN-MINUS (-)
        elif code == 0x2D:
            res.append(name_code_point)

        # all other
        else:
            res.append("0x00")

    for code in range(128, 255):
        res.append(name_start_code_point)

    map_list = res.create(10, True, def_before)

    return ''.join(map_list)

if __name__ == "__main__":
    print(name())