################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/ap/ap.c 

OBJS += \
./App/ap/ap.o 

C_DEPS += \
./App/ap/ap.d 


# Each subdirectory must supply rules for building sources it contributes
App/ap/%.o App/ap/%.su App/ap/%.cyclo: ../App/ap/%.c App/ap/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/01_led/App" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/01_led/App/ap" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/01_led/App/common" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/01_led/App/hw" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/01_led/App/hw/driver/include" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/01_led/App/hw/driver/src" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/01_led/App/hw/driver" -I"C:/Users/user/Desktop/projects/STM/stm32h5_kit/example/01_led/Core/Src" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-ap

clean-App-2f-ap:
	-$(RM) ./App/ap/ap.cyclo ./App/ap/ap.d ./App/ap/ap.o ./App/ap/ap.su

.PHONY: clean-App-2f-ap

