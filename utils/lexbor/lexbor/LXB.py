
import json
import sys, re, os

class Temp:
    def __init__(self, filename, filename_to_save):
        if os.path.exists(filename) == False:
            sys.stderr.write('File not found:' + filename)
            return None

        self.buffer = []
        self.patterns = {}
        self.fn_to_read = filename
        self.fn_to_save = filename_to_save

    def pattern_append(self, name, body):
        if name in self.patterns:
            self.patterns[name].append(body)
        else:
            self.patterns[name] = [body]

    def pattern_remove(self, name):
        self.patterns[name].pop(name, None)

    def build(self):
        fh = open(self.fn_to_read, 'rt', encoding="utf-8")

        for line in fh:
            for name in self.patterns:
                line = re.sub(name, '\n'.join(self.patterns[name]), line)
            self.buffer.append(line)
        fh.close()

    def save(self, with_build = False):
        if with_build:
            self.build()

        w_fh = open(self.fn_to_save, 'w', encoding="utf-8")
        w_fh.write(''.join(self.buffer))
        w_fh.close()

class Switch:
    def __init__(self):
        self.buffer = []

    def append(self, key_id, value, break_val = 'break;', is_comment = False):
        self.buffer.append([int(key_id, 0), value, break_val, is_comment])

    def create(self, var, default, default_break_val = 'break;', on_line = False, data_before = None):
            result = []
            data = self.buffer

            if data_before:
                result.append("{}\n\n".format(data_before))

            result.append("switch ({}) \n{{\n    ".format(var))

            if on_line:
                for entry in data:
                    if entry[3] == False:
                        result.append("case {}: {} {}\n    ".format(entry[0], entry[1], entry[2]))
                    else:
                        result.append("{}".format(entry[0]))

                result.append("default: {} {}\n".format(default, default_break_val))

            else:
                for entry in data:
                    if entry[3] == False:
                        tmp = "case {}:\n        {}\n        {}\n    "
                        result.append((tmp).format(entry[0], entry[1], entry[2]))
                    else:
                        result.append("{}".format(entry[0]))

                tmp = "default:\n        {}\n        {}\n"
                result.append(tmp.format(default, default_break_val))

            result.append("}")

            return result

class HashKey:
    def __init__(self, max_table_size, name, prefix = ''):
        self.max_table_size = max_table_size
        self.struct_name = 'lexbor_shs_hash_t'
        self.name = name
        self.prefix = prefix

        self.buffer = []

    def hash_id(self, hash_id):
        return hash_id

    def append(self, key_id, value):
        self.buffer.append([self.hash_id(int(key_id, 0)), value])

    def create(self, terminate_value = '{0, NULL, 0}', rate = 2, is_const = True, data_before = None):
        test = self.test(int(self.max_table_size / 1.2), int(self.max_table_size * 1.2))

        rate_dn = rate - 1
        buffer = self.buffer
        table_size = test['size']
        table = [None] * (table_size + 1)

        self.table_size = table_size
        self.max_deep = test['max']

        for entry in buffer:
            idx = (entry[0] % table_size) + 1

            if table[idx] == None:
                table[idx] = [entry[0], entry[1], 0]
                continue

            while table[idx][2] != 0:
                idx = table[idx][2]

            table.append([entry[0], entry[1], 0])
            table[idx][2] = len(table) - 1

        result = []

        if data_before:
            result.append("{}\n\n".format(data_before))

        result.append("/* Table size: {}; Max deep: {} */\n".format(self.table_size, self.max_deep))

        prefix = ''
        if self.prefix != '':
            prefix = self.prefix + ' '

        extern_name = "{} {} {}[{}]".format(("const" if is_const else ""),
                                            self.struct_name, self.name, len(table))

        var_name = "{}{}".format(prefix, extern_name)

        result.append("{} = \n{{\n    ".format(var_name))

        result.append("{},".format(terminate_value))

        for idx in range(1, len(table) - 1):
            entry = table[idx]

            if entry:
                result.append("{{{}, {}, {}}},".format(entry[0], entry[1], entry[2]))
            else:
                result.append("{0, NULL, 0},")

            if int(idx) % rate == rate_dn:
                result.append("\n    ")
            else:
                result.append(" ")

        if len(table):
            entry = table[-1]
            if entry:
                result.append("{{{}, {}, {}}}\n".format(entry[0], entry[1], entry[2]))
            else:
                result.append("{0, NULL, 0}\n")

        result.append("};")

        return [result, '#define {}_SIZE {}'.format(self.name.upper(), self.table_size),
                'LXB_EXTERN ' + extern_name + ';', self.table_size]

    def test(self, begin, end):
        result = []
        buffer = self.buffer

        for table_size in range(begin, end):
            table = [0] * table_size

            for entry in buffer:
                table[ entry[0] % table_size ] += 1

            stat = {'empty': 0, 'max': 0, 'size': table_size}

            for val in table:
                if val == 0:
                    stat['empty'] += 1
                elif val > stat['max']:
                    stat['max'] = val

            result.append(stat)

        result.sort(key=lambda x: x['max'])

        best_result = []
        for entry in result:
            if entry['max'] != result[0]['max']:
                break

            best_result.append(entry)

        best_result.sort(key=lambda x: x['size'])

        print(best_result[0])

        return best_result[0]


class Res:
    def __init__(self, struct_name, name, include_def, size = None, prefix = 'static'):
        if isinstance(size, list):
            self.size = size
        else:
            if size != None:
                self.size = [size]
            else:
                self.size = []

        self.prefix = prefix
        self.name = name
        self.struct_name = struct_name
        self.include_def = include_def

        self.buffer = []

    def append(self, data, is_comment = False):
        self.buffer.append([data, is_comment])

    def create(self, rate = 2, is_const = True, data_before = None):
        result = []
        rate_dn = rate - 1
        sizes = []
        data = self.buffer

        if self.include_def:
            result.append("#ifdef {}\n".format(self.name.upper()))
            result.append("#ifndef {}_ENABLED\n".format(self.name.upper()))
            result.append("#define {}_ENABLED\n".format(self.name.upper()))

        if data_before:
            result.append("{}\n\n".format(data_before))

        for size in self.size:
            sizes.append("[{}]".format(size))

        prefix = ''
        if self.prefix != '':
            prefix = self.prefix + ' '

        result.append("{}{} {} {}{} = \n{{\n    ".format(prefix, ("const" if is_const else ""),
            self.struct_name, self.name, ("".join(sizes) if len(sizes) != 0 else "")))

        for idx in range(0, len(data) - 1):
            if data[idx][1] == False:
                result.append("{},".format(data[idx][0]))
            else:
                result.append("{}".format(data[idx][0]))

            if int(idx) % rate == rate_dn:
                result.append("\n    ")
            else:
                result.append(" ")

        if len(data):
            result.append("{}\n".format(data[-1][0]))

        result.append("};")

        if self.include_def:
            result.append("\n#endif /* {}_ENABLED */\n".format(self.name.upper()))
            result.append("#endif /* {} */".format(self.name.upper()))

        return result

class SHS:
    def __init__(self, data, table_size, include_def = False, prefix = 'static'):
        self.include_def = include_def

        self.data = {}
        self.idx = 0
    
        self.table = []
        self.table_size = 0
        self.real_table_size = 0
    
        self.unused = []
        self.unused_pos = 0
        
        self.max = 0
        self.used = 0

        self.prefix = prefix

        self.init(data, table_size)

    def init(self, data, table_size):
        self.data = data
        self.table_size = table_size
        self.real_table_size = table_size + 1

    def table_size_set(self, table_size):
        self.table_size = table_size
        self.real_table_size = table_size + 1

        return self.table_size

    def make_id(self, key, table_size):
        key = key.lower()
        return ((((ord(key[:1]) * ord(key[-1:])) * ord(key[:1])) + len(key)) % table_size) + 1

    def create(self, data_name, rate = 2):
        rate_dn = rate - 1
        result = []

        self.make()
        lst = self.build()

        if self.include_def:
            result.append("#ifdef {}\n".format(data_name.upper()))
            result.append("#ifndef {}_ENABLED\n".format(data_name.upper()))
            result.append("#define {}_ENABLED\n".format(data_name.upper()))

        prefix = ''
        if self.prefix != '':
            prefix = self.prefix + ' '

        result.append("{}const lexbor_shs_entry_t {}[{}] = \n{{\n    ".format(prefix, data_name, self.idx + 1))

        for key in range(0, self.idx):
            if key not in lst:
                 result.append("{{{}, {}, {}, {}}}".format("NULL", "NULL", 0, 0))
            else:
                key_val = "\"{}\"".format(lst[key][0]) if lst[key][0] != None else "NULL"
                result.append("{{{}, {}, {}, {}}}".format(key_val, lst[key][1],
                                                              lst[key][2], lst[key][3]))

            result.append(", ")

            if int(key) % rate == rate_dn:
                result.append("\n    ")

        key = self.idx

        if key not in lst:
            result.append("{{{}, {}, {}, {}}}".format("NULL", "NULL", 0, 0))
        else:
            result.append("{{\"{}\", {}, {}, {}}}".format(lst[key][0], lst[key][1],
                                                          lst[key][2], lst[key][3]))

        result.append("\n}};".format(data_name))

        if self.include_def:
            result.append("\n#endif /* {}_ENABLED */\n".format(data_name.upper()))
            result.append("#endif /* {} */".format(data_name.upper()))

        return result

    def make(self):
        for idx in range(0, self.real_table_size):
            self.table.append([])

        for entry in self.data:
            idx = self.make_id(entry['key'], self.table_size)

            if len(self.table[idx]) == 0:
                self.used += 1

            self.table[idx].append(entry)
            self.table[idx].sort(key = lambda entr: len(entr['key']))

            if len(self.table[idx]) > self.max:
                self.max = len(self.table[idx])

    def make_test(self, idx_from, idx_to):
        stat = []

        for i in range(idx_from, idx_to):
            max = 0
            used = 0
            result = {}

            for entry in self.data:
                idx = self.make_id(entry['key'], i)
    
                if idx not in result:
                    used += 1
                    result[idx] = 0
    
                result[idx] += 1
    
                if result[idx] > max:
                    max = result[idx]
            
            stat.append([max, used, i])

        stat.sort(key = lambda entr: entr[0])

        return stat

    def build(self):
        result = {}
        unused = []

        result[0] = [None, "NULL", self.table_size, 0, True]

        for key in range(1, self.real_table_size):
            if len(self.table[key]) == 0:
                unused.append(key)
            else:
                entry = self.table[key].pop(0)

                result[key] = [
                    entry["key"],
                    entry["value"],
                    len(entry["key"]),
                    0,
                    True
                ]

        self.idx = self.table_size
        self.unused = unused
        self.unused_pos = 0

        for key in range(1, self.real_table_size):
            if len(self.table[key]) == 0:
                continue

            last_entry = result[key]

            for entry in self.table[key]:
                last_entry[3] = self.get_next_free_pos()

                new_entry = [
                    entry["key"],
                    entry["value"],
                    len(entry["key"]),
                    0,
                    False
                ]

                result[ last_entry[3] ] = new_entry
                last_entry = new_entry

        return result

    def get_next_free_pos(self):
        if len(self.unused) > self.unused_pos:
            idx = self.unused[self.unused_pos]
            self.unused_pos += 1
            return idx

        self.idx += 1
        return self.idx
    

class FormatEnum:
    def __init__(self, enum_name, value_prefix = ""):
        self.buffer = []
        self.len_max = 0
        self.enum_name = ""
        self.value_prefix = ""
        self.enum_name = enum_name
        self.value_prefix = value_prefix

    def append(self, key, value):
        if key != None:
            if len(key) > self.len_max:
                self.len_max = len(key)

        self.buffer.append([key, value])

    def build(self, join_val = "= "):
        if len(self.buffer) == 0:
            return []

        buffer = []
        last_entry = self.buffer.pop()

        buffer.append("typedef enum {")
        
        for entry in self.buffer:
            if entry[0] == None:
                buffer.append(entry[1])
                continue

            self.make_line(entry, buffer, join_val, ",")

        self.make_line(last_entry, buffer, join_val)

        buffer.append("}")
        buffer.append("{0};".format(self.enum_name))

        return buffer

    def make_line(self, entry, buffer, join_val, postfix = ""):
        buffer.append("    {0}{1}{2}{3}{4}{5}".format(self.value_prefix, entry[0],
                                                      ''.ljust((self.len_max + 1) - len(entry[0])),
                                                      join_val, entry[1], postfix))
