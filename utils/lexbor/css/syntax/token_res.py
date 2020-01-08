
import os, sys

# Find and append run script run dir to module search path
ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../../lexbor/".format(ABS_PATH))

import LXB

enum_tokens = {
    "undefined": "LXB_CSS_SYNTAX_TOKEN_UNDEF",
    "ident": "LXB_CSS_SYNTAX_TOKEN_IDENT",
    "function": "LXB_CSS_SYNTAX_TOKEN_FUNCTION",
    "at-keyword": "LXB_CSS_SYNTAX_TOKEN_AT_KEYWORD",
    "hash": "LXB_CSS_SYNTAX_TOKEN_HASH",
    "string": "LXB_CSS_SYNTAX_TOKEN_STRING",
    "bad-string": "LXB_CSS_SYNTAX_TOKEN_BAD_STRING",
    "url": "LXB_CSS_SYNTAX_TOKEN_URL",
    "bad-url": "LXB_CSS_SYNTAX_TOKEN_BAD_URL",
    "delim": "LXB_CSS_SYNTAX_TOKEN_DELIM",
    "number": "LXB_CSS_SYNTAX_TOKEN_NUMBER",
    "percentage": "LXB_CSS_SYNTAX_TOKEN_PERCENTAGE",
    "dimension": "LXB_CSS_SYNTAX_TOKEN_DIMENSION",
    "whitespace": "LXB_CSS_SYNTAX_TOKEN_WHITESPACE",
    "CDO": "LXB_CSS_SYNTAX_TOKEN_CDO",
    "CDC": "LXB_CSS_SYNTAX_TOKEN_CDC",
    "colon": "LXB_CSS_SYNTAX_TOKEN_COLON",
    "semicolon": "LXB_CSS_SYNTAX_TOKEN_SEMICOLON",
    "comma": "LXB_CSS_SYNTAX_TOKEN_COMMA",
    "left-square-bracket": "LXB_CSS_SYNTAX_TOKEN_LS_BRACKET",
    "right-square-bracket": "LXB_CSS_SYNTAX_TOKEN_RS_BRACKET",
    "left-parenthesis": "LXB_CSS_SYNTAX_TOKEN_L_PARENTHESIS",
    "right-parenthesis": "LXB_CSS_SYNTAX_TOKEN_R_PARENTHESIS",
    "left-curly-bracket": "LXB_CSS_SYNTAX_TOKEN_LC_BRACKET",
    "right-curly-bracket": "LXB_CSS_SYNTAX_TOKEN_RC_BRACKET",
    "comment": "LXB_CSS_SYNTAX_TOKEN_COMMENT",
    "end-of-file": "LXB_CSS_SYNTAX_TOKEN__EOF"
}

def create(name, enum_tokens):
    tlist = []

    for (key, value) in enum_tokens.items():
        tlist.append({'key': key.lower(), 'value': "(void *) " + value})

    shs = LXB.SHS(tlist, 0, True)

    test = shs.make_test(5, 128)
    shs.table_size_set(test[0][2])

    return shs.create(name)

if __name__ == "__main__":
    print("".join(create("lxb_css_syntax_token_res_name_shs_map", enum_tokens)))
