#######################################################################
# Makefile for STM32F767 NUCLEO144 board projects

PROJECT = schwebekoerper
CMSIS_PATH ?= .
ACADO_PATH ?= .
OPENOCD_SCRIPT_DIR ?= /usr/share/openocd/scripts
HEAP_SIZE = 0x400

################
# Sources

SOURCES_S = ${CMSIS_PATH}/src/startup_stm32f767xx.s

SOURCES_C = ./src/main.c
SOURCES_C += ./src/acado_function.c
SOURCES_C += ${CMSIS_PATH}/src/system_stm32f7xx.c
SOURCES_C += ${CMSIS_PATH}/src/stm32f7xx_hal_msp.c
SOURCES_C += ${CMSIS_PATH}/src/stm32f7xx_it.c
SOURCES_C += sys/stubs.c sys/_sbrk.c sys/_io.c
SOURCES_C += ${CMSIS_PATH}/BSP/STM32F7xx_Nucleo_144/stm32f7xx_nucleo_144.c
SOURCES_C += ${CMSIS_PATH}/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c
SOURCES_C += ${CMSIS_PATH}/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c.c
SOURCES_C += ${CMSIS_PATH}/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c
SOURCES_C += ${CMSIS_PATH}/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
SOURCES_C += ${CMSIS_PATH}/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c
SOURCES_C += ${CMSIS_PATH}/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c
SOURCES_C += ${CMSIS_PATH}/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c
SOURCES_C += ${CMSIS_PATH}/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c
SOURCES_C += ${CMSIS_PATH}/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c
SOURCES_C += ${CMSIS_PATH}/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c
SOURCES_C += ${CMSIS_PATH}/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_usart.c


#ACADO Source files
ACADO_SOURCES_C = ${ACADO_PATH}/acado/acado_integrator.c \
	${ACADO_PATH}/acado/acado_solver.c \
	${ACADO_PATH}/acado/acado_auxiliary_functions.c \

	
ACADO_SOURCES_CPP = ./acado/qpoases/SRC/Bounds.cpp \
	./acado/qpoases/SRC/Constraints.cpp \
	./acado/qpoases/SRC/CyclingManager.cpp \
	./acado/qpoases/SRC/Indexlist.cpp \
	./acado/qpoases/SRC/MessageHandling.cpp \
	./acado/qpoases/SRC/QProblem.cpp \
	./acado/qpoases/SRC/QProblemB.cpp \
	./acado/qpoases/SRC/SubjectTo.cpp \
	./acado/qpoases/SRC/Utils.cpp \
	./acado/qpoases/SRC/EXTRAS/SolutionAnalysis.cpp \
	./acado/acado_qpoases_interface.cpp \
	
SOURCES_CPP =

SOURCES = $(SOURCES_S) $(SOURCES_C) $(SOURCES_CPP)
OBJS = $(SOURCES_S:.s=.o) $(SOURCES_C:.c=.o) $(SOURCES_CPP:.cpp=.o)
#ACADO objective files
ACADO_OBJ_C = $(ACADO_SOURCES_C:.c=.o)
ACADO_OBJ_CPP = $(ACADO_SOURCES_CPP:.cpp=.o)
################
# Includes and Defines
#ACADO include

#STM32F7 include
INCLUDES = -I src
INCLUDES += -I ${CMSIS_PATH}/Drivers/CMSIS/Include
INCLUDES += -I ${CMSIS_PATH}/src
INCLUDES += -I ${CMSIS_PATH}/Drivers/CMSIS/Device/Include
INCLUDES += -I ${CMSIS_PATH}/Inc
INCLUDES += -I ${CMSIS_PATH}/STM32F7xx_HAL_Driver/Inc
INCLUDES += -I ${CMSIS_PATH}/BSP/STM32F7xx_Nucleo_144
INCLUDES += -I ./acado 
INCLUDES += -I ./acado/qpoases/INCLUDE
INCLUDES += -I ./acado/qpoases/INCLUDE/EXTRAS
INCLUDES += -I ./acado/qpoases/SRC
INCLUDES += -I ./acado/qpoases


#Device Macro
DEFINES = -DSTM32 -DSTM32F7 -DSTM32F767xx

################
# Compiler/Assembler/Linker/etc

PREFIX = arm-none-eabi

CC = $(PREFIX)-gcc
AS = $(PREFIX)-as
AR = $(PREFIX)-ar
LD = $(PREFIX)-gcc
NM = $(PREFIX)-nm
OBJCOPY = $(PREFIX)-objcopy
OBJDUMP = $(PREFIX)-objdump
READELF = $(PREFIX)-readelf
SIZE = $(PREFIX)-size
GDB = $(PREFIX)-gdb
RM = rm -f
OPENOCD=openocd

################
# Compiler options
#core information
MCUFLAGS = -mcpu=cortex-m7 -mlittle-endian
MCUFLAGS += -mfloat-abi=hard -mfpu=fpv5-sp-d16
MCUFLAGS += -mthumb
# compile optimization
# DEBUG_OPTIMIZE_FLAGS = -O0 -g -gdwarf-5
DEBUG_OPTIMIZE_FLAGS = -O2
# c standard
CFLAGS = -std=c11
#all warning
CFLAGS += -Wall -Wextra --pedantic
# generate listing files
CFLAGS += -Wa,-aghlms=$(<:%.c=%.lst)
#heap size setting
CFLAGS += -DHEAP_SIZE=$(HEAP_SIZE)
CFLAGS += -fstack-usage

CFLAGS_EXTRA = -nostartfiles -nodefaultlibs -nostdlib
#data section and function section used
CFLAGS_EXTRA += -fdata-sections -ffunction-sections
CFLAGS += $(DEFINES) $(MCUFLAGS) $(DEBUG_OPTIMIZE_FLAGS) $(CFLAGS_EXTRA) $(INCLUDES)
#ACADO flags
#-g default debug information
ACADO_CFLAGS = -std=c11 -g -O2 -Wall 
ACADO_CFLAGS += $(MCUFLAGS) $(DEFINES) $(INCLUDES)
ACADO_CFLAGS += -fsingle-precision-constant --specs=nosys.specs  -fno-exceptions
#ACADO_CFLAGS +=--specs=nosys.specs  -fno-exceptions
#-c compile without linking
ACADO_CFLAGS += ${ACADO_INCLUDES} -c
#ACADO C++ flags
#-g default debug information
ACADO_CPPFLAGS = -g -O2 -Wall 
ACADO_CPPFLAGS += $(MCUFLAGS) $(DEFINES) $(INCLUDES)
# ACADO_CPPFLAGS += -fsingle-precision-constant --specs=nosys.specs  -fno-exceptions
ACADO_CPPFLAGS += --specs=nosys.specs  -fno-exceptions
#-c compile without linking
ACADO_CPPFLAGS += ${ACADO_INCLUDES} -c


LDFLAGS = -static $(MCUFLAGS)
#pass 'newlib.h' to the compiler optimize code size and enable float print
LDFLAGS += -specs=nano.specs -u _printf_float
#for retargeting
LDFLAGS += -specs=nosys.specs 
#iteratively link to static library
LDFLAGS += -Wl,--start-group -lgcc -lm -lc -lg -lstdc++ -lsupc++ -Wl,--end-group
LDFLAGS += -Wl,--gc-sections -Wl,--print-gc-sections -Wl,--cref,-Map=$(@:%.elf=%.map)
#dynamic linking
LDFLAGS += -L ${CMSIS_PATH}/ld/ -T STM32F767ZITx_FLASH.ld

################
# phony rules

.PHONY: all clean flash erase check_cube_exists

all: check_cube_exists $(PROJECT).bin $(PROJECT).hex $(PROJECT).asm

clean:
	$(RM) $(OBJS) $(ACADO_OBJ_C) $(OBJS:$.o=$.lst) $(OBJS:$.o=$.su) $(PROJECT).elf $(PROJECT).bin $(PROJECT).hex $(PROJECT).map $(PROJECT).asm

flash: $(PROJECT).bin
	st-flash write $(PROJECT).bin 0x08000000

erase:
	st-flash erase

check_cube_exists:
	if ! test -e ${CMSIS_PATH}; then echo 'please download and extract or symlink the STM32-CUBE-F7 pack (>= 1.6.0) to ${CMSIS_PATH}'; false; fi

#not used yet
GDB_PORT:=4242
gdb-server:
	# st-util will listen on port :4242
	st-util
	# openocd will listen on port :3333
	#$(OPENOCD) -f $(OPENOCD_SCRIPT_DIR)/interface/stlink-v2-1.cfg -f $(OPENOCD_SCRIPT_DIR)/target/stm32f7x.cfg

gdb: $(PROJECT).elf
	# $(GDB) --eval-command="target extended-remote localhost:$(GDB_PORT)" --eval-command="monitor halt" $(PROJECT).elf
	$(GDB) --eval-command="target extended-remote localhost:$(GDB_PORT)" --eval-command="monitor halt" $(PROJECT).elf

################
# dependency graphs for wildcard rules

$(PROJECT).elf: $(OBJS) $(ACADO_OBJ_C) $(ACADO_OBJ_CPP)
# acado_make: $(ACADO_OBJ_C) $(ACADO_OBJ_CPP)

acado_make_cpp:$(ACADO_OBJ_CPP)
$(ACADO_OBJ_CPP): $(ACADO_SRC_CPP)
	$(CC) $(ACADO_CPPFLAGS)  $(@:.o=.cpp) -o $@	
acado_make_c: $(ACADO_OBJ_C)
$(ACADO_OBJ_C): $(ACADO_SOURCES_C)
	$(CC) $(ACADO_CFLAGS) $(@:.o=.c) -o $@
################
# wildcard rules

%.elf:
	$(LD) $(OBJS) $(ACADO_OBJ_C) $(ACADO_OBJ_CPP) $(LDFLAGS) -o $@
	$(SIZE) -A $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.asm: %.elf
	$(OBJDUMP) -dgCxwsSh --show-raw-insn $< > $@
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
# EOF
