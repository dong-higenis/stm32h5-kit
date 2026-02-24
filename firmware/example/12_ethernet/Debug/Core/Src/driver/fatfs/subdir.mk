################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/driver/fatfs/diskio.c \
../Core/Src/driver/fatfs/ff.c \
../Core/Src/driver/fatfs/ffsystem.c \
../Core/Src/driver/fatfs/ffunicode.c 

OBJS += \
./Core/Src/driver/fatfs/diskio.o \
./Core/Src/driver/fatfs/ff.o \
./Core/Src/driver/fatfs/ffsystem.o \
./Core/Src/driver/fatfs/ffunicode.o 

C_DEPS += \
./Core/Src/driver/fatfs/diskio.d \
./Core/Src/driver/fatfs/ff.d \
./Core/Src/driver/fatfs/ffsystem.d \
./Core/Src/driver/fatfs/ffunicode.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/driver/fatfs/%.o Core/Src/driver/fatfs/%.su Core/Src/driver/fatfs/%.cyclo: ../Core/Src/driver/fatfs/%.c Core/Src/driver/fatfs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Src" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Src/driver" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Inc/driver" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Core/Src/driver/fatfs" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/lwip" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/lwip/app" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/lwip/target" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/src" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/src/include" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/system" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/system/arch" -I"C:/work/git/STM32-H5-KIT/firmware/example/12_ethernet/Drivers/BSP" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-driver-2f-fatfs

clean-Core-2f-Src-2f-driver-2f-fatfs:
	-$(RM) ./Core/Src/driver/fatfs/diskio.cyclo ./Core/Src/driver/fatfs/diskio.d ./Core/Src/driver/fatfs/diskio.o ./Core/Src/driver/fatfs/diskio.su ./Core/Src/driver/fatfs/ff.cyclo ./Core/Src/driver/fatfs/ff.d ./Core/Src/driver/fatfs/ff.o ./Core/Src/driver/fatfs/ff.su ./Core/Src/driver/fatfs/ffsystem.cyclo ./Core/Src/driver/fatfs/ffsystem.d ./Core/Src/driver/fatfs/ffsystem.o ./Core/Src/driver/fatfs/ffsystem.su ./Core/Src/driver/fatfs/ffunicode.cyclo ./Core/Src/driver/fatfs/ffunicode.d ./Core/Src/driver/fatfs/ffunicode.o ./Core/Src/driver/fatfs/ffunicode.su

.PHONY: clean-Core-2f-Src-2f-driver-2f-fatfs

