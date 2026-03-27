################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/hw/driver/button.c \
../App/hw/driver/buzzer.c \
../App/hw/driver/can.c \
../App/hw/driver/etnet.c \
../App/hw/driver/fram.c \
../App/hw/driver/gpio.c \
../App/hw/driver/i2c.c \
../App/hw/driver/lcd.c \
../App/hw/driver/led.c \
../App/hw/driver/log.c \
../App/hw/driver/pwm.c \
../App/hw/driver/reset.c \
../App/hw/driver/resize.c \
../App/hw/driver/rtc.c \
../App/hw/driver/sd.c \
../App/hw/driver/spi.c \
../App/hw/driver/spi_flash.c \
../App/hw/driver/swtimer.c \
../App/hw/driver/uart.c 

OBJS += \
./App/hw/driver/button.o \
./App/hw/driver/buzzer.o \
./App/hw/driver/can.o \
./App/hw/driver/etnet.o \
./App/hw/driver/fram.o \
./App/hw/driver/gpio.o \
./App/hw/driver/i2c.o \
./App/hw/driver/lcd.o \
./App/hw/driver/led.o \
./App/hw/driver/log.o \
./App/hw/driver/pwm.o \
./App/hw/driver/reset.o \
./App/hw/driver/resize.o \
./App/hw/driver/rtc.o \
./App/hw/driver/sd.o \
./App/hw/driver/spi.o \
./App/hw/driver/spi_flash.o \
./App/hw/driver/swtimer.o \
./App/hw/driver/uart.o 

C_DEPS += \
./App/hw/driver/button.d \
./App/hw/driver/buzzer.d \
./App/hw/driver/can.d \
./App/hw/driver/etnet.d \
./App/hw/driver/fram.d \
./App/hw/driver/gpio.d \
./App/hw/driver/i2c.d \
./App/hw/driver/lcd.d \
./App/hw/driver/led.d \
./App/hw/driver/log.d \
./App/hw/driver/pwm.d \
./App/hw/driver/reset.d \
./App/hw/driver/resize.d \
./App/hw/driver/rtc.d \
./App/hw/driver/sd.d \
./App/hw/driver/spi.d \
./App/hw/driver/spi_flash.d \
./App/hw/driver/swtimer.d \
./App/hw/driver/uart.d 


# Each subdirectory must supply rules for building sources it contributes
App/hw/driver/%.o App/hw/driver/%.su App/hw/driver/%.cyclo: ../App/hw/driver/%.c App/hw/driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/hw" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/ap" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/common" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App" -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/common/hw" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/common/hw/include" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/common/core" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/hw/driver/lcd" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/common/hw/include/lcd" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/hw/driver/lcd/driver" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/App/hw/driver/lcd/font" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/lwip" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/Drivers/BSP" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/lwip/app" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/lwip/target" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/src" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/src/include" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/system" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/system/arch" -I"C:/Users/user/Desktop/projects/STM/touchgfx/H5_KIT_Test/src/core" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-hw-2f-driver

clean-App-2f-hw-2f-driver:
	-$(RM) ./App/hw/driver/button.cyclo ./App/hw/driver/button.d ./App/hw/driver/button.o ./App/hw/driver/button.su ./App/hw/driver/buzzer.cyclo ./App/hw/driver/buzzer.d ./App/hw/driver/buzzer.o ./App/hw/driver/buzzer.su ./App/hw/driver/can.cyclo ./App/hw/driver/can.d ./App/hw/driver/can.o ./App/hw/driver/can.su ./App/hw/driver/etnet.cyclo ./App/hw/driver/etnet.d ./App/hw/driver/etnet.o ./App/hw/driver/etnet.su ./App/hw/driver/fram.cyclo ./App/hw/driver/fram.d ./App/hw/driver/fram.o ./App/hw/driver/fram.su ./App/hw/driver/gpio.cyclo ./App/hw/driver/gpio.d ./App/hw/driver/gpio.o ./App/hw/driver/gpio.su ./App/hw/driver/i2c.cyclo ./App/hw/driver/i2c.d ./App/hw/driver/i2c.o ./App/hw/driver/i2c.su ./App/hw/driver/lcd.cyclo ./App/hw/driver/lcd.d ./App/hw/driver/lcd.o ./App/hw/driver/lcd.su ./App/hw/driver/led.cyclo ./App/hw/driver/led.d ./App/hw/driver/led.o ./App/hw/driver/led.su ./App/hw/driver/log.cyclo ./App/hw/driver/log.d ./App/hw/driver/log.o ./App/hw/driver/log.su ./App/hw/driver/pwm.cyclo ./App/hw/driver/pwm.d ./App/hw/driver/pwm.o ./App/hw/driver/pwm.su ./App/hw/driver/reset.cyclo ./App/hw/driver/reset.d ./App/hw/driver/reset.o ./App/hw/driver/reset.su ./App/hw/driver/resize.cyclo ./App/hw/driver/resize.d ./App/hw/driver/resize.o ./App/hw/driver/resize.su ./App/hw/driver/rtc.cyclo ./App/hw/driver/rtc.d ./App/hw/driver/rtc.o ./App/hw/driver/rtc.su ./App/hw/driver/sd.cyclo ./App/hw/driver/sd.d ./App/hw/driver/sd.o ./App/hw/driver/sd.su ./App/hw/driver/spi.cyclo ./App/hw/driver/spi.d ./App/hw/driver/spi.o ./App/hw/driver/spi.su ./App/hw/driver/spi_flash.cyclo ./App/hw/driver/spi_flash.d ./App/hw/driver/spi_flash.o ./App/hw/driver/spi_flash.su ./App/hw/driver/swtimer.cyclo ./App/hw/driver/swtimer.d ./App/hw/driver/swtimer.o ./App/hw/driver/swtimer.su ./App/hw/driver/uart.cyclo ./App/hw/driver/uart.d ./App/hw/driver/uart.o ./App/hw/driver/uart.su

.PHONY: clean-App-2f-hw-2f-driver

