module_info_lexbor_description := base module, it is used by all other modules
module_info_lexbor_dependencies :=

module_target_unit_all: $(PROJECT_MODULE_unit_OBJECTS)

module_target_unit_clean: 
	rm -f $(PROJECT_MODULE_unit_OBJECTS)
