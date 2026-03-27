################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/app/lwip.c 

OBJS += \
./lwip/app/lwip.o 

C_DEPS += \
./lwip/app/lwip.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/app/%.o lwip/app/%.su lwip/app/%.cyclo: ../lwip/app/%.c lwip/app/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/hw" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/ap" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/common" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App" -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/common/hw" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/common/hw/include" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/common/core" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/hw/driver/lcd" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/common/hw/include/lcd" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/hw/driver/lcd/driver" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/hw/driver/lcd/font" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/lwip" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/Drivers/BSP" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/lwip/app" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/lwip/target" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/src" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/src/include" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/system" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/system/arch" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/src/core" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-app

clean-lwip-2f-app:
	-$(RM) ./lwip/app/lwip.cyclo ./lwip/app/lwip.d ./lwip/app/lwip.o ./lwip/app/lwip.su

.PHONY: clean-lwip-2f-app

