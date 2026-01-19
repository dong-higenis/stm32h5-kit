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
	arm-none-eabi-gcc -mcpu=cortex-m33 -g3 -DDEBUG -c -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/hw" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/ap" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/common" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App" -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/common/hw" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/common/hw/include" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/common/core" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/hw/driver/lcd" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/common/hw/include/lcd" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/hw/driver/lcd/driver" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/hw/driver/lcd/font" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/lwip" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/Drivers/BSP" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/lwip/app" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/lwip/target" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/src" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/src/include" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/system" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/system/arch" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/src/core" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/ap/toy" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/hw/driver/fatfs" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/ap/toy/peripheral" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/ap/toy/ui" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/ap/toy/peripheral/can" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32h563vitx.d ./Core/Startup/startup_stm32h563vitx.o

.PHONY: clean-Core-2f-Startup

