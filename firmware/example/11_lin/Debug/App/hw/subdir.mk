################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/hw/hw.c 

OBJS += \
./App/hw/hw.o 

C_DEPS += \
./App/hw/hw.d 


# Each subdirectory must supply rules for building sources it contributes
App/hw/%.o App/hw/%.su App/hw/%.cyclo: ../App/hw/%.c App/hw/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/work/git/STM32-H5-KIT/firmware/example/02_uart/App" -I"C:/work/git/STM32-H5-KIT/firmware/example/02_uart/App/ap" -I"C:/work/git/STM32-H5-KIT/firmware/example/02_uart/App/common" -I"C:/work/git/STM32-H5-KIT/firmware/example/02_uart/App/hw" -I"C:/work/git/STM32-H5-KIT/firmware/example/02_uart/App/hw/driver/include" -I"C:/work/git/STM32-H5-KIT/firmware/example/02_uart/App/hw/driver/src" -I"C:/work/git/STM32-H5-KIT/firmware/example/02_uart/App/hw/driver" -I"C:/work/git/STM32-H5-KIT/firmware/example/02_uart/Core/Src" -I"C:/work/git/STM32-H5-KIT/firmware/example/02_uart/App/common/core" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-hw

clean-App-2f-hw:
	-$(RM) ./App/hw/hw.cyclo ./App/hw/hw.d ./App/hw/hw.o ./App/hw/hw.su

.PHONY: clean-App-2f-hw

