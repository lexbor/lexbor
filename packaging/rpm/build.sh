#!/bin/sh

for opt in "$@"; do
    case "$opt" in
        -*=*) value=`echo "$opt" | sed -e 's/[-_a-zA-Z0-9]*=//'`        ;;
           *) value=""                                                  ;;
    esac

    case "$opt" in
        --arch=*)      arch="$value"                                    ;;
        --builddir=*)  builddir="$value"                                ;;
        --version=*)   version="$value"                                 ;;
        --yes)         say_yes="y"                                      ;;

        --help)
            cat << END

    --version=ident       required. set package version, default: x86_64
    --arch=ident          set architecture for build, default: x86_64
    --builddir=path       set build directory path, default: build
    --y                   says yes for all questions

END
            exit 0
        ;;

        *)
            echo
            echo $0: error: invalid RPM option \"$opt\"
            echo
            exit 1
        ;;
    esac
done

function check_command {
    command=$1
    package=$2
    if ! type "$command" > /dev/null 2>&1; then
        echo "Missing command '$command', run: yum install $package"
        exit 1
    fi
}

check_command "rpmbuild" "rpm-build"

arch=${arch="x86_64"}
say_yes=${say_yes=""}
builddir=${builddir=build}
base="liblexbor-$version"
archive="$base.tar.gz"
files="CMakeLists.txt config.cmake feature.cmake README.md INSTALL.md NOTICE LICENSE source version"
lib_extension=so

if [[ -z $version ]]; then
    echo $0: RPM build error: version argument not set.
    exit 1;
fi

echo -n "Build for "

if [[ "$OSTYPE" == "linux-gnu" ]]; then
    echo "Linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "macOS"
    lib_extension=dylib
elif [[ "$OSTYPE" == "cygwin" ]]; then
    echo "Windows POSIX"
elif [[ "$OSTYPE" == "msys" ]]; then
    echo "Windows GNU utilities"
elif [[ "$OSTYPE" == "freebsd"* ]]; then
    echo "FreeBSD"
else
    echo "unknown system"
fi

echo "Building version ${version}"

if [[ -z "$say_yes" && -d build ]]; then
    read -p "Build directory exists, remove? [y|n] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -rf build
    fi
fi

mkdir -p $builddir/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
mkdir -p "$builddir/SOURCES/$base"

echo "Copying files"
for file in $files; do
  cp -r  "../../$file" "$builddir/SOURCES/$base"
done

echo "Archiving $archive"
pushd "$builddir/SOURCES"
tar zcf $archive $base
popd

echo "Building package:"
rpmbuild --target $arch \
    --define "lib_ext ${lib_extension}" \
    --define "_topdir ${PWD}/build"     \
    --define "lib_version $version"     \
    -ba liblexbor.spec
