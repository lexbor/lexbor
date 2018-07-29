module_info_html_description := HTML module
module_info_html_dependencies := core

module_target_html_all: $(PROJECT_MODULE_html_EXECUTE)

module_target_html_clean: 
	rm -f $(PROJECT_MODULE_html_EXECUTE)

module_target_html_tokenizer_tokens_args_1 := ../files/lexbor/html/tokenizer
