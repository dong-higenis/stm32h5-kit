################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/common/core/qbuffer.c \
../App/common/core/util.c 

OBJS += \
./App/common/core/qbuffer.o \
./App/common/core/util.o 

C_DEPS += \
./App/common/core/qbuffer.d \
./App/common/core/util.d 


# Each subdirectory must supply rules for building sources it contributes
App/common/core/%.o App/common/core/%.su App/common/core/%.cyclo: ../App/common/core/%.c App/common/core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/ap" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App" -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/hw" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/hw/include" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/core" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw/driver/lcd" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/hw/include/lcd" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw/driver/lcd/driver" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw/driver/lcd/font" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/lwip" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/Drivers/BSP" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/lwip/app" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/lwip/target" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/src" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/src/include" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/system" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/system/arch" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/src/core" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-common-2f-core

clean-App-2f-common-2f-core:
	-$(RM) ./App/common/core/qbuffer.cyclo ./App/common/core/qbuffer.d ./App/common/core/qbuffer.o ./App/common/core/qbuffer.su ./App/common/core/util.cyclo ./App/common/core/util.d ./App/common/core/util.o ./App/common/core/util.su

.PHONY: clean-App-2f-common-2f-core

