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
	arm-none-eabi-gcc -mcpu=cortex-m33 -g3 -DDEBUG -c -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/ap" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App" -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/hw" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/hw/include" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/core" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw/driver/lcd" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/hw/include/lcd" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw/driver/lcd/driver" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw/driver/lcd/font" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/lwip" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/Drivers/BSP" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/lwip/app" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/lwip/target" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/src" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/src/include" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/system" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/system/arch" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/src/core" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32h563vitx.d ./Core/Startup/startup_stm32h563vitx.o

.PHONY: clean-Core-2f-Startup

