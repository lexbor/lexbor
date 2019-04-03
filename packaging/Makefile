#!/usr/bin/make

ARCH?=x86_64
RELEASE?=1
VERSION?=$(shell ../get_version.sh ../source/lexbor/core/base.h)

default:
	@echo "available targets: rpm"

rpm:
	@cd rpm && VERSION=$(VERSION) RELEASE=$(RELEASE) ARCH=$(ARCH) $(MAKE) all

clean:
	make all

.PHONY: default rpm clean
