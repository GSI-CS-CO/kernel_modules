CPU ?= L866
# we don't want to replace anything by mistake
ACC ?= none

#no verbose
V?=@

# by default run all rule
all:

include /acc/dsc/src/co/Make.auto

# ALL_CPUS has to be at least two elements due to brace expansion
ALL_CPUS=L865 L866 L867

ifeq ($(CPU),L865)
KVER ?= 3.2.43-rt63
endif

ifeq ($(CPU),L866)
KVER ?= 3.6.11-rt30.25.el6rt.x86_64
endif

ifeq ($(CPU),L867)
KVER ?= 3.10.0-229.1.2.rt56.141.2.el7_1.x86_64
endif

GIT_VERSION = $(shell git describe --dirty --long --tags --always)

KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)

CROSS_COMPILE=$(TOOLS)

# used to convert list separated with spaces into list separated with commas
comma:= ,
empty:=
space:= $(empty) $(empty)
#used for removals
ALL_CPUS_COMMAS=$(subst $(space),$(comma),$(ALL_CPUS))

# use bash due to brace expansion
SHELL=/bin/bash

# check if variable is defined
# usage $(call check_defined,XXX)
# !! there is no space before variable !!
check_defined = \
    $(if $(value $1),, \
      $(error Undefined $1))

###################################
# Helper rules for the deployment
###################################

# DEPLOY_PATH has to be explicitly set to "/" to deliver to /acc
DEPLOY_PATH?=$(shell pwd)/
export DEPLOY_PATH
# path to deploy drivers and scripts
INST_DRIVER_PATH=$(DEPLOY_PATH)acc/dsc/$(ACC)/$(CPU)/$(KVER)/$(PRODUCT_NAME)
# used for libs, tools and headers
INST_LIB_PATH_RELATIVE=acc/local/$(CPU)/drv/$(PRODUCT_NAME)
INST_LIB_PATH=$(DEPLOY_PATH)$(INST_LIB_PATH_RELATIVE)

INSTALL_BIN_CMD=cp
INSTALL_BIN_PARAMS=--preserve=all -d
INSTALL_DIR_CMD=$(MKDIR)
INSTALL_DIR_PARAMS=-p
INSTALL_LINK=ln
INSTALL_LINK_PARAMS=-sfT

# set default shared objects dir SO_DIR
SO_DIR?=lib

#
# All of the rules below requires PRODUCT_NAME to be specified
# 

# Rule to deploy drivers
#
# Requires:
#     PRODUCT_NAME -- name of a driver
#     DRIVERS_LIST -- contains list of drivers (with source paths) to be
#                     deployed
install_drivers_global:
# check whether all needed variables are defined
	$(call check_defined,PRODUCT_NAME)
	$(call check_defined,DRIVERS_LIST)
# deploy drivers
	@echo "Install drivers into $(INST_DRIVER_PATH)"
	$(V)$(INSTALL_DIR_CMD) $(INSTALL_DIR_PARAMS) $(INST_DRIVER_PATH)
	$(V)$(INSTALL_BIN_CMD) $(INSTALL_BIN_PARAMS) $(DRIVERS_LIST) $(INST_DRIVER_PATH)/

# Rule to deploy scripts used during loading of drivers
#
# Requires:
#     PRODUCT_NAME -- name of a driver to which scripts belong
#     SCRIPTS_LIST -- contains list of installation scripts to be deployed
install_scripts_global:
# check whether all needed variables are defined
	$(call check_defined,PRODUCT_NAME)
	$(call check_defined,SCRIPTS_LIST)
# deploy scripts
	@echo "Install scrtipts into $(INST_DRIVER_PATH)"
	$(V)$(INSTALL_DIR_CMD) $(INSTALL_DIR_PARAMS) $(INST_DRIVER_PATH)
	$(V)$(INSTALL_BIN_CMD) $(INSTALL_BIN_PARAMS) $(SCRIPTS_LIST) $(INST_DRIVER_PATH)/

# Rule to deploy libraries
# Strips CPU from the name
#
# Requires:
#     PRODUCT_NAME -- name of a driver to which libraries belong
#     LIBS_LIST -- contains list of libraries (with CPU) to be deployed
#     LIB_MAJOR -- Major version of libraries
#     LIB_MINOR -- Minor version of libraries
#     LIB_PATCH -- Patch version of libraries
install_libs_global: install_version_files
# check whether all needed variables are defined
	$(call check_defined,PRODUCT_NAME)
	$(call check_defined,LIBS_LIST)
	$(call check_defined,LIB_MAJOR)
	$(call check_defined,LIB_MINOR)
	$(call check_defined,LIB_PATCH)
# create dir a.b.c
	@echo "Install libraries into $(INST_LIB_PATH)/$(LIB_MAJOR).$(LIB_MINOR).$(LIB_PATCH)/lib"
	@echo "    Create $(INST_LIB_PATH)/$(LIB_MAJOR).$(LIB_MINOR).$(LIB_PATCH)/lib"
	$(V)$(INSTALL_DIR_CMD) $(INSTALL_DIR_PARAMS) $(INST_LIB_PATH)/$(LIB_MAJOR).$(LIB_MINOR).$(LIB_PATCH)/lib
# create link a.b -> a.b.c
	@echo "    Link $(LIB_MAJOR).$(LIB_MINOR) to $(LIB_MAJOR).$(LIB_MINOR).$(LIB_PATCH)"
	$(V)$(INSTALL_LINK) $(INSTALL_LINK_PARAMS) $(LIB_MAJOR).$(LIB_MINOR).$(LIB_PATCH) $(INST_LIB_PATH)/$(LIB_MAJOR).$(LIB_MINOR)
# deploy files
	@echo "    Install libraries into $(INST_LIB_PATH)/$(LIB_MAJOR).$(LIB_MINOR).$(LIB_PATCH)/lib:"
	$(V)$(foreach FILE,$(LIBS_LIST),\
		export FILE_NO_CPU=$(subst .$(CPU),,$(notdir $(FILE)));\
		echo "        $$FILE_NO_CPU"; \
		$(INSTALL_BIN_CMD) $(INSTALL_BIN_PARAMS) $(FILE) $(INST_LIB_PATH)/$(LIB_MAJOR).$(LIB_MINOR).$(LIB_PATCH)/lib/$$FILE_NO_CPU || exit 1;\
		)

# Rule to deploy shared libraries
# Strips CPU from the name
#
# Requires:
#     PRODUCT_NAME -- name of a driver to which libraries belong
#     LIBSSO_LIST -- contains list of libraries (with CPU) to be deployed
#     LIBSO_MAJOR -- Major version of libraries
#     LIBSO_MINOR -- Minor version of libraries
#     LIBSO_PATCH -- Patch version of libraries
install_libsso_global: install_version_files
# check whether all needed variables are defined
	$(call check_defined,PRODUCT_NAME)
	$(call check_defined,LIBSSO_LIST)
	$(call check_defined,LIBSO_MAJOR)
	$(call check_defined,LIBSO_MINOR)
	$(call check_defined,LIBSO_PATCH)
# create dir a.b.c
	@echo "Install shared libraries into $(INST_LIB_PATH)/$(LIBSO_MAJOR).$(LIBSO_MINOR).$(LIBSO_PATCH)/$(SO_DIR)"
	@echo "    Create $(INST_LIB_PATH)/$(LIBSO_MAJOR).$(LIBSO_MINOR).$(LIBSO_PATCH)/$(SO_DIR)"
	$(V)$(INSTALL_DIR_CMD) $(INSTALL_DIR_PARAMS) $(INST_LIB_PATH)/$(LIBSO_MAJOR).$(LIBSO_MINOR).$(LIBSO_PATCH)/$(SO_DIR)
# create link a.b -> a.b.c
	@echo "    Link $(LIBSO_MAJOR).$(LIBSO_MINOR) to $(LIBSO_MAJOR).$(LIBSO_MINOR).$(LIBSO_PATCH)"
	$(V)$(INSTALL_LINK) $(INSTALL_LINK_PARAMS) $(LIBSO_MAJOR).$(LIBSO_MINOR).$(LIBSO_PATCH) $(INST_LIB_PATH)/$(LIBSO_MAJOR).$(LIBSO_MINOR)
# deploy files
	@echo "    Install libraries into $(INST_LIB_PATH)/$(LIBSO_MAJOR).$(LIBSO_MINOR).$(LIBSO_PATCH)/$(SO_DIR):"
	$(V)$(foreach FILE,$(LIBSSO_LIST),\
		export FILE_NO_CPU=$(subst .$(CPU),,$(notdir $(FILE)));\
		echo "        $$FILE_NO_CPU"; \
		$(INSTALL_BIN_CMD) $(INSTALL_BIN_PARAMS) $(FILE) $(INST_LIB_PATH)/$(LIBSO_MAJOR).$(LIBSO_MINOR).$(LIBSO_PATCH)/$(SO_DIR)/$$FILE_NO_CPU || exit 1;\
		)

# Rule to deploy driver's or library's headers
#
# Requires:
#     PRODUCT_NAME -- name of a driver to which headers belong
#     HEADERS_LIST -- contains list of headers to be deployed
#     HEADER_MAJOR -- Major version of headers
#     HEADER_MINOR -- Minor version of headers
#     HEADER_PATCH -- Patch version of headers
install_headers_global: install_version_files
# check whether all needed variables are defined
	$(call check_defined,PRODUCT_NAME)
	$(call check_defined,HEADERS_LIST)
	$(call check_defined,HEADER_MAJOR)
	$(call check_defined,HEADER_MINOR)
	$(call check_defined,HEADER_PATCH)
# create dir a.b.c/include/y
	@echo "Install headers into $(INST_LIB_PATH)/$(HEADER_MAJOR).$(HEADER_MINOR).$(HEADER_PATCH)/include/$(PRODUCT_NAME)"
	@echo "    Create $(INST_LIB_PATH)/$(HEADER_MAJOR).$(HEADER_MINOR).$(HEADER_PATCH)/include/$(PRODUCT_NAME)"
	$(V)$(INSTALL_DIR_CMD) $(INSTALL_DIR_PARAMS) $(INST_LIB_PATH)/$(HEADER_MAJOR).$(HEADER_MINOR).$(HEADER_PATCH)/include/$(PRODUCT_NAME)
# create link a.b -> a.b.c
	@echo "    Link $(HEADER_MAJOR).$(HEADER_MINOR) to $(HEADER_MAJOR).$(HEADER_MINOR).$(HEADER_PATCH)"
	$(V)$(INSTALL_LINK) $(INSTALL_LINK_PARAMS) $(HEADER_MAJOR).$(HEADER_MINOR).$(HEADER_PATCH) $(INST_LIB_PATH)/$(HEADER_MAJOR).$(HEADER_MINOR)
# deploy files
	@echo "    Install headers into $(INST_LIB_PATH)/$(HEADER_MAJOR).$(HEADER_MINOR).$(HEADER_PATCH)/include/$(PRODUCT_NAME):"
	$(V)$(foreach FILE,$(HEADERS_LIST),\
		export FILE_NO_CPU=$(subst .$(CPU),,$(notdir $(FILE)));\
		echo "        $$FILE_NO_CPU"; \
		$(INSTALL_BIN_CMD) $(INSTALL_BIN_PARAMS) $(FILE) $(INST_LIB_PATH)/$(HEADER_MAJOR).$(HEADER_MINOR).$(HEADER_PATCH)/include/$(PRODUCT_NAME)/$$FILE_NO_CPU || exit 1;\
		)
	@echo "    Create links to headers in $(INST_LIB_PATH)/include/$(PRODUCT_NAME)"

# Rule to deploy programs or tools
# Strips CPU from the name
#
# Requires:
#     PRODUCT_NAME -- name of a driver to which tools/programs belong
#     PROGS_LIST -- contains list of tools/programs to be deployed
#     PROG_MAJOR -- Major version of tools/programs
#     PROG_MINOR -- Minor version of tools/programs
#     PROG_PATCH -- Patch version of tools/programs
install_prog_global: install_version_files
# check whether all needed variables are defined
	$(call check_defined,PRODUCT_NAME)
	$(call check_defined,PROGS_LIST)
	$(call check_defined,PROG_MAJOR)
	$(call check_defined,PROG_MINOR)
	$(call check_defined,PROG_PATCH)
# create dir a.b.c
	@echo "Install programs into $(INST_LIB_PATH)/$(PROG_MAJOR).$(PROG_MINOR).$(PROG_PATCH)/bin"
	@echo "    Create $(INST_LIB_PATH)/$(PROG_MAJOR).$(PROG_MINOR).$(PROG_PATCH)/bin"
	$(V)$(INSTALL_DIR_CMD) $(INSTALL_DIR_PARAMS) $(INST_LIB_PATH)/$(PROG_MAJOR).$(PROG_MINOR).$(PROG_PATCH)/bin
# create link a.b -> a.b.c
	@echo "    Link $(PROG_MAJOR).$(PROG_MINOR) to $(PROG_MAJOR).$(PROG_MINOR).$(PROG_PATCH)"
	$(V)$(INSTALL_LINK) $(INSTALL_LINK_PARAMS) $(PROG_MAJOR).$(PROG_MINOR).$(PROG_PATCH) $(INST_LIB_PATH)/$(PROG_MAJOR).$(PROG_MINOR)
# deploy files
	@echo "    Install programs into $(INST_LIB_PATH)/$(PROG_MAJOR).$(PROG_MINOR).$(PROG_PATCH)/bin:"
	$(V)$(foreach FILE,$(PROGS_LIST),\
		export FILE_NO_CPU=$(subst .$(CPU),,$(notdir $(FILE)));\
		echo "        $$FILE_NO_CPU"; \
		$(INSTALL_BIN_CMD) $(INSTALL_BIN_PARAMS) $(FILE) $(INST_LIB_PATH)/$(PROG_MAJOR).$(PROG_MINOR).$(PROG_PATCH)/bin/$$FILE_NO_CPU || exit 1;\
		)
	@echo "    Create links to programs in $(INST_LIB_PATH)/bin"

install_version_files:
	$(V)$(INSTALL_DIR_CMD) $(INSTALL_DIR_PARAMS) $(INST_LIB_PATH)/$(PROG_MAJOR).$(PROG_MINOR).$(PROG_PATCH)/
# use if instead of ifneq...
	$(V)if [ -n "$(VER_PREV)" ] ; then \
		echo "    Store previous version information ($(VER_PREV))"; \
		echo -n "$(VER_PREV)" > $(INST_LIB_PATH)/prev.txt ; \
	fi
	$(V)if [ -n "$(VER_CURRENT)" ] ; then \
		echo "    Store current version information ($(VER_CURRENT))"; \
		echo -n "$(VER_CURRENT)" > $(INST_LIB_PATH)/current.txt ; \
	fi
	$(V)if [ -n "$(VER_NEXT)" ] ; then \
		echo "    Store next version information ($(VER_NEXT))"; \
		echo -n "$(VER_NEXT)" > $(INST_LIB_PATH)/next.txt ; \
	fi
