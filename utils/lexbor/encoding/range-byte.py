
import sys, re, os

# Find and append run script run dir to module search path
ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../lexbor/".format(ABS_PATH))

import LXB

class RangeByte:
    range_name_prefix = 'lxb_encoding_range_index_'
    range_index_typename = 'lxb_encoding_range_index_t'

    def __init__(self, dir_path, silent = False):
        if not os.path.isdir(dir_path):
            raise Exception('Directory "{}" not exit'.format(dir_path))

        self.dir_path = dir_path
        self.silent = silent

    def make(self):
        buf = []
        externs = []
        sizes = []
        dir_path = self.dir_path

        for f in sorted(os.listdir(dir_path)):
            f_path = os.path.join(dir_path, f)
            if not os.path.isfile(f_path):
                continue

            print('File: {}'.format(f_path))

            values = []

            fo = open(f_path, "rb")

            for lineno, line in enumerate(fo):
                line = line.strip()
                if not line or line[:1] == b'#':
                    continue

                entries = line.split(b'\t')

                if not entries[0] or not entries[0]:
                    raise Exception('Failed to get chars variant on line {}'.format(lineno))

                values.append(entries)

            name = self.make_name(f)
            res = self.make_range_index(name, values)

            buf.append(''.join(res))
            externs.append('{};'.format(self.make_extern_name(name, len(values))))
            sizes.append(self.make_range_size(name, len(values)))

            fo.close()

        return [buf, externs, sizes]

    def make_name(self, filename):
        name = re.sub("[^a-zA-Z0-9]", "_", filename)
        name = re.sub("^index_", "", name)
        name = re.sub("_txt$", "", name)
        name = re.sub("_ranges$", "", name)

        return name

    def make_range_name(self, name):
        return '{}{}'.format(self.range_name_prefix, name)

    def make_extern_name(self, name, buffer_size):
        var_name = self.make_range_name(name) + '[{}]'.format(buffer_size)

        return 'LXB_EXTERN const {} {}'.format(self.range_index_typename, var_name)

    def make_range_size(self, name, size):
        return '#define {}{}_SIZE {}'.format(self.range_name_prefix.upper(), name.upper(), size)

    def make_range_index(self, name, values):
        res = LXB.Res(self.range_index_typename, 
                self.make_range_name(name) + '[{}]'.format(len(values)), False, None, 'LXB_API')

        for entry in values:
            res.append('{{{}, {}}}'.format(entry[0].decode('utf-8'), entry[1].decode('utf-8')))

        buf = res.create(rate = 4)

        return buf

    def save_res(self, temp_file_h, save_to_h, temp_file_c, save_to_c):
        res = self.make()

        lxb_temp = LXB.Temp(temp_file_h, save_to_h)
        lxb_temp.pattern_append("%%EXTERNS%%", '\n'.join(res[1]))
        lxb_temp.pattern_append("%%SIZES%%", '\n'.join(res[2]))
        lxb_temp.build()
        lxb_temp.save()

        lxb_temp = LXB.Temp(temp_file_c, save_to_c)
        lxb_temp.pattern_append("%%INDEX%%", '\n\n'.join(res[0]))
        lxb_temp.build()
        lxb_temp.save()

def toHex(s):
    lst = []

    for ch in bytes(s, 'utf-8'):
        hv = hex(ch).replace('0x', '\\\\x')
        lst.append(hv)

    return ''.join(lst)

if __name__ == "__main__":
    sb = RangeByte("ranges")

    sb.save_res("tmp/range.h", "../../../source/lexbor/encoding/range.h",
                "tmp/range.c", "../../../source/lexbor/encoding/range.c")
