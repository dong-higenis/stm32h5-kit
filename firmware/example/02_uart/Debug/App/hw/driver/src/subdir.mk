################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/hw/driver/src/gpio.c \
../App/hw/driver/src/led.c \
../App/hw/driver/src/uart.c 

OBJS += \
./App/hw/driver/src/gpio.o \
./App/hw/driver/src/led.o \
./App/hw/driver/src/uart.o 

C_DEPS += \
./App/hw/driver/src/gpio.d \
./App/hw/driver/src/led.d \
./App/hw/driver/src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
App/hw/driver/src/%.o App/hw/driver/src/%.su App/hw/driver/src/%.cyclo: ../App/hw/driver/src/%.c App/hw/driver/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/02_uart/App" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/02_uart/App/ap" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/02_uart/App/common" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/02_uart/App/hw" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/02_uart/App/hw/driver/include" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/02_uart/App/hw/driver/src" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/02_uart/App/hw/driver" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/02_uart/Core/Src" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/02_uart/App/common/core" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-hw-2f-driver-2f-src

clean-App-2f-hw-2f-driver-2f-src:
	-$(RM) ./App/hw/driver/src/gpio.cyclo ./App/hw/driver/src/gpio.d ./App/hw/driver/src/gpio.o ./App/hw/driver/src/gpio.su ./App/hw/driver/src/led.cyclo ./App/hw/driver/src/led.d ./App/hw/driver/src/led.o ./App/hw/driver/src/led.su ./App/hw/driver/src/uart.cyclo ./App/hw/driver/src/uart.d ./App/hw/driver/src/uart.o ./App/hw/driver/src/uart.su

.PHONY: clean-App-2f-hw-2f-driver-2f-src

