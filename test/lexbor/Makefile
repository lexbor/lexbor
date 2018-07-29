TARGET := test
SRCDIR := .

CC ?= gcc

.DEFAULT_GOAL := all

#********************
# other Makefile
#***************
include ../../Makefile.cfg
include ../../make/Makefile.cfg
include ../../make/Makefile.utils.cfg
include ../../make/Makefile.module.cfg

$(call project_utils_modules_load_from_dir_without,$(strip $(SRCDIR)),unit)
$(call project_utils_module_load,$(strip $(SRCDIR))/unit,unit,UNIT_LIB)

#********************
# All test
#***************
TEST_BUILD_MODULES_TARGET_ALL   := $(call project_utils_modules_target_all)
TEST_BUILD_MODULES_TARGET_CLEAN := $(call project_utils_modules_target_clean)

TEST_BUILD_MODULES_TARGET_SOURCES := $(call project_utils_modules_target_sources)

#********************
# UnitTest
#***************
UNITTEST_BUILD_MODULES_TARGET_ALL   := $(call project_utils_modules_target_all,UNIT_LIB)
UNITTEST_BUILD_MODULES_TARGET_CLEAN := $(call project_utils_modules_target_clean,UNIT_LIB)

UNITTEST_BUILD_MODULES_TARGET_OBJECTS := $(call project_utils_modules_target_objects,UNIT_LIB)
UNITTEST_BUILD_MODULES_TARGET_EXECUTE := $(call project_utils_modules_target_execute,UNIT_LIB)

#********************
# Name and Version
#***************
UNITTEST_LIBRARY_NAME := test
UNITTEST_LIBRARY = $(abspath $(call project_utils_dir_join,unit lib$(UNITTEST_LIBRARY_NAME)$(LIB_NAME_SUFFIX)))

#********************
# Set ARGS for flags
#***************
LEXBOR_LIB_DIR := $(call project_utils_dir_join,$(abspath ../..) $(LIB_DIR_BASE))

TEST_CFLAGS += $(PROJECT_CFLAGS) -I. -I$(call project_utils_dir_join,$(abspath ../..) $(PROJECT_TARGET))
TEST_LDFLAGS += -L$(LEXBOR_LIB_DIR) -Lunit -l$(UNITTEST_LIBRARY_NAME) -l$(LIB_NAME)
UNIT_LDFLAGS += -L$(LEXBOR_LIB_DIR) -l$(LIB_NAME)

override CFLAGS := $(strip $(TEST_CFLAGS))
override LDFLAGS := $(strip $(TEST_LDFLAGS))
override LDLIBS := $(strip $(TEST_LDLIBS))

#********************
# Include all modules Makefile.mk
#***************
include $(call project_utils_modules_make)
include $(call project_utils_modules_make,UNIT_LIB)

#********************
# Objects
#***************
TEST_BUILD_MODULES_EXECUTE := $(call project_utils_modules_executes_with_args)

UNITTEST_BUILD_OBJECT_SHARED  ?= $(CC) -shared $(UNIT_LDFLAGS) $(1) -o $(2)
UNITTEST_BUILD_OBJECT_STATIC  ?= $(AR) crus $(2) $(1)
UNITTEST_BUILD_OBJECT_MODULES := $(foreach name,$(UNITTEST_BUILD_MODULES_TARGET_OBJECTS),$($(name)))

#********************
# Target options
#***************
all: unit ${TEST_BUILD_MODULES_TARGET_ALL}
clean: unit_clean $(TEST_BUILD_MODULES_TARGET_CLEAN)

unit: ${UNITTEST_BUILD_MODULES_TARGET_ALL}
	$(call UNITTEST_BUILD_OBJECT_SHARED,$(UNITTEST_BUILD_OBJECT_MODULES),$(call UNITTEST_LIBRARY))

unit_clean: $(UNITTEST_BUILD_MODULES_TARGET_CLEAN)
	$(RM) -f $(call UNITTEST_LIBRARY)

run:
	$(TEST_BUILD_MODULES_EXECUTE)
