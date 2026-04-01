################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/apps/mqtt/mqtt.c 

OBJS += \
./src/apps/mqtt/mqtt.o 

C_DEPS += \
./src/apps/mqtt/mqtt.d 


# Each subdirectory must supply rules for building sources it contributes
src/apps/mqtt/%.o src/apps/mqtt/%.su src/apps/mqtt/%.cyclo: ../src/apps/mqtt/%.c src/apps/mqtt/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/ap" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App" -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/hw" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/hw/include" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/core" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw/driver/lcd" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/hw/include/lcd" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw/driver/lcd/driver" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw/driver/lcd/font" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/lwip" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/Drivers/BSP" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/lwip/app" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/lwip/target" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/src" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/src/include" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/system" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/system/arch" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/src/core" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-src-2f-apps-2f-mqtt

clean-src-2f-apps-2f-mqtt:
	-$(RM) ./src/apps/mqtt/mqtt.cyclo ./src/apps/mqtt/mqtt.d ./src/apps/mqtt/mqtt.o ./src/apps/mqtt/mqtt.su

.PHONY: clean-src-2f-apps-2f-mqtt

