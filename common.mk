THIS_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

OSNAME=${shell uname -s}

ifeq ($(OSNAME),Darwin)
$(THIS_DIR)cc_cache.gen.mk : $(THIS_DIR)scripts/gen_tool_defs_macos.sh
	@ $(THIS_DIR)scripts/gen_tool_defs_macos.sh > $@
else
$(THIS_DIR)cc_cache.gen.mk : $(THIS_DIR)scripts/gen_tool_defs_linux.sh
	@ $(THIS_DIR)scripts/gen_tool_defs_linux.sh > $@
endif

include $(THIS_DIR)cc_cache.gen.mk
CLANG_SYSTEM_INC_PATH=$(shell $(THIS_DIR)./scripts/get_clang_builtin_include_dir.sh $(LLVM_CXX))
LIUM_NCPU?=1

dump_config : 
	@ cat $(THIS_DIR)cc_cache.gen.mk
	@ echo CLANG_SYSTEM_INC_PATH=$(CLANG_SYSTEM_INC_PATH)
