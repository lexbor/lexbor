module_dom_info_description := base module, it is used by all other modules
module_dom_info_dependencies :=

module_dom_install_headers := 1

module_target_dom_all: $(PROJECT_MODULE_dom_OBJECTS)

module_target_dom_clean:
	$(RM) -f $(PROJECT_MODULE_dom_OBJECTS)
