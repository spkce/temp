ROOT_DIR = $(shell pwd)
DIR_LIB ?=  $(ROOT_DIR)/lib
#DIR_INC ?
CC ?= gcc
CXX ?= g++
AR ?= ar

INC := -I include -I include/infra -I include/jsoncpp -I src -I src/net 

CFLAGS = -L./lib -Wall  -lpthread -lrt -lstdc++ -std=c++11 -ldl -g

SRC_DIR += \
./		\
./src \
./src/net \
./library/infra \
./library/jsoncpp \


DIR_OBJ := ./obj
TARGET := a.out

.PHONY: all CHECKDIR clean 

all: CHECKDIR $(TARGET)

CPP_SRCS += $(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.cpp))
C_SRCS += $(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.c))

CPP_OBJS :=  $(patsubst %.cpp, %.o, $(CPP_SRCS))
C_OBJS :=  $(patsubst %.c, %.o, $(C_SRCS))


COBJS   := $(addprefix $(DIR_OBJ)/, $(C_OBJS))
CPPOBJS := $(addprefix $(DIR_OBJ)/, $(CPP_OBJS))

$(COBJS): $(DIR_OBJ)/%.o: %.c
	@echo "CC $(notdir $@)"
	@mkdir -p $(dir $(@))
	@$(CC) $(CFLAGS) -o $@ -c $< $(INC)
	
$(CPPOBJS): $(DIR_OBJ)/%.o: %.cpp
	@echo "CC $(notdir $@)"
	@mkdir -p $(dir $(@))
	@$(CXX) $(CFLAGS) -o $@ -c $< $(INC)

$(TARGET): $(COBJS) $(CPPOBJS)
	@$(CC) $(COBJS) $(CPPOBJS) -g -o $@ $(INC) $(CFLAGS) 


CHECKDIR:
	@mkdir -p $(DIR_OBJ)
	

clean:
	@rm -rf ${DIR_OBJ}
ifeq ($(TARGET), $(wildcard $(TARGET)))
	@rm $(TARGET)
endif