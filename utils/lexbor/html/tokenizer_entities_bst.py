
import json
import sys, re, os

# Find and append run script run dir to module search path
ABS_PATH = os.path.dirname(os.path.abspath(__file__))
sys.path.append("{}/../lexbor/".format(ABS_PATH))

import LXB

def entities_bst(tmp_name, save_name, json_filename):
    print(ABS_PATH)
    index = entities_bst_load(os.path.join(ABS_PATH, json_filename))
    bst = entities_bst_create(index);
    result = entities_bst_result("lxb_html_tokenizer_res_entities_sbst", bst)

    lxb_temp = LXB.Temp(os.path.join(ABS_PATH, tmp_name), os.path.join(ABS_PATH, save_name))
    lxb_temp.pattern_append("%%BODY%%", ''.join(result))

    lxb_temp.build()
    lxb_temp.save()

    # entities_bst_print(bst)
    print(''.join(lxb_temp.buffer))

    print("Saved to {}".format(lxb_temp.fn_to_save))
    print("Done!")


def entities_bst_load(json_filename):
    jdata = json.load(open(json_filename))
    index = {}

    for name in jdata:
        entities_bst_create_layer(name, jdata[name], index)

    return index

def entities_bst_save(json_filename):
    jdata = json.load(open(json_filename))
    index = {}

    for name in jdata:
        entities_bst_create_layer(name, jdata[name], index)

    return index

def entities_bst_create_layer(name, entry, index):
    chars = list(name)
    last = len(chars)

    for pos in range(1, last):
        char = chars[pos]

        if char not in index:
            index[char] = {'next': {}, 'values': []}

        if pos == (last - 1):
            index[char]['values'].append(entry);

        index = index[char]['next']

def entities_bst_create(index):
    bst = {}
    bst[0] = ["\0", 0, 0, 0, "NULL"]

    begin = 1
    idx = end = entities_bst_create_tree(index, bst, begin)

    stack = []
    while True:
        while begin < end:
            if len(bst[begin][5]) != 0:
                index = bst[begin][5]

                bst[begin][5] = 0
                bst[begin][3] = idx

                stack.append([begin + 1, end])

                begin = idx
                end = idx = entities_bst_create_tree(index, bst, idx)

            else:
                begin = begin + 1

        if len(stack) == 0:
            break

        entry = stack.pop();
        begin = entry[0]
        end = entry[1]

    return bst

def entities_bst_create_tree(index, bst, idx):
    keys = list(index.keys())
    keys.sort()

    optv = []
    inr = -1

    while len(keys):
        split = entities_bst_split(keys);

        idx_left = idx + 2 if len(split[1]) != 0 else 0
        idx_right = idx + 1 if len(split[2]) != 0 else 0

        if idx_right == 0 and idx_left != 0:
            idx_left = idx_left - 1

        inr = bst[inr][2] if inr in bst else idx

        assert len(index[ split[0] ]['values']) < 2, 'Double values'

        if len(index[ split[0] ]['values']) == 0:
            value = "NULL"
        else:
            value = '"{}"'.format(toHex(index[ split[0] ]['values'][0]['characters']))

        #           key     , left    , right    , next, value,
        bst[inr] = [split[0], idx_left, idx_right,    0, value,
                    index[ split[0] ]['next'] ]

        if len(split[2]) != 0:
            optv.append([split[2], inr])

        if idx_left:
            idx = idx_left

        inr = -1
        keys = split[1]

        if len(keys) == 0:
            if len(optv) == 0:
                break

            entry = optv.pop()
            keys = entry[0];
            inr = entry[1];

    return idx + 1


def toHex(s):
    lst = []

    for ch in bytes(s, 'utf-8'):
        hv = hex(ch).replace('0x', '\\x')
        lst.append(hv)

    return ''.join(lst)


def entities_bst_split(keys):
    srt = int(len(keys) / 2)
    return [keys[srt], keys[0:srt], keys[srt + 1:len(keys)]]


def entities_bst_result(name, bst):
    entries = list(bst.keys());
    entries.sort()

    result = []
    last_idx = entries.pop()

    result.append("#ifdef {}\n".format(name.upper()))
    result.append("#ifndef {}_ENABLED\n".format(name.upper()))
    result.append("#define {}_ENABLED\n".format(name.upper()))
    result.append("static const lexbor_sbst_entry_static_t {}[] =\n".format(name))
    result.append("{\n\t")

    for idx in entries:
        result.append("{{0x{0:02x}, {1}, {2}, {3}, {4}, {5}}}"
                      .format(ord(bst[idx][0]), bst[idx][4], bst[idx][4].count('\\x'),
                              bst[idx][1], bst[idx][2],
                              bst[idx][3]));

        if int(idx) % 2 == 1:
            result.append(",\n\t")
        else:
            result.append(", ")

    result.append("{{0x{0:02x}, {1}, {2}, {3}, {4}, {5}}}\n"
                  .format(ord(bst[last_idx][0]), bst[last_idx][4],
                          bst[last_idx][4].count('\\x'),
                          bst[last_idx][1], bst[last_idx][2],
                          bst[last_idx][3]));

    result.append("};\n")
    result.append("#endif /* {}_ENABLED */\n".format(name.upper()))
    result.append("#endif /* {} */\n".format(name.upper()))

    return result


def entities_bst_print(bst):
    entries = list(bst.keys());
    entries.sort()

    result = []

    for idx in entries:
        print("{}: {{'{}', {}, {}, {}, {}, {}}}"
              .format(idx, bst[idx][0], bst[idx][4], bst[idx][4].count('\\x'),
                      bst[idx][1], bst[idx][2], bst[idx][3]));

    return result


if __name__ == "__main__":
    entities_bst("tmp/tokenizer_res.h",
                 "../../../source/lexbor/html/tokenizer_res.h",
                 "data/entities.json");
