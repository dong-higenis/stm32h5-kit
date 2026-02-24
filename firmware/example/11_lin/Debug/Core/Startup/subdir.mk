################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32h563vitx.s 

OBJS += \
./Core/Startup/startup_stm32h563vitx.o 

S_DEPS += \
./Core/Startup/startup_stm32h563vitx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m33 -g3 -DDEBUG -c -I"C:/work/git/STM32-H5-KIT/firmware/example/11_lin/Core/Inc" -I"C:/work/git/STM32-H5-KIT/firmware/example/11_lin/Core/Src" -I"C:/work/git/STM32-H5-KIT/firmware/example/11_lin/Core/Src/driver" -I"C:/work/git/STM32-H5-KIT/firmware/example/11_lin/Core/Inc/driver" -I"C:/work/git/STM32-H5-KIT/firmware/example/11_lin/Core/Src/driver/fatfs" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32h563vitx.d ./Core/Startup/startup_stm32h563vitx.o

.PHONY: clean-Core-2f-Startup

