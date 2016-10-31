################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_15.12.1.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./Datalogger.obj" \
"./HDC1000.obj" \
"./OPT3001.obj" \
"./RF430CL331H.obj" \
"./RF430_I2C.obj" \
"./RF430_Request_Processor.obj" \
"./RTC.obj" \
"./TIME.obj" \
"./TMP112.obj" \
"./main.obj" \
"./system_pre_init.obj" \
"../targetConfigs/lnk_msp430fr5969.cmd" \
$(GEN_CMDS__FLAG) \
-llibmpu_init.a \
-llibc.a \
-llibmath.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include targetConfigs/subdir_vars.mk
-include subdir_rules.mk
-include targetConfigs/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
Cold_Chain_Demo_v1_00.out \

EXE_OUTPUTS__QUOTED += \
"Cold_Chain_Demo_v1_00.out" \

BIN_OUTPUTS += \
Cold_Chain_Demo_v1_00.hex \

BIN_OUTPUTS__QUOTED += \
"Cold_Chain_Demo_v1_00.hex" \


# All Target
all: Cold_Chain_Demo_v1_00.out secondary-outputs

# Tool invocations
Cold_Chain_Demo_v1_00.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: MSP430 Linker'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_15.12.1.LTS/bin/cl430" -vmspx --data_model=large --code_model=large -O0 --use_hw_mpy=F5 --advice:power="all" --advice:hw_config=all -g --define=__MSP430FR5969__ --define=_MPU_ENABLE --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --abi=eabi --printf_support=minimal -z -m"Cold_Chain_Demo_v1_00.map" --stack_size=160 --heap_size=160 --cinit_hold_wdt=on -i"C:/ti/ccsv6/ccs_base/msp430/include" -i"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_15.12.1.LTS/lib" -i"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_15.12.1.LTS/include" -i"C:/ti/ccsv6/ccs_base/msp430/lib" -i"C:/ti/ccsv6/ccs_base/msp430/lib/5xx_6xx_FRxx" -i"C:/ti/ccsv6/ccs_base/msp430/lib/FR59xx" --priority --reread_libs --define=_MPU_ENABLE --warn_sections --display_error_number --diag_wrap=off --xml_link_info="Cold_Chain_Demo_v1_00_linkInfo.xml" --use_hw_mpy=F5 --rom_model -o "Cold_Chain_Demo_v1_00.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

Cold_Chain_Demo_v1_00.hex: $(EXE_OUTPUTS)
	@echo 'Invoking: MSP430 Hex Utility'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_15.12.1.LTS/bin/hex430" --memwidth=8 --order=MS --romwidth=16 --intel -o "Cold_Chain_Demo_v1_00.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)$(BIN_OUTPUTS__QUOTED)
	-$(RM) "Datalogger.d" "HDC1000.d" "OPT3001.d" "RF430CL331H.d" "RF430_I2C.d" "RF430_Request_Processor.d" "RTC.d" "TIME.d" "TMP112.d" "main.d" "system_pre_init.d" 
	-$(RM) "Datalogger.obj" "HDC1000.obj" "OPT3001.obj" "RF430CL331H.obj" "RF430_I2C.obj" "RF430_Request_Processor.obj" "RTC.obj" "TIME.obj" "TMP112.obj" "main.obj" "system_pre_init.obj" 
	-@echo 'Finished clean'
	-@echo ' '

secondary-outputs: $(BIN_OUTPUTS)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

