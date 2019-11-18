
import sys, re, os

# Find and append run script run dir to module search path
ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../lexbor/".format(ABS_PATH))

import LXB

class SingleByte:
    var_name_prefix = 'lxb_encoding_single_index_'
    hash_name_prefix = 'lxb_encoding_single_hash_'
    flat_index_typename = 'lxb_encoding_single_index_t'
    const_prefix = "LXB_ENCODING_"

    def __init__(self, dir_path, temp_file_c, save_to, silent = False):
        if not os.path.isdir(dir_path):
            raise Exception('Directory "{}" not exit'.format(dir_path))

        self.dir_path = dir_path
        self.silent = silent
        self.temp_file_c = temp_file_c
        self.save_to = save_to

    def make(self):
        dir_path = self.dir_path

        for f in sorted(os.listdir(dir_path)):
            f_path = os.path.join(dir_path, f)
            if not os.path.isfile(f_path) or f[0] == '.':
                continue

            print('File: {}'.format(f_path))

            values = {}
            fo = open(f_path, "rb")

            for lineno, line in enumerate(fo):
                line = line.rstrip()
                if not line or line[:1] == b'#':
                    continue

                entries = line.split(b'\t')
                captions = entries[-1].split(b' ', maxsplit=1)

                if not captions[0]:
                    raise Exception('Failed to get chars variant on line {}'.format(lineno))

                entries.append(captions[0])
                entries.append(captions[1] if captions[1] else b'')

                entries[2] = entries[3]
                entries[3] = len(entries[2])

                values[ int(entries[0].decode('utf-8')) ] = entries

            flat_name = self.make_name(f)

            const_name = "{}{}".format(self.const_prefix, flat_name.upper())
            self.save_test(self.make_var_name(flat_name), const_name, flat_name)

            fo.close()

    def make_name(self, filename):
        name = re.sub("[^a-zA-Z0-9]", "_", filename)
        name = re.sub("^index_", "", name)
        name = re.sub("_txt$", "", name)

        return name

    def make_var_name(self, name):
        return '{}{}'.format(self.var_name_prefix, name)

    def make_hash_name(self, name):
        return '{}{}'.format(self.hash_name_prefix, name)

    def make_extern_name(self, name):
        var_name = self.make_var_name(name) + '[{}]'.format(self.buffer_size)

        return 'extern const {} {}'.format(self.flat_index_typename, var_name)

    def save_test(self, data, const_name, name):
        save_to_c = os.path.join(self.save_to, name + ".c")

        lxb_temp = LXB.Temp(self.temp_file_c, save_to_c)
        lxb_temp.pattern_append("%%DATA%%", data)
        lxb_temp.pattern_append("%%CONST_NAME%%", const_name)
        lxb_temp.pattern_append("%%NAME%%", name)
        lxb_temp.build()
        lxb_temp.save()

if __name__ == "__main__":

    sb = SingleByte("single-byte", "tmp/buffer_single_byte_test.c",
                    "../../../test/lexbor/encoding/buffer")
