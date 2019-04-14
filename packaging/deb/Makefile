#!/usr/bin/make

RELEASE ?= 1

SRCDIR = liblexbor-$(VERSION)
CODENAME = $(shell lsb_release -cs)
DATE = $(shell date +"%a, %d %b %Y %T +0300")
BUILD_DEPENDS = build-essential debhelper devscripts fakeroot lintian lsb-release xsltproc
DEB_HOST_MULTIARCH ?= $(shell dpkg-architecture -qDEB_HOST_MULTIARCH)

default:
	@echo "available targets: all build clean"

all: liblexbor

debuild/$(SRCDIR)/debian:
	set -e ; \
	mkdir -p debuild/$(SRCDIR) ; \
	cp -pr debian debuild/$(SRCDIR)

mkonfly: debuild/$(SRCDIR)/debian/changelog debuild/$(SRCDIR)/debian/liblexbor.install debuild/$(SRCDIR)/debian/liblexbor-dev.install

debuild/$(SRCDIR)/debian/changelog: debuild/$(SRCDIR)/debian
	cat debuild/$(SRCDIR)/debian/changelog.in | sed \
		-e "s#%%VERSION%%#$(VERSION)#g" \
		-e "s#%%RELEASE%%#$(RELEASE)#g" \
		-e "s#%%CODENAME%%#$(CODENAME)#g" \
		-e "s#%%DATE%%#$(DATE)#g" \
		> debuild/$(SRCDIR)/debian/changelog
	rm debuild/$(SRCDIR)/debian/changelog.in

debuild/$(SRCDIR)/debian/liblexbor.install: debuild/$(SRCDIR)/debian
	cat debuild/$(SRCDIR)/debian/liblexbor.install.in | sed \
		-e "s#%%ARCH%%#$(DEB_HOST_MULTIARCH)#g" \
		> debuild/$(SRCDIR)/debian/liblexbor.install
	rm debuild/$(SRCDIR)/debian/liblexbor.install.in

debuild/$(SRCDIR)/debian/liblexbor-dev.install: debuild/$(SRCDIR)/debian
	cat debuild/$(SRCDIR)/debian/liblexbor-dev.install.in | sed \
		-e "s#%%ARCH%%#$(DEB_HOST_MULTIARCH)#g" \
		> debuild/$(SRCDIR)/debian/liblexbor-dev.install
	rm debuild/$(SRCDIR)/debian/liblexbor-dev.install.in

debuild/$(SRCDIR)/debian/liblexbor.dirs: debuild/$(SRCDIR)/debian
	cat debuild/$(SRCDIR)/debian/liblexbor.dirs.in | sed \
		-e "s#%%ARCH%%#$(DEB_HOST_MULTIARCH)#g" \
		> debuild/$(SRCDIR)/debian/liblexbor.dirs
	rm debuild/$(SRCDIR)/debian/liblexbor.dirs.in

debuild/$(SRCDIR)/debian/liblexbor-dev.dirs: debuild/$(SRCDIR)/debian
	cat debuild/$(SRCDIR)/debian/liblexbor-dev.dirs.in | sed \
		-e "s#%%ARCH%%#$(DEB_HOST_MULTIARCH)#g" \
		> debuild/$(SRCDIR)/debian/liblexbor-dev.dirs
	rm debuild/$(SRCDIR)/debian/liblexbor-dev.dirs.in

debuild/liblexbor_$(VERSION).orig.tar.gz: mkonfly
	cd ../.. && tar -czf packaging/deb/debuild/$(SRCDIR).tar.gz \
		--transform "s#^#$(SRCDIR)/#" \
		LICENSE NOTICE CHANGELOG.md README.md CMakeLists.txt \
		config.cmake feature.cmake source test
	mv debuild/$(SRCDIR).tar.gz debuild/liblexbor_$(VERSION).orig.tar.gz
	cd debuild && tar zxf liblexbor_$(VERSION).orig.tar.gz

liblexbor: debuild/liblexbor_$(VERSION).orig.tar.gz
	@echo "===> Building $@ package"
	cd debuild/$(SRCDIR) && debuild -us -uc
	mkdir -p debs/$(CODENAME)
	find debuild/ -maxdepth 1 -type f -name "*.deb" -exec cp {} debs/$(CODENAME)/ \;

clean:
	rm -rf debuild debuild-* debs
	rm -f check-build-depends-*

.PHONY: default all build clean
