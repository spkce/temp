#导入系统的Makefile全局变量
-include $(ROOT_PATH)/build/Config.mk

C_FLAGS += -lstdc++ -std=c++11 -lpthread #-ldl -lm -lrt

#添加模块私有的编译参数
MODULE_FLAGS := -I include 
#-DNET_TERMINAL 

#添加模块子目录，不设置则只编译当前目录的文件
MODULE_SUB_DIR := 

#添加模块子目录的个别文件
MODULE_SUB_DIR_FILE := 

#导入系统的Makefile
-include $(ROOT_PATH)/build/Module.mk
