
import sys, re, os, datetime

# Find and append run script run dir to module search path
ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../lexbor/".format(ABS_PATH))

import LXB

# CSS StyleSheet entries

at_rules = {
    "media": {},
    "namespace": {}
}

min_max = ["auto", "min-content", "max-content", "_length", "_percentage", "_number", "_angle"]
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

styles = {
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
        ]
    },

    # https://drafts.csswg.org/css-sizing-3/

    "box-sizing": {"values": ["content-box", "border-box"]},

    "width": {"values": min_max},
    "height": {"values": min_max},
    "min-width": {"values": min_max},
    "min-height": {"values": min_max},
    "max-width": {"values": min_max},
    "max-height": {"values": min_max},

    # https://drafts.csswg.org/css-box-3/

    "margin": {"values": margin_padding},
    "margin-top": {"values": margin_padding},
    "margin-right": {"values": margin_padding},
    "margin-bottom": {"values": margin_padding},
    "margin-left": {"values": margin_padding},

    "padding": {"values": margin_padding},
    "padding-top": {"values": margin_padding},
    "padding-right": {"values": margin_padding},
    "padding-bottom": {"values": margin_padding},
    "padding-left": {"values": margin_padding},

    # https://drafts.csswg.org/css-backgrounds-3/

    "border": {"values": border_values},
    "border-top": {"values": border_values},
    "border-right": {"values": border_values},
    "border-bottom": {"values": border_values},
    "border-left": {"values": border_values},

    "color": {"values": color_values, "hide": True}
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

unit_angel = {
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
    ["angel", unit_angel],
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
                           "     {}, {}, {}}}".format(
                               enum_name, func_name, create, destroy, serialize
                            ))
                continue
            elif name == "_custom":
                res.append("{{(lxb_char_t *) \"#сustom\", 7, {}, {},\n"
                           "     {}, {}, {}}}".format(
                               enum_name, func_name, create, destroy, serialize
                            ))
                continue
            elif "hide" not in values[origin] or values[origin]["hide"] == False:
                res.append("{{(lxb_char_t *) \"{}\", {}, {}, {},\n"
                           "     {}, {}, {}}}".format(origin, len(name),
                                enum_name, func_name, create, destroy, serialize
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
