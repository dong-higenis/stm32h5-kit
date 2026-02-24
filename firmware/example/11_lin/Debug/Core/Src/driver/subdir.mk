################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/driver/ssd1306.c 

OBJS += \
./Core/Src/driver/ssd1306.o 

C_DEPS += \
./Core/Src/driver/ssd1306.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/driver/%.o Core/Src/driver/%.su Core/Src/driver/%.cyclo: ../Core/Src/driver/%.c Core/Src/driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/work/git/STM32-H5-KIT/firmware/example/11_lin/Core/Src" -I"C:/work/git/STM32-H5-KIT/firmware/example/11_lin/Core/Src/driver" -I"C:/work/git/STM32-H5-KIT/firmware/example/11_lin/Core/Inc/driver" -I"C:/work/git/STM32-H5-KIT/firmware/example/11_lin/Core/Src/driver/fatfs" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-driver

clean-Core-2f-Src-2f-driver:
	-$(RM) ./Core/Src/driver/ssd1306.cyclo ./Core/Src/driver/ssd1306.d ./Core/Src/driver/ssd1306.o ./Core/Src/driver/ssd1306.su

.PHONY: clean-Core-2f-Src-2f-driver

