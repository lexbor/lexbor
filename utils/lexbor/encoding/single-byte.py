
import sys, re, os

# Find and append run script run dir to module search path
ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../lexbor/".format(ABS_PATH))

import LXB

class SingleByte:
    buffer_size = 128
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
        buf = []
        externs = []
        hash_externs = []
        hash_buf = []
        hash_sizes = []
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
            res = self.make_flat_index(flat_name, values)
            hash_index = self.make_hash_index(flat_name, values)

            buf.append(''.join(res))
            externs.append('{};'.format(self.make_extern_name(flat_name)))
            hash_buf.append(''.join(hash_index[0]))
            hash_sizes.append(hash_index[1])
            hash_externs.append(hash_index[2])

            const_name = "{}{}".format(self.const_prefix, flat_name.upper())
            self.save_test(self.make_var_name(flat_name), const_name, flat_name)

            fo.close()

        externs.append('')
        externs += hash_externs

        return [buf, externs, hash_buf, hash_sizes]

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

        return 'LXB_EXTERN const {} {}'.format(self.flat_index_typename, var_name)

    def make_flat_index(self, name, values):
        res = LXB.Res(self.flat_index_typename, 
                self.make_var_name(name) + '[{}]'.format(self.buffer_size), False, None, 'LXB_API')

        for idx in range(0, self.buffer_size):
            if idx in values:
                entries = values[idx]

                res.append('{{(lxb_char_t *) "{}", {}, {}}}'.format(toHex(entries[2].decode('utf-8')), entries[3], entries[1].decode('utf-8')))
                res.append('/* {} */'.format(entries[4].decode('utf-8')), is_comment = True)
            else:
                res.append('{NULL, 0, LXB_ENCODING_DECODE_ERROR}')
                res.append('/* Not defined */', is_comment = True)

        buf = res.create()

        return buf

    def make_hash_index(self, name, values):
        name = self.make_hash_name(name)

        hash_key = LXB.HashKey(412, name, 'LXB_API')

        for idx in range(0, self.buffer_size):
            if idx in values:
                entries = values[idx]
                key_id = entries[1].decode('utf-8')

                hash_key.append(key_id, '(void *) {}'.format(idx + 0x80))

        return hash_key.create(rate = 1)

    def save_res(self, temp_file_h, save_to_h, temp_file_c, save_to_c):
        res = self.make()

        lxb_temp = LXB.Temp(temp_file_h, save_to_h)
        lxb_temp.pattern_append("%%EXTERNS%%", '\n'.join(res[1]))
        lxb_temp.pattern_append("%%SIZES%%", '\n'.join(res[3]))
        lxb_temp.build()
        lxb_temp.save()

        lxb_temp = LXB.Temp(temp_file_c, save_to_c)
        lxb_temp.pattern_append("%%INDEX%%", '\n\n'.join(res[0]))
        lxb_temp.pattern_append("%%HASH%%", '\n\n'.join(res[2]))
        lxb_temp.build()
        lxb_temp.save()

    def save_test(self, data, const_name, name):
        save_to_c = os.path.join(self.save_to, name + ".c")

        lxb_temp = LXB.Temp(self.temp_file_c, save_to_c)
        lxb_temp.pattern_append("%%DATA%%", data)
        lxb_temp.pattern_append("%%CONST_NAME%%", const_name)
        lxb_temp.pattern_append("%%NAME%%", name)
        lxb_temp.build()
        lxb_temp.save()

def toHex(s):
    lst = []

    for ch in bytes(s, 'utf-8'):
        hv = hex(ch).replace('0x', '\\\\x')
        lst.append(hv)

    return ''.join(lst)

if __name__ == "__main__":

    sb = SingleByte("single-byte", "tmp/single_byte_test.c",
                    "../../../test/lexbor/encoding/single")

    sb.save_res("tmp/single.h", "../../../source/lexbor/encoding/single.h",
                "tmp/single.c", "../../../source/lexbor/encoding/single.c")

    # yn = input("Print encode declarations? (y - yes, n - no): ")
    # if yn.lower() == 'y':
    #     print('asd')
