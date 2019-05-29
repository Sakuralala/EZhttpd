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
TARGET_NAME1 := server_test
TARGET_NAME2 := client_test
ifeq ($(OS),Windows_NT)
	TARGET_NAME := $(addsuffix .exe,$(TARGET_NAME))
endif
TARGET1 := $(BIN_PATH)/$(TARGET_NAME1)
TARGET2 := $(BIN_PATH)/$(TARGET_NAME2)
TARGET_DEBUG1 := $(DEBUG_PATH)/$(TARGET_NAME1)
TARGET_DEBUG2 := $(DEBUG_PATH)/$(TARGET_NAME2)
#server_test_SRC := server_test.cpp

# src files & obj files
SRC1 := $(foreach x, $(SRC_PATH1), $(wildcard $(addprefix $(x)/*,.c*)))
SRC2 := $(foreach x, $(SRC_PATH2), $(wildcard $(addprefix $(x)/*,.c*)))
SRC3 := $(foreach x, $(SRC_PATH3), $(wildcard $(addprefix $(x)/*,.c*)))
SRC4 := $(foreach x, $(SRC_PATH4), $(wildcard $(addprefix $(x)/*,.c*)))
SRC5 := $(foreach x, $(SRC_PATH5), $(wildcard $(addprefix $(x)/*,.c*)))
SRC_TOTAL1 := $(SRC1) $(SRC2) $(SRC3) $(SRC4)  $(SRC5) server_test.cpp
SRC_TOTAL2 := $(SRC1) $(SRC2) $(SRC3) $(SRC4)  $(SRC5) client_test.cpp
OBJ1 := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC_TOTAL1)))))
OBJ_DEBUG1 := $(addprefix $(DEBUG_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC_TOTAL1)))))
OBJ2 := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC_TOTAL2)))))
OBJ_DEBUG2 := $(addprefix $(DEBUG_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC_TOTAL2)))))

# clean files list
DISTCLEAN_LIST := $(OBJ1) $(OBJ2) \
                  $(OBJ_DEBUG2) $(OBJ_DEBUG1)
CLEAN_LIST := $(TARGET1) $(TARGET2) \
			  $(TARGET_DEBUG1)  $(TARGET_DEBUG2) \
			  $(DISTCLEAN_LIST)

# default rule
default: all
# non-phony targets
$(TARGET1): $(OBJ1)
	$(CC) $(CCFLAG) -o $@ $^

$(TARGET2): $(OBJ2)
	$(CC) $(CCFLAG) -o $@ $^
 
$(OBJ_PATH)/client_test.o: client_test.cpp 
	$(CC) $(CCOBJFLAG) -o $@ $^
$(OBJ_PATH)/server_test.o: server_test.cpp 
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


$(DEBUG_PATH)/client_test.o: client_test.cpp
	$(CC) $(CCOBJFLAG) $(DBGFLAG) -o $@ $^
$(DEBUG_PATH)/server_test.o: server_test.cpp
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

$(TARGET_DEBUG1): $(OBJ_DEBUG1)
	$(CC) $(CCFLAG) $(DBGFLAG)  -o $@ $^
$(TARGET_DEBUG2): $(OBJ_DEBUG2)
	$(CC) $(CCFLAG) $(DBGFLAG)  -o $@ $^
# phony rules
.PHONY: all
all: $(TARGET1) $(TARGET2)

.PHONY: debug
debug: $(TARGET_DEBUG1) $(TARGET_DEBUG2)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

.PHONY: distclean
distclean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(DISTCLEAN_LIST)