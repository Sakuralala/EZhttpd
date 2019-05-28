#TODO:头文件修改时不会更新
# tool marcros
CC := clang++
CCFLAG :=  -fno-limit-debug-info -fno-elide-constructors -std=c++17 -pthread
DBGFLAG := -g
CCOBJFLAG := $(CCFLAG) -c

# path marcros
BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH1 := bases
SRC_PATH2 := event
SRC_PATH3 := log
SRC_PATH4 := net
SRC_PATH5 := toys
DEBUG_PATH := debug

# compile marcros
TARGET_NAME := main
ifeq ($(OS),Windows_NT)
	TARGET_NAME := $(addsuffix .exe,$(TARGET_NAME))
endif
TARGET := $(BIN_PATH)/$(TARGET_NAME)
TARGET_DEBUG := $(DEBUG_PATH)/$(TARGET_NAME)
#MAIN_SRC := main.cpp

# src files & obj files
SRC1 := $(foreach x, $(SRC_PATH1), $(wildcard $(addprefix $(x)/*,.c*)))
SRC2 := $(foreach x, $(SRC_PATH2), $(wildcard $(addprefix $(x)/*,.c*)))
SRC3 := $(foreach x, $(SRC_PATH3), $(wildcard $(addprefix $(x)/*,.c*)))
SRC4 := $(foreach x, $(SRC_PATH4), $(wildcard $(addprefix $(x)/*,.c*)))
SRC5 := $(foreach x, $(SRC_PATH5), $(wildcard $(addprefix $(x)/*,.c*)))
SRC := $(SRC1) $(SRC2) $(SRC3) $(SRC4)  $(SRC5) main.cpp
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))
OBJ_DEBUG := $(addprefix $(DEBUG_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))

# clean files list
DISTCLEAN_LIST := $(OBJ) \
                  $(OBJ_DEBUG)
CLEAN_LIST := $(TARGET) \
			  $(TARGET_DEBUG) \
			  $(DISTCLEAN_LIST)

# default rule
default: all

# non-phony targets
$(TARGET): $(OBJ)
	$(CC) $(CCFLAG) -o $@ $^
 
$(OBJ_PATH)/main.o: main.cpp 
	$(CC) $(CCOBJFLAG) -o $@ $^
$(OBJ_PATH)/%.o: $(SRC_PATH1)/%.c* 
	$(CC) $(CCOBJFLAG) -o $@ $^
$(OBJ_PATH)/%.o: $(SRC_PATH2)/%.c* 
	$(CC) $(CCOBJFLAG) -o $@ $^
$(OBJ_PATH)/%.o: $(SRC_PATH3)/%.c* 
	$(CC) $(CCOBJFLAG) -o $@ $^
$(OBJ_PATH)/%.o: $(SRC_PATH4)/%.c* 
	$(CC) $(CCOBJFLAG) -o $@ $^
$(OBJ_PATH)/%.o: $(SRC_PATH5)/%.c* 
	$(CC) $(CCOBJFLAG) -o $@ $^


$(DEBUG_PATH)/main.o: main.cpp
	$(CC) $(CCOBJFLAG) $(DBGFLAG) -o $@ $^
$(DEBUG_PATH)/%.o: $(SRC_PATH1)/%.c*
	$(CC) $(CCOBJFLAG) $(DBGFLAG) -o $@ $^
$(DEBUG_PATH)/%.o: $(SRC_PATH2)/%.c*
	$(CC) $(CCOBJFLAG) $(DBGFLAG) -o $@ $^
$(DEBUG_PATH)/%.o: $(SRC_PATH3)/%.c*
	$(CC) $(CCOBJFLAG) $(DBGFLAG) -o $@ $^
$(DEBUG_PATH)/%.o: $(SRC_PATH4)/%.c*
	$(CC) $(CCOBJFLAG) $(DBGFLAG) -o $@ $^
$(DEBUG_PATH)/%.o: $(SRC_PATH5)/%.c*
	$(CC) $(CCOBJFLAG) $(DBGFLAG) -o $@ $^

$(TARGET_DEBUG): $(OBJ_DEBUG)
	$(CC) $(CCFLAG) $(DBGFLAG)  -o $@ $^
# phony rules
.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: $(TARGET_DEBUG)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

.PHONY: distclean
distclean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(DISTCLEAN_LIST)