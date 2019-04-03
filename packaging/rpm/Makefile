#!/usr/bin/make

default:
	@echo "available targets: rpm"

all: build

build:
	./build.sh --version=$(VERSION) --arch=$(ARCH) --yes

clean:
	rm -rf build

.PHONY: default all build clean
