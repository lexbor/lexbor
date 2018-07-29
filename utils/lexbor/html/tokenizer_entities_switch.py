
import json, sys
import re, os

class LXBTemp:
    fh = None
    buffer = []
    patterns = {}
    fn_to_save = ""

    def __init__(self, filename, filename_to_save):
        if os.path.exists(filename) == False:
            sys.stderr.write('File not found:' + filename)
            return None

        self.fh = open(filename, 'rt', encoding="utf-8")
        self.fn_to_save = filename_to_save

    def pattern_append(self, name, body):
        if name in self.patterns:
            self.patterns[name].append(body)
        else:
            self.patterns[name] = [body]

    def pattern_remove(self, name):
        self.patterns[name].pop(name, None)

    def build(self):
        for line in self.fh:
            for name in self.patterns:
                self.buffer.append(re.sub(name,
                                          '\n'.join(self.patterns[name]),
                                          line))

    def save(self, with_build = False):
        if with_build:
            self.build()

        w_fh = open(self.fn_to_save, 'w', encoding="utf-8")
        w_fh.write(''.join(self.buffer))
        w_fh.close()

    def finish(self):
        self.fh.close()


def entities_switch(tmp_name, save_name, json_filename):
    index = entities_switch_load(json_filename)
    res = entities_switch_result("lxb_html_tokenizer_res_entities_sbst", index)

    header = res[0]
    result = res[1]

    lxb_temp = LXBTemp(tmp_name, save_name)
    lxb_temp.pattern_append("%%BODY%%", ''.join(result))

    # lxb_temp.build()
    # lxb_temp.save()

    print(''.join(header))
    print("")
    print(''.join(result))


def entities_switch_load(json_filename):
    jdata = json.load(open(json_filename))
    index = {}

    for name in jdata:
        entities_switch_create_layer(name, jdata[name], index)

    return index

def entities_switch_save(json_filename):
    jdata = json.load(open(json_filename))
    index = {}

    for name in jdata:
        entities_switch_create_layer(name, jdata[name], index)

    return index

def entities_switch_create_layer(name, entry, index):
    chars = list(name)
    last = len(chars)

    for pos in range(1, last):
        char = chars[pos]

        if char not in index:
            index[char] = {'next': {}, 'values': []}

        if pos == (last - 1):
            index[char]['values'].append(entry);

        index = index[char]['next']


def entities_switch_result(func_name, index):
    result = []
    header = []
    func_array = [func_name]

    stack = [[list(index.keys()), index]]
    stack[-1][0].sort()

    while True:
        if len(index) != 0:
            entities_switch_create_function(''.join(func_array), index, header, result)

        while len(stack[-1][0]) == 0:
            if len(stack) == 1:
                return [header, result]

            stack.pop()
            func_array.pop()

            index = stack[-1][1]

        name = stack[-1][0].pop()
        index = index[name]['next']

        func_array.append(name)

        stack.append([list(index.keys()), index])
        stack[-1][0].sort()
    
    return [header, result]


def entities_switch_create_function(func_name, index, header, result):
    
    header.append("void *\n")
    header.append("{}(lxb_char_t key, void *value);\n".format(func_name))

    result.append("void *\n")
    result.append("{}(lxb_char_t key, void *value)\n".format(func_name))
    result.append("{\n")

    result.append("\tswitch (key) {\n")

    for name in index:
        # result.append("\t\t/* {} */\n".format(name))
        if len(index[name]['values']):
            result.append("\t\tcase 0x{0:02x}: value = (void *) {1};".format(ord(name), 1))
        else:
            result.append("\t\tcase 0x{0:02x}:".format(ord(name)))

        if len(index[name]['next']) != 0:
            result.append(" return {}{};\n".format(func_name, name))
        else:
            result.append(" return NULL;\n")

    result.append("\t\tdefault: return NULL;\n")
    result.append("\t}\n")
    result.append("}\n\n")


if __name__ == "__main__":
    entities_switch("tmp/tokenizer_res.h",
                 "../../../source/lexbor/html/tokenizer_res.h",
                 "data/entities.json");
