module_html_info_description := base module, it is used by all other modules
module_html_info_dependencies :=

module_html_install_headers := 1

module_target_html_all: $(PROJECT_MODULE_html_OBJECTS)

module_target_html_clean:
	$(RM) -f $(PROJECT_MODULE_html_OBJECTS)
