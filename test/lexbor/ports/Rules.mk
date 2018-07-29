MODULE_PORTS_DIR := $(call project_utils_dir_join,$(SRCDIR) ports)
$(call project_utils_modules_load_rules_from_dir,$(MODULE_PORTS_DIR))

#*******************************
# Default POSIX
#*******************
ifeq ($(strip $(PROJECT_PORT_NAME)),)
  PROJECT_CFLAGS += -fPIC -D_POSIX_C_SOURCE=199309L

  # Important!
  PROJECT_PORT_NAME := posix
endif 
# end of POSIX

$(call project_utils_modules_append,$(MODULE_PORTS_DIR)$(PROJECT_CONF_DIR_SEPARATOR)$(PROJECT_PORT_NAME),ports)
