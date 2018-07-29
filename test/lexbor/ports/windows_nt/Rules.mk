#*******************************
# Windows_NT
#*******************
ifeq ($(OS),Windows_NT)
	LIB_NAME_SUFFIX := .dll
	LIB_NAME_SUFFIX_STATIC := .dll.a

	PROJECT_CFLAGS += -Wno-unused-variable -Wno-unused-function -std=c99
    PROJECT_LDFLAGS += -Wl,--out-implib,$(call PROJECT_LIBRARY_STATIC)

	PROJECT_OS_NAME := WinNT
	PROJECT_PORT_NAME := windows_nt
endif
# end of Windows_NT
