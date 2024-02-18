# lexbor packaging

## RPM

In project root directory:
1. cmake . -DLEXBOR_MAKE_RPM_FILES=ON
2. cd packaging
3. make rpm
4. ls -l rpm/build/RPMS/

## DEB

In project root directory:
1. cmake . -DLEXBOR_MAKE_DEB_FILES=ON
2. cd packaging
3. make deb
4. ls -l deb/debs/
