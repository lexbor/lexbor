
import sys, re, os

# Find and append run script run dir to module search path
ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../../lexbor/".format(ABS_PATH))

import LXB

pseudo_default_state_success = "lxb_css_selectors_state_success"

pseudo_classes = [
    # https://drafts.csswg.org/selectors-4/#overview
    "warning", "any-link", "link", "visited", "local-link", "target",
    "target-within", "scope", "current", "past", "future", "active",
    "hover", "focus", "focus-within", "focus-visible", "enabled", 
    "disabled", "read-write", "read-only", "placeholder-shown",
    "default", "checked", "indeterminate", "valid", "invalid",
    "in-range", "out-of-range", "required", "optional", "blank",
    "user-invalid", "root", "empty", "first-child", "last-child",
    "only-child", "first-of-type", "last-of-type", "only-of-type",

    # https://fullscreen.spec.whatwg.org/#index
    "fullscreen"
]

pseudo_class_functions_dict = {
    # [callback end function, can empty]
    #
    # https://drafts.csswg.org/selectors-4/#overview
    "not": [pseudo_default_state_success, False, "CLOSE"],
    "is": ["lxb_css_selectors_state_forgiving", False, "CLOSE"],
    "where": ["lxb_css_selectors_state_forgiving", False, "CLOSE"],
    "has": ["lxb_css_selectors_state_forgiving_relative", False, "DESCENDANT"],
    "dir": [pseudo_default_state_success, False, "CLOSE"],
    "lang": [pseudo_default_state_success, False, "CLOSE"],
    "current": [pseudo_default_state_success, False, "CLOSE"],
    "nth-child": [pseudo_default_state_success, False, "CLOSE"],
    "nth-last-child": [pseudo_default_state_success, False, "CLOSE"],
    "nth-of-type": [pseudo_default_state_success, False, "CLOSE"],
    "nth-last-of-type": [pseudo_default_state_success, False, "CLOSE"],
    "nth-col": [pseudo_default_state_success, False, "CLOSE"],
    "nth-last-col": [pseudo_default_state_success, False, "CLOSE"]
}

pseudo_class_functions = list(pseudo_class_functions_dict.keys())

pseudo_elements = [
    # https://drafts.csswg.org/css-pseudo-4/#index
    "after", "before", "first-letter", "first-line", "grammar-error",
    "inactive-selection", "marker", "placeholder", "selection",
    "spelling-error", "target-text",

    # https://fullscreen.spec.whatwg.org/#index
    "backdrop"
]

pseudo_element_functions_dict = {
}

pseudo_element_functions = list(pseudo_element_functions_dict.keys())

pseudo_classes.sort()
pseudo_classes.insert(0, "_undef")

pseudo_class_functions.sort()
pseudo_class_functions.insert(0, "_undef")

pseudo_elements.sort()
pseudo_elements.insert(0, "_undef")

pseudo_element_functions.sort()
pseudo_element_functions.insert(0, "_undef")

class Pseudo:
    prefix = "LXB_CSS_SELECTOR_"

    def make(self, pseude_name, arr, names = None):
        idx = 0
        state = ""
        entries = []
        type_data_name = ""
        state_declaration = []
        state_bodies = []
        data_name = "lxb_css_selectors_pseudo_data_{}".format(pseude_name)

        make_func = names != None

        if make_func == True:
            type_data_name = "lxb_css_selectors_pseudo_data_func_t"
        else:
            type_data_name = "lxb_css_selectors_pseudo_data_t"

        res = LXB.Res(type_data_name, data_name, False,
                      self.make_enum_name(pseude_name, "_LAST_ENTRY"))

        frmt_enum = LXB.FormatEnum("{}{}_id_t".format(self.prefix.lower(),
                                                      self.make_name(pseude_name)))

        for name in arr:
            enum_name = self.make_enum_name(pseude_name, name)

            frmt_enum.append(enum_name, "0x{0:04x}".format(idx))
            idx += 1

            if make_func == True:
                state = "lxb_css_selectors_state_{}_{}".format(self.make_name(pseude_name),
                                                               self.make_name(name))
                state_declaration.append(self.make_state_declaration(state))
                state_bodies.append(self.make_state_body(state))

                success = pseudo_default_state_success
                can_empty = False
                combinator = "CLOSE"

                if name in names:
                    success = names[name][0]
                    can_empty = names[name][1]
                    combinator = names[name][2]

                if name == "_undef":
                    res.append("{{(lxb_char_t *) \"#undef\", 6, {},\n     {}, {},"
                               " {},\n     {}}}".format(enum_name,
                                        success, state, str(can_empty).lower(),
                                        self.make_combinator_name(combinator)))
                    continue
                else:
                    res.append("{{(lxb_char_t *) \"{}\", {}, {},\n     {}, {},"
                               " {},\n     {}}}".format(name, len(name),
                                        enum_name, success, state, str(can_empty).lower(),
                                        self.make_combinator_name(combinator)))
            else:
                if name == "_undef":
                    res.append("{{(lxb_char_t *) \"#undef\", 6, {}}}".format(enum_name))
                    continue
                else:
                    res.append("{{(lxb_char_t *) \"{}\", {}, {}}}".format(name, len(name),
                                                                          enum_name))

            entries.append({"key": name,
                            "value": "(void *) &{}[{}]".format(data_name,  enum_name)})

        frmt_enum.append(self.make_enum_name(pseude_name, "_LAST_ENTRY"),
                                             "0x{0:04x}".format(idx))

        shs = LXB.SHS(entries, 0, False)
        test = shs.make_test(5, 256)
        shs.table_size_set(test[0][2])

        result_enum = frmt_enum.build()
        result_res = res.create(rate = 1, is_const = True)
        result_sh = shs.create("lxb_css_selectors_{}_shs".format(pseude_name), rate = 1)

        print(self.shs_stat(shs))

        return [result_enum, result_res, result_sh, state_declaration, state_bodies]

    def make_combinator_name(self, name):
        name = re.sub(r"[^a-zA-Z0-9_]", "_", name)
        return "{}COMBINATOR_{}".format(self.prefix, name.upper())

    def make_enum_name(self, pseudo_name, name):
        name = re.sub(r"[^a-zA-Z0-9_]", "_", name)
        pseudo_name = re.sub(r"[^a-zA-Z0-9_]", "_", pseudo_name)
        return "{}{}_{}".format(self.prefix, pseudo_name.upper(), name.upper())

    def make_name(self, name):
        return re.sub("[^a-zA-Z0-9]", "_", name)

    def shs_stat(self, shs):
       return "Max deep {}; Used {} of {}".format(shs.max, shs.used, shs.table_size)

    def make_state_declaration(self, name):
        return ("bool\n{}(lxb_css_parser_t *parser,\n"
                "    lxb_css_syntax_token_t *token, void *ctx)").format(name)

    def make_state_body(self, name):
        return "{}\n{{\n    return true;\n}}".format(self.make_state_declaration(name))

    def save(self, const_temp_file, const_save_to, res_temp_file, res_save_to):
        res_class = ps.make("pseudo_class", pseudo_classes)
        res_class_function = ps.make("pseudo_class_function", pseudo_class_functions, pseudo_class_functions_dict)
        res_element = ps.make("pseudo_element", pseudo_elements)
        res_element_function = ps.make("pseudo_element_function", pseudo_element_functions, pseudo_element_functions_dict)

        # const
        lxb_temp = LXB.Temp(const_temp_file, const_save_to)

        lxb_temp.pattern_append("%%BODY%%",
                                "{}\n\n{}\n\n{}\n\n{}".format('\n'.join(res_class[0]),
                                                        '\n'.join(res_class_function[0]),
                                                        '\n'.join(res_element[0]),
                                                        '\n'.join(res_element_function[0])))

        lxb_temp.build()
        lxb_temp.save()

        print("Const saved to {}".format(const_save_to))

        # res
        lxb_temp = LXB.Temp(res_temp_file, res_save_to)

        lxb_temp.pattern_append("%%SHS_DATA%%",
                                "{}\n\n{}\n\n{}\n\n{}".format(''.join(res_class[1]),
                                                        ''.join(res_class_function[1]),
                                                        ''.join(res_element[1]),
                                                        ''.join(res_element_function[1])))
        lxb_temp.pattern_append("%%SHS%%",
                                "{}\n\n{}\n\n{}\n\n{}".format(''.join(res_class[2]),
                                                        ''.join(res_class_function[2]),
                                                        ''.join(res_element[2]),
                                                        ''.join(res_element_function[2])))

        lxb_temp.build()
        lxb_temp.save()

        print("Res saved to {}".format(res_save_to))
        print("Done")

        yn = input("Print declarations? (y - yes, n - no): ")
        if yn.lower() == 'y':
            print('LXB_API ', ';\n\nLXB_API '.join(res_class_function[3]), ';', sep='')
            print('LXB_API ', ';\n\nLXB_API '.join(res_element_function[3]), ';', sep='')
            print('\n\n'.join(res_class_function[4]))
            print('\n\n'.join(res_element_function[4]))

if __name__ == "__main__":
    ps = Pseudo()

    ps.save("tmp/const.h", "../../../../source/lexbor/css/selectors/pseudo_const.h",
            "tmp/res.h", "../../../../source/lexbor/css/selectors/pseudo_res.h")
