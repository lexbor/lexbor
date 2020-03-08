
import sys, re, os

# Find and append run script run dir to module search path
ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../lexbor/".format(ABS_PATH))

import LXB

class MultiByte:
    var_name_prefix = 'lxb_encoding_multi_index_'
    hash_name_prefix = 'lxb_encoding_multi_hash_'
    flat_index_typename = 'lxb_encoding_multi_index_t'

    def __init__(self, dir_path, temp_file_h, temp_file_c, save_to, silent = False):
        if not os.path.isdir(dir_path):
            raise Exception('Directory "{}" not exit'.format(dir_path))

        self.dir_path = dir_path
        self.silent = silent
        self.temp_file_h = temp_file_h
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
            if not os.path.isfile(f_path):
                continue

            print('File: {}'.format(f_path))

            idx = 0
            values = {'buffer_size': idx, 'max_size': 0}

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

                idx = int(entries[0].decode('utf-8'))
                values[idx] = entries

                if values['max_size'] < idx:
                    values['max_size'] = idx

            values['buffer_len'] = len(values)

            flat_name = self.make_name(f)
            res = self.make_flat_index(flat_name, values)
            hash_index = self.make_hash_index(flat_name, values)

            buf.append(''.join(res))
            externs.append('{};'.format(self.make_extern_name(flat_name, self.buffer_size(values))))
            hash_buf.append(''.join(hash_index[0]))
            hash_sizes.append(hash_index[1])
            hash_externs.append(hash_index[2])

            self.save_res(flat_name, ''.join(res), ''.join(hash_index[0]))

            fo.close()

        externs.append('')
        externs += hash_externs

        save_to_h = os.path.join(self.save_to, "multi.h")

        lxb_temp = LXB.Temp(self.temp_file_h, save_to_h)
        lxb_temp.pattern_append("%%EXTERNS%%", '\n'.join(externs))
        lxb_temp.pattern_append("%%SIZES%%", '\n'.join(hash_sizes))
        lxb_temp.build()
        lxb_temp.save()

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

    def make_extern_name(self, name, buffer_size):
        var_name = self.make_var_name(name) + '[{}]'.format(buffer_size)

        return 'LXB_EXTERN const {} {}'.format(self.flat_index_typename, var_name)

    def buffer_size(self, values):
        return values['max_size'] + 1

    def make_flat_index(self, name, values):
        buffer_size = self.buffer_size(values)

        print("Flat buffer size:", buffer_size)

        res = LXB.Res(self.flat_index_typename, 
                self.make_var_name(name) + '[{}]'.format(buffer_size), False, None, 'LXB_API')

        for idx in range(0, buffer_size):
            if idx in values:
                entries = values[idx]

                res.append('{{(lxb_char_t *) "{}", {}, {}}}'.format(toHex(entries[2].decode('utf-8')), 
                                                                          entries[3], entries[1].decode('utf-8')))
                res.append('/* {} */'.format(entries[4].decode('utf-8')), is_comment = True)
            else:
                res.append('{NULL, 0, LXB_ENCODING_ERROR_CODEPOINT}')
                res.append('/* Not defined */', is_comment = True)

        buf = res.create()

        return buf

    def make_hash_index(self, name, values):
        buffer_size = self.buffer_size(values)
        name = self.make_hash_name(name)

        hash_key = LXB.HashKey(buffer_size, name, 'LXB_API')

        for idx in range(0, buffer_size):
            if idx in values:
                entries = values[idx]

                key_id = entries[1].decode('utf-8')

                hash_key.append(key_id, '(void *) {}'.format(idx))

        return hash_key.create(rate = 1)

    def save_res(self, filename, buf, hash_buf):
        save_to_c = os.path.join(self.save_to, filename + ".c")

        print("Save to:", save_to_c)

        lxb_temp = LXB.Temp(self.temp_file_c, save_to_c)
        lxb_temp.pattern_append("%%INDEX%%", buf)
        lxb_temp.pattern_append("%%HASH%%", hash_buf)
        lxb_temp.build()
        lxb_temp.save()

def toHex(s):
    lst = []

    for ch in bytes(s, 'utf-8'):
        hv = hex(ch).replace('0x', '\\\\x')
        lst.append(hv)

    return ''.join(lst)

if __name__ == "__main__":
    sb = MultiByte("multi-byte", "tmp/multi.h", "tmp/multi.c", 
                   "../../../source/lexbor/encoding")
    sb.make()
