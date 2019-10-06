#!/bin/sh

set -e

if [[ ! -d $1 ]]; then
    >&2 echo "$0: error: directory not exist $1"
    exit 1
fi

mkdir -p ./pvs-studio/build
cd ./pvs-studio/build

pvs-studio-analyzer credentials ${LEXBOR_PVS_NAME} ${LEXBOR_PVS_KEY}

cmake $1 -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DLEXBOR_BUILD_EXAMPLES=ON

pvs-studio-analyzer analyze -o ../project.log -j8
plog-converter -a GA:1,2 -t tasklist -o ../project.tasks ../project.log
