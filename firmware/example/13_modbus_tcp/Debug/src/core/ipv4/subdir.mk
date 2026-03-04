################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/core/ipv4/acd.c \
../src/core/ipv4/autoip.c \
../src/core/ipv4/dhcp.c \
../src/core/ipv4/etharp.c \
../src/core/ipv4/icmp.c \
../src/core/ipv4/igmp.c \
../src/core/ipv4/ip4.c \
../src/core/ipv4/ip4_addr.c \
../src/core/ipv4/ip4_frag.c 

OBJS += \
./src/core/ipv4/acd.o \
./src/core/ipv4/autoip.o \
./src/core/ipv4/dhcp.o \
./src/core/ipv4/etharp.o \
./src/core/ipv4/icmp.o \
./src/core/ipv4/igmp.o \
./src/core/ipv4/ip4.o \
./src/core/ipv4/ip4_addr.o \
./src/core/ipv4/ip4_frag.o 

C_DEPS += \
./src/core/ipv4/acd.d \
./src/core/ipv4/autoip.d \
./src/core/ipv4/dhcp.d \
./src/core/ipv4/etharp.d \
./src/core/ipv4/icmp.d \
./src/core/ipv4/igmp.d \
./src/core/ipv4/ip4.d \
./src/core/ipv4/ip4_addr.d \
./src/core/ipv4/ip4_frag.d 


# Each subdirectory must supply rules for building sources it contributes
src/core/ipv4/%.o src/core/ipv4/%.su src/core/ipv4/%.cyclo: ../src/core/ipv4/%.c src/core/ipv4/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Src" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Src/driver" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Inc/driver" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Src/driver/fatfs" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/lwip" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/lwip/app" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/lwip/target" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/src" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/src/include" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/system" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/system/arch" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Drivers/BSP" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-src-2f-core-2f-ipv4

clean-src-2f-core-2f-ipv4:
	-$(RM) ./src/core/ipv4/acd.cyclo ./src/core/ipv4/acd.d ./src/core/ipv4/acd.o ./src/core/ipv4/acd.su ./src/core/ipv4/autoip.cyclo ./src/core/ipv4/autoip.d ./src/core/ipv4/autoip.o ./src/core/ipv4/autoip.su ./src/core/ipv4/dhcp.cyclo ./src/core/ipv4/dhcp.d ./src/core/ipv4/dhcp.o ./src/core/ipv4/dhcp.su ./src/core/ipv4/etharp.cyclo ./src/core/ipv4/etharp.d ./src/core/ipv4/etharp.o ./src/core/ipv4/etharp.su ./src/core/ipv4/icmp.cyclo ./src/core/ipv4/icmp.d ./src/core/ipv4/icmp.o ./src/core/ipv4/icmp.su ./src/core/ipv4/igmp.cyclo ./src/core/ipv4/igmp.d ./src/core/ipv4/igmp.o ./src/core/ipv4/igmp.su ./src/core/ipv4/ip4.cyclo ./src/core/ipv4/ip4.d ./src/core/ipv4/ip4.o ./src/core/ipv4/ip4.su ./src/core/ipv4/ip4_addr.cyclo ./src/core/ipv4/ip4_addr.d ./src/core/ipv4/ip4_addr.o ./src/core/ipv4/ip4_addr.su ./src/core/ipv4/ip4_frag.cyclo ./src/core/ipv4/ip4_frag.d ./src/core/ipv4/ip4_frag.o ./src/core/ipv4/ip4_frag.su

.PHONY: clean-src-2f-core-2f-ipv4

