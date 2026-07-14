#!/usr/bin/env python3
#
# WPT HTML tree construction helper.
#
# By default, the script uses existing downloaded `.dat` files from --dest.  If
# no `.dat` files are present, it downloads WPT resources first, then runs the C
# runner.
#
# Arguments:
#   --ref              WPT git ref to download from; defaults to "master".
#   --dest             Directory for downloaded WPT resources.
#   --build-dir        Lexbor build directory used to locate the default runner.
#   --runner           Explicit path to the wpt_tree_construction executable.
#   --force-download   Download resources even when local `.dat` files exist.
#   --clean            Remove the downloaded resource directory; with
#                      --force-download, remove it before downloading again.
#   --no-run           Prepare resources but do not run the C runner.

import argparse
import json
import shutil
import subprocess
import sys
import tempfile
import urllib.parse
import urllib.request
from pathlib import Path


WPT_API = (
    "https://api.github.com/repos/web-platform-tests/wpt/contents/"
    "html/syntax/parsing/resources"
)
DOWNLOAD_TIMEOUT = 60


def repo_root() -> Path:
    return Path(__file__).resolve().parents[3]


def fetch_json(url: str):
    req = urllib.request.Request(url, headers={"User-Agent": "lexbor-wpt-runner"})
    with urllib.request.urlopen(req, timeout=DOWNLOAD_TIMEOUT) as resp:
        return json.loads(resp.read().decode("utf-8"))


def fetch_bytes(url: str) -> bytes:
    req = urllib.request.Request(url, headers={"User-Agent": "lexbor-wpt-runner"})
    with urllib.request.urlopen(req, timeout=DOWNLOAD_TIMEOUT) as resp:
        return resp.read()


def list_resource_files(ref: str):
    url = "{}?ref={}".format(WPT_API, urllib.parse.quote(ref, safe=""))
    entries = fetch_json(url)

    if not isinstance(entries, list):
        raise RuntimeError("unexpected GitHub API response")

    result = []
    for entry in entries:
        name = entry.get("name")
        if entry.get("type") != "file" or name is None:
            continue

        if name == "README.md" or name.endswith(".dat"):
            result.append(entry)

    return sorted(result, key=lambda item: item["name"])


def replace_directory(source: Path, dest: Path):
    backup = None

    if dest.exists():
        backup = Path(tempfile.mkdtemp(prefix=".{}.backup.".format(dest.name),
                                      dir=dest.parent))
        backup.rmdir()
        dest.replace(backup)

    try:
        source.replace(dest)
    except Exception:
        if backup is not None:
            backup.replace(dest)
        raise

    if backup is not None:
        shutil.rmtree(backup, ignore_errors=True)


def download_resources(ref: str, dest: Path) -> int:
    entries = list_resource_files(ref)

    dest.parent.mkdir(parents=True, exist_ok=True)
    tmp = Path(tempfile.mkdtemp(prefix=".{}.download.".format(dest.name),
                               dir=dest.parent))

    count = 0
    dat_count = 0

    try:
        for entry in entries:
            name = entry["name"]
            url = entry.get("download_url")

            if url is None:
                path = "html/syntax/parsing/resources/{}".format(name)
                url = "https://raw.githubusercontent.com/web-platform-tests/wpt/{}/{}".format(
                    urllib.parse.quote(ref, safe="/"),
                    urllib.parse.quote(path, safe="/"),
                )

            data = fetch_bytes(url)
            (tmp / name).write_bytes(data)
            count += 1

            if name.endswith(".dat"):
                dat_count += 1

            print("downloaded {}".format(name))

        if dat_count == 0:
            raise RuntimeError("no WPT .dat resource files found")

        replace_directory(tmp, dest)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)

    return count


def has_resource_files(dest: Path) -> bool:
    return dest.is_dir() and any(dest.glob("*.dat"))


def clean_resources(dest: Path):
    if dest.exists():
        print("removing {}".format(dest))
        shutil.rmtree(dest)
    else:
        print("nothing to remove: {}".format(dest))


def default_runner(root: Path, build_dir: Path) -> Path:
    if not build_dir.is_absolute():
        build_dir = root / build_dir

    name = "wpt_tree_construction.exe" if sys.platform == "win32" else "wpt_tree_construction"

    return build_dir / "test" / "lexbor" / "html" / name


def main(argv=None) -> int:
    root = repo_root()

    parser = argparse.ArgumentParser(
        description="Prepare WPT HTML tree construction tests and run Lexbor's C runner."
    )
    parser.add_argument("--ref", default="master", help="WPT git ref to download")
    parser.add_argument("--dest", type=Path,
                        help="directory for downloaded WPT resources; defaults "
                             "to a location under --build-dir")
    parser.add_argument("--build-dir", type=Path, default=Path("build"),
                        help="Lexbor build directory used to find the runner")
    parser.add_argument("--runner", type=Path,
                        help="path to the wpt_tree_construction executable")
    parser.add_argument("--force-download", action="store_true",
                        help="download WPT resources even if local .dat files exist")
    parser.add_argument("--clean", action="store_true",
                        help="remove the downloaded WPT resource directory")
    parser.add_argument("--no-run", action="store_true",
                        help="prepare WPT resources but do not run the C runner")

    args = parser.parse_args(argv)

    build_dir = args.build_dir
    if not build_dir.is_absolute():
        build_dir = root / build_dir

    if args.dest is None:
        dest = build_dir / "test" / "files" / "lexbor" / "html" / "wpt_tree_construction"
    else:
        dest = args.dest if args.dest.is_absolute() else root / args.dest

    if args.clean:
        clean_resources(dest)

        if not args.force_download:
            return 0

    if args.force_download or not has_resource_files(dest):
        count = download_resources(args.ref, dest)
        print("downloaded {} WPT resource files into {}".format(count, dest))
    else:
        print("using existing WPT resource files in {}".format(dest))

    if args.no_run:
        return 0

    runner = args.runner
    if runner is None:
        runner = default_runner(root, args.build_dir)
    elif not runner.is_absolute():
        runner = root / runner

    if not runner.exists():
        print("runner not found: {}".format(runner), file=sys.stderr)
        print("build tests first, for example: cmake --build {}".format(args.build_dir),
              file=sys.stderr)
        return 2

    sys.stdout.flush()

    return subprocess.call([str(runner), str(dest)])


if __name__ == "__main__":
    raise SystemExit(main())
