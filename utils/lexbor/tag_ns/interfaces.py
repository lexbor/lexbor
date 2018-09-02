
import json
import sys, re, os

# Find and append run script run dir to module search path
ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../lexbor/".format(ABS_PATH))

import LXB
import tags

class Interfaces:
    prefix = "lxb"

    def __init__(self, json_filename):
        self.obj = json.load(open(json_filename))
        self.interfaces = self.obj["interfaces"];

    def make(self, temp_file_h, temp_file_c, save_path):
        interfaces = self.interfaces
        typedefs_list = []
        interfaces_list = []

        for type in interfaces:
            path = os.path.abspath(save_path)
            full_path = "{}/{}".format(path, type.lower())

            if not os.path.exists(full_path):
                os.makedirs(full_path)

            interface = interfaces[type]
            names = list(interface.keys())
            names.sort()

            for name in names:
                inter_obj = interface[name]
                save_to_h = "{}/{}.h".format(full_path, inter_obj["c_name"])
                save_to_c = "{}/{}.c".format(full_path, inter_obj["c_name"])

                # Make and Save *.h
                lxb_temp_h = LXB.Temp(temp_file_h, save_to_h)

                if type == "DOM":
                    lxb_temp_h.pattern_append("%%mraw%%", "document->mraw")
                else:
                    lxb_temp_h.pattern_append("%%mraw%%", "document->mem->mraw")

                lxb_temp_h.pattern_append("%%M_PREFIX%%", type.upper())
                lxb_temp_h.pattern_append("%%M_NAME%%", inter_obj["c_name"].upper())
                lxb_temp_h.pattern_append("%%prefix%%", type.lower())
                lxb_temp_h.pattern_append("%%name%%", inter_obj["c_name"])

                typedef_name = self.make_type_name(type, inter_obj)
                lxb_temp_h.pattern_append("%%type%%", typedef_name)

                def_name = inter_obj["c_name"]
                def_name = def_name.replace('_element', '')

                interfaces_list.append("#define {}_{}_interface_{}(obj) (({} *) obj)".format(
                    self.prefix, type.lower(), def_name, typedef_name)
                )

                vars = self.make_vars(type, inter_obj)
                lxb_temp_h.pattern_append("%%vars%%", "\n    ".join(vars))

                includes = self.make_inherit_includes(type, inter_obj)
                lxb_temp_h.pattern_append("%%includes%%", "\n".join(includes) + "\n")

                lxb_temp_h.build()
                lxb_temp_h.save()

                # Make and Save *.c
                lxb_temp_c = LXB.Temp(temp_file_c, save_to_c)

                if type == "DOM":
                    lxb_temp_c.pattern_append("%%mraw%%", "document->mraw")
                else:
                    lxb_temp_c.pattern_append("%%mraw%%", "document->mem->mraw")

                lxb_temp_c.pattern_append("%%prefix%%", type.lower())
                lxb_temp_c.pattern_append("%%name%%", inter_obj["c_name"])
                lxb_temp_c.pattern_append("%%type%%", typedef_name)

                lxb_temp_c.build()
                lxb_temp_c.save()

                # Print typedefs
                struct = "struct {}_{}_{}".format(self.prefix, type.lower(), inter_obj["c_name"])
                typedefs_list.append( "typedef {} {};".format(struct, typedef_name) )
        
        print("\n".join(typedefs_list))
        print("\n".join(interfaces_list))

    def make_vars(self, type, inter_obj):
        vars = []
        interfaces = self.interfaces

        if "inherit" in inter_obj:
            inherit = inter_obj["inherit"]
            inherit_type = type

            if "inherit_type" in inter_obj:
                inherit_type = inter_obj["inherit_type"]
            else:
                inherit_type = type

            inherit_obj = interfaces[inherit_type][inherit]
            typedef_name = self.make_type_name(inherit_type, inherit_obj)

            def_name = "{} {};".format(typedef_name, inherit_obj["c_name"])
            vars.append(def_name)

        return vars

    def make_type_name(self, type, inter_obj):
        return "{}_{}_{}_t".format(self.prefix, type.lower(), inter_obj["c_name"])

    def make_function_create_name(self, type, name):
        interfaces = self.interfaces
        inter_obj = interfaces[type][name]

        return "{}_{}_{}_interface_create".format(self.prefix, type.lower(), inter_obj["c_name"])

    def make_function_destroy_name(self, type, name):
        interfaces = self.interfaces
        inter_obj = interfaces[type][name]

        return "{}_{}_{}_interface_destroy".format(self.prefix, type.lower(), inter_obj["c_name"])

    def make_inherit_includes(self, type, inter_obj):
        includes = []
        interfaces = self.interfaces

        if "inherit" in inter_obj:
            inherit = inter_obj["inherit"]
            inherit_type = type

            if "inherit_type" in inter_obj:
                inherit_type = inter_obj["inherit_type"]
            else:
                inherit_type = type

            inherit_obj = interfaces[inherit_type][inherit]

            include = "#include \"lexbor/{}/interfaces/{}.h\"".format(inherit_type.lower(), inherit_obj["c_name"])
            includes.append(include)

        return includes

    def make_include(self, type, name):
        interfaces = self.interfaces
        inter_obj = interfaces[type][name]

        include = "#include \"lexbor/{}/interfaces/{}.h\"".format(type.lower(), inter_obj["c_name"])

    def make_includes(self):
        includes = []
        interfaces = self.interfaces

        for type in interfaces:
            interface = interfaces[type]
            names = list(interface.keys())
            names.sort()

            for name in names:
                inter_obj = interface[name]

                includes.append("#include \"lexbor/{}/interfaces/{}.h\"".format(type.lower(), inter_obj["c_name"]))

        return includes

if __name__ == "__main__":
    interfaces = Interfaces("data/interfaces.json")
    interfaces.make("tmp/interface.h", "tmp/interface.c", ".")

