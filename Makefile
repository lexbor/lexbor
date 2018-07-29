TARGET := source
SRCDIR := source

CC ?= gcc

.DEFAULT_GOAL := all

#********************
# other Makefile
#***************
include Makefile.cfg
include make/Makefile.cfg
include make/Makefile.utils.cfg
include make/Makefile.module.cfg

$(call project_utils_modules_load_from_dir,$(strip $(SRCDIR)))

PROJECT_BUILD_MODULES_TARGET_ALL   := $(call project_utils_modules_target_all)
PROJECT_BUILD_MODULES_TARGET_CLEAN := $(call project_utils_modules_target_clean)

PROJECT_BUILD_MODULES_TARGET_SOURCES := $(call project_utils_modules_target_sources)
PROJECT_BUILD_MODULES_TARGET_OBJECTS := $(call project_utils_modules_target_objects)

#********************
# Set -D
#***************
ifeq ($(LEXBOR_DEBUG),YES)
	override PROJECT_OPTIMIZATION_LEVEL := -O0
	PROJECT_CFLAGS += -g3 -ggdb3 -fno-omit-frame-pointer -DLEXBOR_DEBUG
endif

ifeq ($(LEXBOR_WITHOUT_THREADS),YES)
	PROJECT_CFLAGS += -DLEXBOR_WITHOUT_THREADS
endif

ifeq ($(LEXBOR_WITH_PERF),YES)
	PROJECT_CFLAGS += -DLEXBOR_WITH_PERF
endif

PROJECT_CFLAGS += $(PROJECT_OPTIMIZATION_LEVEL)
PROJECT_CFLAGS += -std=$(PROJECT_SPEC_VERSION)

#********************
# Set ARGS for flags
#***************
PROJECT_CFLAGS += $(patsubst -DMY%,-DMy%,$(call project_utils_upcase,-DLEXBOR_OS_$(PROJECT_OS_NAME)))

override CFLAGS += $(strip $(PROJECT_CFLAGS))
override LDFLAGS += $(strip $(PROJECT_LDFLAGS))

#********************
# Include all modules Makefile.mk
#***************
include $(call project_utils_modules_make)

#********************
# Objects
#***************
PROJECT_BUILD_OBJECT_SHARED  ?= $(CC) -shared $(LDFLAGS) $(1) -o $(abspath $(2))
PROJECT_BUILD_OBJECT_STATIC  ?= $(AR) crus $(2) $(1)
PROJECT_BUILD_OBJECT_MODULES := $(foreach name,$(PROJECT_BUILD_MODULES_TARGET_OBJECTS),$($(name)))

PROJECT_BUILD_SUB_DIRS :=

#********************
# Target options
#***************
all: library
	for f in $(PROJECT_BUILD_SUB_DIRS); do $(MAKE) -C $$f all; done

library: shared static

shared: make-pc-file create $(PROJECT_BUILD_MODULES_TARGET_ALL)
	$(call PROJECT_BUILD_OBJECT_SHARED,$(PROJECT_BUILD_OBJECT_MODULES),$(call PROJECT_LIBRARY_WITH_VERSION))
	$(call PROJECT_BUILD_SHARED_AFTER)

static: make-pc-file create $(PROJECT_BUILD_MODULES_TARGET_ALL)
	$(call PROJECT_BUILD_OBJECT_STATIC,$(PROJECT_BUILD_OBJECT_MODULES),$(call PROJECT_LIBRARY_STATIC))
	$(call PROJECT_BUILD_STATIC_AFTER)

create:
	$(MKDIR) -p $(BINARY_DIR_BASE) $(LIB_DIR_BASE) $(TEST_DIR_BASE)

clean: $(PROJECT_BUILD_MODULES_TARGET_CLEAN)
	$(RM) -f $(call PROJECT_LIBRARY_WITH_VERSION) && $(RM) -f $(call PROJECT_LIBRARY_STATIC)
	$(call PROJECT_BUILD_CLEAN_AFTER)
	$(MAKE) -C test/lexbor clean
	for f in $(PROJECT_BUILD_SUB_DIRS); do $(MAKE) -C $$f clean; done

api_clean:
	$(RM) -rf $(PROJECT_INCLUDE_DIR_API)

api_update: api_clean
	$(call project_utils_modules_headers_clone,source/)
	$(call PROJECT_CLONE_SED_HEADER_COMMAND)
	$(FIND) $(PROJECT_INCLUDE_DIR_API) -name "*.h.bak" -exec rm -f {} \; | 2>&1

make-pc-file:
	$(call PROJECT_PKG_CONFIG_PROCESS,$(PROJECT_PKG_CONFIG_FILE_IN),$(PROJECT_PKG_CONFIG_FILE))

modules:
	$(info $(call project_utils_modules_info))

test:
	$(MAKE) -C test/lexbor all
	$(MAKE) -C test/lexbor run

.PHONY: test
