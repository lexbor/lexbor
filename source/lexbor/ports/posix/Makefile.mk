module_ports_info_description := base module, it is used by all other modules
module_ports_info_dependencies :=

module_target_ports_all: $(PROJECT_MODULE_ports_OBJECTS)

module_target_ports_clean:
	rm -f $(PROJECT_MODULE_ports_OBJECTS)
