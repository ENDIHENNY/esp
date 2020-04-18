include ../../common/systemc.mk

FX_WIDTH ?= 32
FX32_IL   = 12
FX64_IL   = 42
TYPEDEF ?= 1 

ifeq ("$(FX_WIDTH)", "")
$(error please define the desired FX_WIDTH for simulation)
endif

ifeq ("$(FX32_IL)", "")
$(error please define the desired FX32_IL for simulation)
endif

ifeq ("$(FX64_IL)", "")
$(error please define the desired FX64_IL for simulation)
endif


CXXFLAGS += -std=c++11
CXXFLAGS += -DFX_WIDTH=$(FX_WIDTH)
CXXFLAGS += -DFX32_IL=$(FX32_IL)
CXXFLAGS += -DFX64_IL=$(FX64_IL)
CXXFLAGS += -DTYPEDEF=$(TYPEDEF)


