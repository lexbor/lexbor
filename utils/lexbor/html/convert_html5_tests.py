
import glob
import re
import ntpath

class Converter:

    def __init__(self, dir_path):
        self.entries = {}

        files = glob.glob("{}/*.dat".format(dir_path))

        for filename in files:
            entries = []
            fh = open(filename, 'r')

            name = ""

            for line in fh:
                res = re.match("^#(.+)", line)

                if res is not None:
                    name = res.group(1)

                    if name == 'data':
                        if len(entries) != 0:
                            if "document" in entries[-1]:
                                entries[-1]["document"].pop()

                        entries.append({})

                    entries[-1][name] = []

                else:
                    entries[-1][name].append(line[:-1])

            fh.close()

            fname = ntpath.basename(filename)
            self.entries[fname[:-4]] = entries

    def make_unit_files(self, save_to_dir):
        entries = self.entries

        for name in entries:
            print("File name: {}".format(name))

            save_to = "{}/{}.ton".format(save_to_dir, name)

            res = self.make_unit_file(entries[name])

            w_fh = open(save_to, 'w')
            w_fh.write("[\n{}\n]".format("\n".join(res)))
            w_fh.close()

    def make_unit_file(self, entries):
        json_entries = []
        idx = 0
        
        json_entries.append("    /* Test count: {} */".format(len(entries)))

        for entry in entries:
            idx += 1;

            json_entries.append("    /* Test number: {} */".format(idx))
            json_entries.append("    {")
            json_entries.append("        {}".format("\n        ".join(self.make_myjson(entry))))
            json_entries.append("    },")

        return json_entries

    def make_myjson(self, entry):
        if "data" not in entry:
            raise Exception("Data not exist")

        result = []

        # data
        data = "\n    ".join(entry["data"]).split("\n")

        for idx in range(0, len(data)):
            data[idx] = data[idx].replace(b"\r", b"\\r")
            data[idx] = data[idx].replace(b"\0", b"\\0")

        result.append('"data": $DATA{ ,12}')
        result.append("    {}".format("\n        ".join(data)))
        result.append('$DATA,')

        # script
        if "script-on" in entry:
            result.append('"scripting": true,')
        elif "script-off" in entry:
            result.append('"scripting": false,')

        # result
        if "document-fragment" in entry:
            if len(entry["document-fragment"]) != 1:
                raise Exception("Garbage in document-fragment")

            fnd = re.match("^([^ ]+)\s+(.+)$", entry["document-fragment"][0])

            if fnd is not None:
                result.append('"fragment": {{"tag": "{}", "ns": "{}"}},'.format(fnd.group(2), fnd.group(1)))

            else:
                result.append('"fragment": {{"tag": "{}", "ns": "{}"}},'.format(entry["document-fragment"][0], "html"))

        elif "document" not in entry:
            raise Exception("Bad type")

        new_doc = []

        # oh God...

        for line in entry["document"]:
            if re.match("^\| ", line) is not None:
                fline = "{}\n".format(line[2:])

            else:
                fline = "{}\n".format(line)
                new_doc.append(fline)

                continue

            fnd = re.match("^(\s*?)<([^ ]+)\s+([^>]+)>", fline)

            if fnd is not None and re.match("^\s*?<\!|^\s*?<\?", fline) is None:
                new_doc.append("{}<{}:{}>\n".format(fnd.group(1), fnd.group(2), fnd.group(3)))

            elif re.match("^\s*?\"", fline) is not None:
                new_doc.append(fline)

            elif re.match("^\s*?<", fline) is None:
                fline = re.sub(r"^\s+|\s+$", "", fline)
                fnd = re.match("^(\s*?)<(?:[^>]+:)?template>", new_doc[-1])

                if fline == "content" and fnd is not None:
                    new_doc.append("{}  #document-fragment\n".format(fnd.group(1)))

                else:
                    fnd = re.match("^([^ =]+)\s+([^ =]+)(=.+)?$", fline)

                    if fnd is not None:
                        new_doc[-1] = re.sub(r">$", " {}:{}{}>".format(fnd.group(1), fnd.group(2), fnd.group(3)), new_doc[-1])

                    else:
                        new_doc[-1] = re.sub(r">$", " {}>".format(fline), new_doc[-1])

            else:
                new_doc.append(fline)

        data = "    ".join(new_doc).split("\n")[:-1]

        result.append('"result": $DATA{ ,12}')
        result.append("    {}".format("\n        ".join(data)))
        result.append('$DATA')

        return result

if __name__ == "__main__":
    conv = Converter("tree-construction")
    conv.make_unit_files("html5_test")
