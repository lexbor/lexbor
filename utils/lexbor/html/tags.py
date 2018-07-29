
import json
import sys, re, os
import hashlib

# Find and append run script run dir to module search path
ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../lexbor/".format(ABS_PATH))

import LXB
import interfaces

def convert_to_c(tag_name):
    return re.sub(r"[^a-zA-Z0-9_]", "_", tag_name)

def computeMD5hash(my_string):
    m = hashlib.md5()
    m.update(my_string.encode('utf-8'))
    return m.hexdigest()

class Tags:
    tag_prefix = "lxb_html_tag"
    tag_res_data = "lxb_html_tag_res_data"
    ns_prefix = "lxb_html_ns"
    ns_res_data = "lxb_html_ns_res_data"
    cat_prefix = "lxb_html_tag_category"
    cat_empty = "LXB_HTML_TAG_CATEGORY__UNDEF"
    creation_interface = "lxb_html_interface_creation_f"

    def __init__(self, json_tags, json_interfaces):
        self.interfaces = interfaces.Interfaces(json_interfaces)

        self.tags = json.load(open(json_tags))
        self.make_tags()

    def make_tags(self):
        shs = {}
        shs_list = []

        enum = {}
        enum_list = []
        
        ns_shs_list = []

        # namespace
        self.ns = self.tags["namespaces"]
        self.ns_list = list(self.ns.keys())
        self.ns_list.sort(key = lambda entr: (100000 if "sort" not in self.ns[entr] else self.ns[entr]["sort"], entr))

        interfaces = self.interfaces

        for idx in range(0, len(self.ns_list)):
            self.ns[ self.ns_list[idx] ]["c_name"] = "{}_{}".format(self.ns_prefix.upper(),
                                                                    convert_to_c(self.ns_list[idx]).upper())
            self.ns[ self.ns_list[idx] ]["id"] = idx

            ns_shs_list.append({"key": self.ns_list[idx], "value": "&{}[{}]".format(self.ns_res_data, idx)})

        self.ns_hash = self.ns_make_hash(self.ns_list)
        self.ns_shs_list = ns_shs_list

        # shs data and enum
        data = self.tags["data"]

        for ns in data:
            for tag in data[ns]:
                if "to" not in data[ns][tag]:
                    enum_name = "{}_{}".format(self.tag_prefix.upper(),
                                               convert_to_c(tag).upper())
                else:
                    enum_name = "{}_{}".format(self.tag_prefix.upper(),
                                               convert_to_c(data[ns][tag]["to"]).upper())

                shs[tag] = {"key": tag}

                if enum_name not in enum:
                    enum[enum_name] = {"ns": [], "name": tag, "c_name": enum_name, "interface": []}

                    for idx in self.ns_list:
                        default = self.ns[idx]["default"]
                        interface = interfaces.make_function_create_name(default["interface_type"], default["interface"])

                        enum[enum_name]["ns"].append([])
                        enum[enum_name]["interface"].append(interface)


                ns_id = self.ns[ns]["id"]
                cat = data[ns][tag]["cat"]

                for cat_name in cat:
                    enum[enum_name]["ns"][ns_id].append(
                        "{}_{}".format(self.cat_prefix.upper(), convert_to_c(cat_name).upper())
                    )

                    enum[enum_name]["ns"][ns_id].sort()

                interface = data[ns][tag]["interface"]
                interface_type = data[ns][tag]["interface_type"]

                enum[enum_name]["interface"][ns_id] = interfaces.make_function_create_name(interface_type, interface);

                if "sort" in data[ns][tag]:
                    enum[enum_name]["sort"] = data[ns][tag]["sort"]
                elif "sort" not in enum[enum_name]:
                    enum[enum_name]["sort"] = 1000000

        for key in shs:
            shs_list.append(shs[key])

        for key in enum:
            enum_list.append(enum[key])

            for ns_entry in enum[key]["ns"]:
                ns_entry.sort()

        shs_list.sort(key = lambda entr: entr["key"])
        enum_list.sort(key = lambda entr: (entr["sort"], entr["c_name"]))

        for idx in range(0, len(enum_list)):
            shs[ enum_list[idx]["name"] ]["value"] = "&{}[{}]".format(self.tag_res_data, idx)

        self.shs_list = shs_list

        self.enum = enum
        self.enum_list = enum_list
        self.enum_hash = self.enum_make_hash(enum_list)

        self.ns_last_entry_name = convert_to_c("{}__LAST_ENTRY".format(self.ns_prefix)).upper()
        self.tag_last_entry_name = convert_to_c("{}__LAST_ENTRY".format(self.tag_prefix)).upper()

        return shs_list

    def tag_data_create(self):
        result = []
        res = LXB.Res("lxb_html_tag_data_t", self.tag_res_data, True, self.tag_last_entry_name)

        for entry in self.enum_list:
            ns = ["            "]
            interface = ["            "]

            for idx in range(0, len(entry["ns"]) - 1):
                if len(entry["ns"][idx]) == 0:
                    ns.append(self.cat_empty)
                    ns.append(", ")
                else:
                    ns.append("\n            |".join(entry["ns"][idx]))
                    if len(entry["ns"][idx]) > 1 and idx % 2 != 1:
                        ns.append(",\n            ")
                    else:
                        ns.append(",")

                interface.append("({}) {},".format(self.creation_interface, entry["interface"][idx]))
                interface.append("\n            ")

                if idx % 2 == 1:
                    ns.append("\n            ")

            if len(entry["ns"][-1]) == 0:
                ns.append(self.cat_empty)
            else:
                ns.append("\n|".join(entry["ns"][-1]))

            interface.append("({}) {}".format(self.creation_interface, entry["interface"][-1]))

            res.append("{{(const lxb_char_t *) \"{}\", {}, {},\n        {{\n{}\n        }},\n        {{\n{}\n        }}\n    }}"
                       .format(entry["name"], len(entry["name"]), entry["c_name"], "".join(ns), "".join(interface)))

        return res.create(1, False, "\n".join(self.interfaces.make_includes()))

    def ns_data_create(self):
        result = []
        res = LXB.Res("lxb_html_ns_data_t", self.ns_res_data, True, self.ns_last_entry_name)

        for name in self.ns_list:
            res.append("{{\"{0}\", {1}, \"{2}\", {3}, {4}}}".format(
                self.ns[name]["name"],
                len(self.ns[name]["name"]),
                self.ns[name]["link"],
                len(self.ns[name]["link"]),
                self.ns[name]["c_name"]
            ))

        return res.create(1, False)

    def ns_make_hash(self, ns_list):
        result = []

        for name in ns_list:
            result.append(self.ns[name]["c_name"])

        return computeMD5hash("".join(result)).upper()

    def ns_hash_ifdef(self):
        result = []
        result.append("#ifdef LXB_HTML_NS_CONST_VERSION")
        result.append("#ifndef LXB_HTML_NS_CONST_VERSION_{}".format(self.ns_hash))
        result.append("#error Mismatched namespaces version! See \"lexbor/html/ns_const.h\".".format(self.ns_hash))
        result.append("#endif /* LXB_HTML_NS_CONST_VERSION_{} */".format(self.ns_hash))
        result.append("#else")
        result.append("#error You need to include \"lexbor/html/ns_const.h\".".format(self.ns_hash))
        result.append("#endif /* LXB_HTML_NS_CONST_VERSION */".format(self.ns_hash))

        return "\n".join(result)

    def ns_create(self):
        result = []

        frmt_ns = LXB.FormatEnum("{}_id_enum_t".format(self.ns_prefix))

        for idx in range(0, len(self.ns_list)):
            name = self.ns_list[idx]
            frmt_ns.append(self.ns[name]["c_name"], "0x{0:02x}".format(idx))

        frmt_ns.append(self.ns_last_entry_name, "0x{0:02x}".format(idx + 1))

        return frmt_ns.build()

    def ns_save(self, data, temp_file, save_to):
        lxb_temp = LXB.Temp(temp_file, save_to)
        lxb_temp.pattern_append("%%HASH%%", self.ns_hash)
        lxb_temp.pattern_append("%%BODY%%", "\n".join(data))

        lxb_temp.build()
        lxb_temp.save()

        print("\n".join(data))
        print("Save to {}".format(save_to))
        print("Done")

    def ns_create_and_save(self, temp_file, save_to):
        result = self.ns_create()
        self.ns_save(result, temp_file, save_to)

    def ns_shs_create(self, name):
        self.ns_shs = LXB.SHS(self.ns_shs_list, 0, True)

        test = self.ns_shs.make_test(5, 128)
        self.ns_shs.table_size_set(test[0][2])

        return self.ns_shs.create(name)

    def ns_shs_save(self, data, temp_file, save_to):
        lxb_temp = LXB.Temp(temp_file, save_to)

        lxb_temp.pattern_append("%%CHECK_NS_VERSION%%", self.ns_hash_ifdef())
        lxb_temp.pattern_append("%%NS_DATA%%", ''.join(self.ns_data_create()))
        lxb_temp.pattern_append("%%SHS_DATA%%", ''.join(data))

        lxb_temp.build()
        lxb_temp.save()

        print("".join(data))
        print(self.shs_stat(self.ns_shs))
        print("Save to {}".format(save_to))
        print("Done")

    def ns_shs_create_and_save(self, name, temp_file, save_to):
        result = self.ns_shs_create(name)
        self.ns_shs_save(result, temp_file, save_to)

    def ns_test_name_create(self):
        result = []
        ns = self.ns

        for name in self.ns_list:
            result.append("    entry = lxb_html_ns_data_by_name((const lxb_char_t *) \"{}\", {});".format(name, len(name)))
            result.append("    test_ne(entry, NULL); test_eq_str(entry->name, \"{}\");".format(ns[name]["name"]))

        return result

    def ns_test_create_and_save(self, temp_file, save_to):
        lxb_temp = LXB.Temp(temp_file, save_to)

        lxb_temp.pattern_append("%%TEST_NAMES%%", "\n".join(self.ns_test_name_create()))

        lxb_temp.build()
        lxb_temp.save()

        print("Test saved to {}".format(save_to))

    def enum_make_hash(self, enum_list):
        result = []

        for entry in enum_list:
            result.append(entry["name"])
            result.append(entry["c_name"])

            for ns_entry in entry["ns"]:
                result.append("".join(ns_entry))

        return computeMD5hash("".join(result)).upper()

    def enum_hash_ifdef(self):
        result = []
        result.append("#ifdef LXB_HTML_TAG_CONST_VERSION")
        result.append("#ifndef LXB_HTML_TAG_CONST_VERSION_{}".format(self.enum_hash))
        result.append("#error Mismatched tags version! See \"lexbor/html/tag_const.h\".".format(self.enum_hash))
        result.append("#endif /* LXB_HTML_TAG_CONST_VERSION_{} */".format(self.enum_hash))
        result.append("#else")
        result.append("#error You need to include \"lexbor/html/tag_const.h\".".format(self.enum_hash))
        result.append("#endif /* LXB_HTML_TAG_CONST_VERSION */".format(self.enum_hash))

        return "\n".join(result)

    def enum_create(self):
        result = []

        frmt_enum = LXB.FormatEnum("{}_id_enum_t".format(self.tag_prefix))

        for idx in range(0, len(self.enum_list)):
            frmt_enum.append(self.enum_list[idx]["c_name"], "0x{0:04x}".format(idx))

        frmt_enum.append(self.tag_last_entry_name, "0x{0:04x}".format(idx + 1))

        return frmt_enum.build()

    def enum_save(self, data, temp_file, save_to):
        lxb_temp = LXB.Temp(temp_file, save_to)
        lxb_temp.pattern_append("%%HASH%%", self.enum_hash)
        lxb_temp.pattern_append("%%BODY%%", "\n".join(data))

        lxb_temp.build()
        lxb_temp.save()

        print("\n".join(data))
        print("Save to {}".format(save_to))
        print("Done")

    def enum_create_and_save(self, temp_file, save_to):
        result = self.enum_create()
        self.enum_save(result, temp_file, save_to)

    def shs_create(self, name):
        self.shs = LXB.SHS(self.shs_list, 0, True)

        test = self.shs.make_test(128, 1024)
        self.shs.table_size_set(test[0][2])

        return self.shs.create(name)

    def shs_save(self, data, temp_file, save_to):
        lxb_temp = LXB.Temp(temp_file, save_to)

        lxb_temp.pattern_append("%%CHECK_TAG_VERSION%%", self.enum_hash_ifdef())
        lxb_temp.pattern_append("%%CHECK_NS_VERSION%%", self.ns_hash_ifdef())
        lxb_temp.pattern_append("%%TAG_DATA%%", ''.join(self.tag_data_create()))
        lxb_temp.pattern_append("%%SHS_DATA%%", ''.join(data))

        lxb_temp.build()
        lxb_temp.save()

        print("".join(data))
        print(self.shs_stat(self.shs))
        print("Save to {}".format(save_to))
        print("Done")

    def shs_create_and_save(self, name, temp_file, save_to):
        result = self.shs_create(name)
        self.shs_save(result, temp_file, save_to)

    def tag_test_name_create(self):
        result = []

        for entry in self.shs_list:
            result.append("    entry = lxb_html_tag_data_by_name(tag_heap, (const lxb_char_t *) \"{}\", {});".format(entry["key"], len(entry["key"])))
            result.append("    test_ne(entry, NULL); test_eq_u_str(entry->name, (const lxb_char_t *) \"{}\");".format(entry["key"]))

        return result

    def tag_test_create_and_save(self, temp_file, save_to):
        lxb_temp = LXB.Temp(temp_file, save_to)

        lxb_temp.pattern_append("%%TEST_NAMES%%", "\n".join(self.tag_test_name_create()))

        lxb_temp.build()
        lxb_temp.save()

        print("Test saved to {}".format(save_to))

    def shs_stat(self, shs):
       return "Max deep {}; Used {} of {}".format(shs.max, shs.used, shs.table_size)

if __name__ == "__main__":
    tags = Tags("data/tags.json", "data/interfaces.json")

    # print(tags.enum_hash_ifdef())
    tags.ns_create_and_save("tmp/ns_const.h", "../../../source/lexbor/html/ns_const.h")
    tags.ns_shs_create_and_save("lxb_html_ns_res_shs_data", "tmp/ns_res.h", "../../../source/lexbor/html/ns_res.h")
    tags.enum_create_and_save("tmp/tag_const.h", "../../../source/lexbor/html/tag_const.h")
    tags.shs_create_and_save("lxb_html_tag_res_shs_data", "tmp/tag_res.h", "../../../source/lexbor/html/tag_res.h")
    tags.ns_test_create_and_save("tmp/test/ns_res.c", "../../../test/lexbor/html/ns_res.c")
    tags.tag_test_create_and_save("tmp/test/tag_res.c", "../../../test/lexbor/html/tag_res.c")
