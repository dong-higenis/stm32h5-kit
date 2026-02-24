################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/target/ethernetif.c 

OBJS += \
./lwip/target/ethernetif.o 

C_DEPS += \
./lwip/target/ethernetif.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/target/%.o lwip/target/%.su lwip/target/%.cyclo: ../lwip/target/%.c lwip/target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Src" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Src/driver" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Inc/driver" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Src/driver/fatfs" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/lwip" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/lwip/app" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/lwip/target" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/src" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/src/include" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/system" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/system/arch" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Drivers/BSP" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-target

clean-lwip-2f-target:
	-$(RM) ./lwip/target/ethernetif.cyclo ./lwip/target/ethernetif.d ./lwip/target/ethernetif.o ./lwip/target/ethernetif.su

.PHONY: clean-lwip-2f-target

