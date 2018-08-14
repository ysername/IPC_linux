#
#                                  Yazaki North and Central America
#
#    Filename: Makefile
#  Description: Main maefile for SPI lib migration project
#
#   Created on: Jan 4, 2018
#   Author: Sergio Espinoza
#
#  2018, Yazaki North and Central America


 
DEBUG="y"

#VARIABLES
TARGET_BIN_NAME=HMI
# SPI_LIB_NAME=libSPI.a
LIB_OPTS=rcs
DIR_LIST+=$(OBJ_DIR)
TOOLCHAIN_DIR=
TOOLCHAIN_PREFIX=

#DIRECTORIES
ROOT_DIR=..
INCLUDE_DIRS=$(ROOT_DIR)/../include
SRC_DIR=$(ROOT_DIR)/src
OBJ_DIR=$(ROOT_DIR)/obj

#LIBRARYS
LIBS+=$(SPI_LIB_NAME)

#FLAGS
DEBUG_FLAGS=-g -O0 -I$(INCLUDE_DIRS) 
RELEASE_FLAGS= -I$(INCLUDE_DIRS)

#TOOLS
CC=gcc
LD=ld
AR=ar
MKDIR=mkdir
RM=rm
MV=mv

#COMPILATION-TIME SWITCHES
ifeq ($(DEBUG),"y")
C_FLAGS=$(DEBUG_FLAGS)  -lrt -pthread
        # -D_LARGEFILE64_SOURCE
        # -D__MSVCRT__
else
C_FLAGS=$(RELEASE_FLAGS)
endif

# Application objects
#OBJS+=main.o 
OBJS+=msg_buf.o
OBJS+=msg_fcn.o
OBJS+=gp_utils.o
OBJS+=clk_api_linux.o
OBJS+=Datapool.o
#OBJS+=spi_buf.o
#OBJS+=spi_lib.o
#OBJS+=gp_utils.o

#OBJS+=Ipc_mgr_as.o
OBJS+=hmi_ss.o
OBJS+=msg_api_signals.o
OBJS+=Hmi_mgr_as.o
OBJS+=Hmi_mgr_as_worktask1.o
OBJS+=Hmi_demo.o
#OBJS+=Hmi_demo.o
# OBJS+=cmd_conn.o
#OBJS+=msg_fcn.o
# OBJS+=imx6_spi_iodevice.o
#OBJS+=crc32.o
# OBJS+=md5.o
OBJS_REQ=$(OBJS:%.o=$(OBJ_DIR)/%.o)

# SPI LIB objs
LIB_OBJS+=spi_buf.o
LIB_OBJS+=spi_lib.o
LIB_OBJS_REQ=$(LIB_OBJS:%.o=$(OBJ_DIR)/%.o)

.DEFAULT:TARGETS
TARGETS: dirs $(LIBS) $(TARGET_BIN_NAME)
	echo "build finished!"	

#### TARGETS ####

$(TARGET_BIN_NAME):  $(OBJS_REQ) $(LIBS)
	$(CC)  $(C_FLAGS) $(LIBS) $(OBJS_REQ) -o $@  -lrt -pthread

# $(SPI_LIB_NAME): $(LIB_OBJS_REQ)
# $(AR) $(LIB_OPTS) $@ $^ 

$(OBJ_DIR):
	$(MKDIR) -p $(DIR_LIST)		


.PHONY:dirs
dirs:
	$(MKDIR) -p $(DIR_LIST)

.PHONY:clean
clean:
	$(RM) -R $(DIR_LIST)
	$(RM) $(TARGET_BIN_NAME)


################ IMPLICIT RULES ######################

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(C_FLAGS) $< -o $@ 
