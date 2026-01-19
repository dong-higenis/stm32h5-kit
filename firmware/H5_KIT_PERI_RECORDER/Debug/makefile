################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/netif/ppp/subdir.mk
-include src/netif/subdir.mk
-include src/core/ipv6/subdir.mk
-include src/core/ipv4/subdir.mk
-include src/core/subdir.mk
-include src/apps/mqtt/subdir.mk
-include src/api/subdir.mk
-include lwip/target/subdir.mk
-include lwip/app/subdir.mk
-include Drivers/STM32H5xx_HAL_Driver/Src/subdir.mk
-include Drivers/BSP/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include App/hw/driver/lcd/subdir.mk
-include App/hw/driver/fatfs/subdir.mk
-include App/hw/driver/subdir.mk
-include App/hw/subdir.mk
-include App/common/hw/src/subdir.mk
-include App/common/core/subdir.mk
-include App/ap/toy/ui/subdir.mk
-include App/ap/toy/peripheral/uart/subdir.mk
-include App/ap/toy/peripheral/rs485/subdir.mk
-include App/ap/toy/peripheral/can/subdir.mk
-include App/ap/toy/subdir.mk
-include App/ap/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := H5_KIT_TOY
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
H5_KIT_TOY.elf \

MAP_FILES += \
H5_KIT_TOY.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
H5_KIT_TOY.list \


# All Target
all: main-build

# Main-build Target
main-build: H5_KIT_TOY.elf secondary-outputs

# Tool invocations
H5_KIT_TOY.elf H5_KIT_TOY.map: $(OBJS) $(USER_OBJS) C:\Users\user\Desktop\projects\touchgfx\H5_KIT_TOY\STM32H563VITX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "H5_KIT_TOY.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m33 -T"C:\Users\user\Desktop\projects\touchgfx\H5_KIT_TOY\STM32H563VITX_FLASH.ld" --specs=nosys.specs -Wl,-Map="H5_KIT_TOY.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

H5_KIT_TOY.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "H5_KIT_TOY.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) H5_KIT_TOY.elf H5_KIT_TOY.list H5_KIT_TOY.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
