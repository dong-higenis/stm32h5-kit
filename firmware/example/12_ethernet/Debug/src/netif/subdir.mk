################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/netif/bridgeif.c \
../src/netif/bridgeif_fdb.c \
../src/netif/ethernet.c \
../src/netif/lowpan6.c \
../src/netif/lowpan6_ble.c \
../src/netif/lowpan6_common.c \
../src/netif/slipif.c \
../src/netif/zepif.c 

OBJS += \
./src/netif/bridgeif.o \
./src/netif/bridgeif_fdb.o \
./src/netif/ethernet.o \
./src/netif/lowpan6.o \
./src/netif/lowpan6_ble.o \
./src/netif/lowpan6_common.o \
./src/netif/slipif.o \
./src/netif/zepif.o 

C_DEPS += \
./src/netif/bridgeif.d \
./src/netif/bridgeif_fdb.d \
./src/netif/ethernet.d \
./src/netif/lowpan6.d \
./src/netif/lowpan6_ble.d \
./src/netif/lowpan6_common.d \
./src/netif/slipif.d \
./src/netif/zepif.d 


# Each subdirectory must supply rules for building sources it contributes
src/netif/%.o src/netif/%.su src/netif/%.cyclo: ../src/netif/%.c src/netif/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Src" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Src/driver" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Inc/driver" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Src/driver/fatfs" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/lwip" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/lwip/app" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/lwip/target" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/src" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/src/include" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/system" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/system/arch" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Drivers/BSP" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-src-2f-netif

clean-src-2f-netif:
	-$(RM) ./src/netif/bridgeif.cyclo ./src/netif/bridgeif.d ./src/netif/bridgeif.o ./src/netif/bridgeif.su ./src/netif/bridgeif_fdb.cyclo ./src/netif/bridgeif_fdb.d ./src/netif/bridgeif_fdb.o ./src/netif/bridgeif_fdb.su ./src/netif/ethernet.cyclo ./src/netif/ethernet.d ./src/netif/ethernet.o ./src/netif/ethernet.su ./src/netif/lowpan6.cyclo ./src/netif/lowpan6.d ./src/netif/lowpan6.o ./src/netif/lowpan6.su ./src/netif/lowpan6_ble.cyclo ./src/netif/lowpan6_ble.d ./src/netif/lowpan6_ble.o ./src/netif/lowpan6_ble.su ./src/netif/lowpan6_common.cyclo ./src/netif/lowpan6_common.d ./src/netif/lowpan6_common.o ./src/netif/lowpan6_common.su ./src/netif/slipif.cyclo ./src/netif/slipif.d ./src/netif/slipif.o ./src/netif/slipif.su ./src/netif/zepif.cyclo ./src/netif/zepif.d ./src/netif/zepif.o ./src/netif/zepif.su

.PHONY: clean-src-2f-netif

