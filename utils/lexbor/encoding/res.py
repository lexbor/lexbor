
import json
import sys, re, os

# Find and append run script run dir to module search path
ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../lexbor/".format(ABS_PATH))

import LXB

class Encoding:
    prefix = 'LXB_ENCODING_'
    const_typename = 'lxb_encoding_t'
    data_typename = 'lxb_encoding_data_t'
    last_entry_name = 'LAST_ENTRY'

    def __init__(self, json_file, silent = False):
        self.codes = json.load(open(json_file))
        self.codes_shs = []
        self.silent = silent

        strict = []
        index_name = {}

        for entry in self.codes:
            encodings = entry['encodings']

            for encode in encodings:
                if encode['name'] in index_name:
                    raise KeyError('Key {} is exist'.format(encode['name']))

                index_name[encode['name']] = 1

                strict.append(encode)

        strict.sort(key=lambda x: x['name'])

        self.strict = strict

    def make_shs(self, shs_name, data_name):
        idx = 2 # Skip first three data position (DEFAULT, AUTO, UNDEFINED)
        tlist = []

        for encode in self.strict:
            idx += 1

            for name in encode['labels']:
                value = '(void *) &{}[{}]'.format(data_name, idx)

                tlist.append({'key': name.lower(), 'value': value})

        shs = LXB.SHS(tlist, 0, False, 'LXB_API')

        test = shs.make_test(20, 2048)
        shs.table_size_set(test[0][2])

        if not self.silent:
            print('Table size: ' + str(test[0][2]))
            print('Max deep: ' + str(test[0][0]))

        buf = shs.create(shs_name)

        if not self.silent:
            print(''.join(buf))
        
        self.shs = shs

        return buf

    def make_name(self, name):
        patern = re.sub("[^a-zA-Z0-9]", "_", name)

        return "{}{}".format(self.prefix, patern)

    def make_name_lower(self, name):
        return self.make_name(name).lower()

    def make_name_upper(self, name):
        return self.make_name(name).upper()

    def make_function_name(self, name, method):
        patern = re.sub("[^a-zA-Z0-9]", "_", name)

        return "{}{}_{}".format(self.prefix, method, patern).lower()

    def make_const(self):
        idx = 0
        fconst = LXB.FormatEnum(self.const_typename)

        fconst.append(self.make_name_upper("DEFAULT"), "0x{0:02x}".format(idx))
        fconst.append(self.make_name_upper("AUTO"), "0x{0:02x}".format(idx + 1))
        fconst.append(self.make_name_upper("UNDEFINED"), "0x{0:02x}".format(idx + 2))

        idx += 2

        for encode in self.strict:
            idx += 1
            name = encode['name']
            fconst.append(self.make_name_upper(name), "0x{0:02x}".format(idx))

        fconst.append(self.make_name_upper(self.last_entry_name), "0x{0:02x}".format(idx + 1))

        buf = fconst.build()

        if not self.silent:
            print('\n'.join(buf))

        return buf

    def make_data(self, data_name):
        res = LXB.Res(self.data_typename, data_name 
                        + '[{}]'.format(self.make_name_upper(self.last_entry_name)), 
                        False, None, 'LXB_API')

        res.append('{{{}, {}, {},\n     {}, {}, (lxb_char_t *) "DEFAULT"}}'.format(
                self.make_name_upper('DEFAULT'),
                self.make_function_name("DEFAULT", 'encode'),
                self.make_function_name("DEFAULT", 'decode'),
                self.make_function_name("DEFAULT_SINGLE", 'encode'),
                self.make_function_name("DEFAULT_SINGLE", 'decode')))

        res.append('{{{}, {}, {},\n     {}, {}, (lxb_char_t *) "AUTO"}}'.format(
                self.make_name_upper('AUTO'),
                self.make_function_name("AUTO", 'encode'),
                self.make_function_name("AUTO", 'decode'),
                self.make_function_name("AUTO_SINGLE", 'encode'),
                self.make_function_name("AUTO_SINGLE", 'decode')))

        res.append('{{{}, {}, {},\n     {}, {}, (lxb_char_t *) "UNDEFINED"}}'.format(
                self.make_name_upper('UNDEFINED'),
                self.make_function_name("UNDEFINED", 'encode'),
                self.make_function_name("UNDEFINED", 'decode'),
                self.make_function_name("UNDEFINED_SINGLE", 'encode'),
                self.make_function_name("UNDEFINED_SINGLE", 'decode')))

        for encode in self.strict:
            name = encode['name']
            value = '{{{}, {}, {},\n     {}, {}, (lxb_char_t *) "{}"}}'.format(
                self.make_name_upper(name),
                self.make_function_name(name, 'encode'),
                self.make_function_name(name, 'decode'),
                self.make_function_name(name + '_single', 'encode'),
                self.make_function_name(name + '_single', 'decode'), name)

            res.append(value)

        buf = res.create(rate = 1)

        if not self.silent:
            print(''.join(buf))

        return buf

    def save_const(self, temp_file, save_to):
        buf = self.make_const()
        lxb_temp = LXB.Temp(temp_file, save_to)

        lxb_temp.pattern_append("%%CONST%%", '\n'.join(buf))

        lxb_temp.build()
        lxb_temp.save()

    def save_res(self, temp_file_h, save_to_h, temp_file_c, save_to_c):
        data = self.make_data('lxb_encoding_res_map')
        shs = self.make_shs('lxb_encoding_res_shs_entities', 'lxb_encoding_res_map')

        lxb_temp = LXB.Temp(temp_file_h, save_to_h)
        lxb_temp.pattern_append("%%MAX%%", self.make_name_upper(self.last_entry_name))
        lxb_temp.pattern_append("%%SHS_MAX%%", "{}".format(self.shs.idx + 1))
        lxb_temp.build()
        lxb_temp.save()

        lxb_temp = LXB.Temp(temp_file_c, save_to_c)
        lxb_temp.pattern_append("%%DATA%%", ''.join(data))
        lxb_temp.pattern_append("%%SHS%%", ''.join(shs))
        lxb_temp.build()
        lxb_temp.save()

    def print_function_templates(self):
        headers_encode = []
        headers_decode = []
        function_encode = []
        function_decode = []

        for encode in [{'name': 'DEFAULT'}, {'name': 'AUTO'}, {'name': 'UNDEFINED'}] + self.strict:
            name = encode['name']

            decl_encode = ('int8_t' + '\n'
                + self.make_function_name(name, 'encode')
                + '(lxb_encoding_encode_t *ctx, lxb_char_t **data,\n'
                + 'const lxb_char_t *end, lxb_codepoint_t cp)')

            decl_decode = ('lxb_codepoint_t' + '\n'
                + self.make_function_name(name, 'decode')
                + '(lxb_encoding_ctx_t *ctx,' + '\n'
                + 'lxb_char_t **data, const lxb_char_t *end)')

            headers_encode.append('LXB_API ' + decl_encode + ';\n')
            headers_decode.append('LXB_API ' + decl_decode + ';\n')

            function_encode.append(decl_encode + '\n{\n\treturn LXB_ENCODING_ENCODE_ERROR;\n}\n')
            function_decode.append(decl_decode + '\n{\n\treturn LXB_ENCODING_DECODE_ERROR;\n}\n')

        return [headers_encode, headers_decode, function_encode, function_decode]

if __name__ == "__main__":
    enc = Encoding("encodings.json")

    yn = input("Generate and save 'const.h', 'res.h', 'res.c' files? (y - yes, n - no): ")
    if yn.lower() == 'y':
        enc.save_const("tmp/const.h", "../../../source/lexbor/encoding/const.h")
        enc.save_res("tmp/res.h", "../../../source/lexbor/encoding/res.h",
            "tmp/res.c", "../../../source/lexbor/encoding/res.c")

    res_dec = enc.print_function_templates()

    yn = input("Print encode declarations? (y - yes, n - no): ")
    if yn.lower() == 'y':
        print('\n'.join(res_dec[0]))

    yn = input("Print decode declarations? (y - yes, n - no): ")
    if yn.lower() == 'y':
        print('\n'.join(res_dec[1]))

    yn = input("Print encode templates for functions? (y - yes, n - no): ")
    if yn.lower() == 'y':
        print('\n'.join(res_dec[2]))

    yn = input("Print decode templates for functions? (y - yes, n - no): ")
    if yn.lower() == 'y':
        print('\n'.join(res_dec[3]))
