#*******************************
# Darwin, Mac OS X
#*******************
ifeq ($(OS),Darwin)
  PROJECT_CFLAGS += -fPIC
  PROJECT_CFLAGS += -std=c99
  LIB_NAME_SUFFIX := .dylib

  PROJECT_OS_NAME := MacOSX
  PROJECT_PORT_NAME := posix
endif
# end of Darwin, Mac OS X
