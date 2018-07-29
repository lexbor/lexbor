module_info_core_description := base module, it is used by all other modules
module_info_core_dependencies :=

module_target_core_all: $(PROJECT_MODULE_core_EXECUTE)

module_target_core_clean: 
	rm -f $(PROJECT_MODULE_core_EXECUTE)

