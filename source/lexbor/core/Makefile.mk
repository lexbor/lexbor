module_core_info_description := base module, it is used by all other modules
module_core_info_dependencies :=

module_core_install_headers := 1

module_target_core_all: $(PROJECT_MODULE_core_OBJECTS)

module_target_core_clean:
	$(RM) -f $(PROJECT_MODULE_core_OBJECTS)
