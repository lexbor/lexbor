#!/bin/sh

rm -rf include
rsync -avm --include='*.h' -f 'hide,! */' source/ include

# For Mac OS X
find include -name "*.h" -exec sed -i '.bak' -E 's/^[ \t]*\#[ \t]*include[ \t]*"([^"]+)"/\#include <\1>/g' {} \; | 2>&1

# For linux
# find include -name "*.h" -exec $(SED) -i.bak -e 's,\s*\#\s*include\s*"\([^"]*\)",\#include <\1>,g' {} \; | 2>&1

find include -name "*.h.bak" -exec rm -f {} \; | 2>&1
