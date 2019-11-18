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
ifneq (,$(wildcard rpm/Makefile))
	$(MAKE) -C rpm clean
endif
ifneq (,$(wildcard deb/Makefile))
	$(MAKE) -C deb clean
endif

.PHONY: default rpm deb clean
