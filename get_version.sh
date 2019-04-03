#!/bin/sh

if [[ ! -f $1 ]]; then
    >&2 echo "$0: error: file not exist $1"
    exit 1
fi

read -d ''  version_script << 'EOF'
BEGIN { major="?"; minor="?"; patch="?" }
/_VERSION_MAJOR/ { major=$3 }
/_VERSION_MINOR/ { minor=$3 }
/_VERSION_PATCH/ { patch=$3 }
/_VERSION_SUFFIX/ { suffix=$3; gsub(/"/, "", suffix) }
END {
    if (length(suffix) > 0)
        printf "%s.%s.%s%s", major, minor, patch, suffix
    else
        printf "%s.%s.%s", major, minor, patch
}
EOF

version=$(grep '#define[ ]\+[^ ]\+_VERSION_\(MAJOR\|MINOR\|PATCH\|SUFFIX\)' $1 | awk "$version_script")
if [[ ! $version =~ ^[0-9]+\.[0-9]+\.[0-9]+([a-zA-Z0-9_\-]+)?$ ]]; then
    echo "Unable to extract version from $1"
    exit 1
fi

echo "$version"
