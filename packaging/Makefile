#!/usr/bin/make

include ../version

ARCH?=x86_64
RELEASE?=1
VERSION?=$(LEXBOR_VERSION)

default:
	@echo "available targets: rpm deb"

rpm:
	@cd rpm && VERSION=$(VERSION) RELEASE=$(RELEASE) ARCH=$(ARCH) $(MAKE) all

deb:
	@cd deb && VERSION=$(VERSION) RELEASE=$(RELEASE) ARCH=$(ARCH) $(MAKE) all

clean:
	$(MAKE) -C rpm clean
	$(MAKE) -C deb clean

.PHONY: default rpm deb clean
