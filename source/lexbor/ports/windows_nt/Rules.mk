#*******************************
# Windows_NT
#*******************
ifeq ($(OS),Windows_NT)
	LIB_NAME_SUFFIX := .dll
	LIB_NAME_SUFFIX_STATIC := .dll.a

	PROJECT_LIBRARY_NAME_WITH_VERSION := lib$(LIB_NAME)-$(PROJECT_VERSION_MAJOR)$(LIB_NAME_SUFFIX)

	PROJECT_CFLAGS += -Wno-unused-variable -Wno-unused-function -std=c99
    PROJECT_LDFLAGS += -Wl,--out-implib,$(call PROJECT_LIBRARY_STATIC)

	PROJECT_BUILD_SHARED_AFTER += cp $(call PROJECT_LIBRARY_WITH_VERSION) $(BINARY_DIR_BASE) $(PROJECT_UTILS_NEW_LINE)
	PROJECT_BUILD_CLEAN_AFTER += rm -f $(BINARY_DIR_BASE)/$(call PROJECT_LIBRARY_NAME_WITH_VERSION) $(PROJECT_UTILS_NEW_LINE)

	PROJECT_OS_NAME := WinNT
	PROJECT_PORT_NAME := windows_nt
endif
# end of Windows_NT
