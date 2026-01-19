################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/ap/toy/ui/ui.c \
../App/ap/toy/ui/ui_draw.c \
../App/ap/toy/ui/ui_game.c \
../App/ap/toy/ui/ui_time.c 

OBJS += \
./App/ap/toy/ui/ui.o \
./App/ap/toy/ui/ui_draw.o \
./App/ap/toy/ui/ui_game.o \
./App/ap/toy/ui/ui_time.o 

C_DEPS += \
./App/ap/toy/ui/ui.d \
./App/ap/toy/ui/ui_draw.d \
./App/ap/toy/ui/ui_game.d \
./App/ap/toy/ui/ui_time.d 


# Each subdirectory must supply rules for building sources it contributes
App/ap/toy/ui/%.o App/ap/toy/ui/%.su App/ap/toy/ui/%.cyclo: ../App/ap/toy/ui/%.c App/ap/toy/ui/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/hw" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/ap" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/common" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App" -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/common/hw" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/common/hw/include" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/common/core" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/hw/driver/lcd" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/common/hw/include/lcd" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/hw/driver/lcd/driver" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/hw/driver/lcd/font" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/lwip" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/Drivers/BSP" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/lwip/app" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/lwip/target" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/src" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/src/include" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/system" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/system/arch" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/src/core" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/ap/toy" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/hw/driver/fatfs" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/ap/toy/peripheral" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/ap/toy/ui" -I"C:/Users/user/Desktop/projects/touchgfx/H5_KIT_TOY/App/ap/toy/peripheral/can" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-ap-2f-toy-2f-ui

clean-App-2f-ap-2f-toy-2f-ui:
	-$(RM) ./App/ap/toy/ui/ui.cyclo ./App/ap/toy/ui/ui.d ./App/ap/toy/ui/ui.o ./App/ap/toy/ui/ui.su ./App/ap/toy/ui/ui_draw.cyclo ./App/ap/toy/ui/ui_draw.d ./App/ap/toy/ui/ui_draw.o ./App/ap/toy/ui/ui_draw.su ./App/ap/toy/ui/ui_game.cyclo ./App/ap/toy/ui/ui_game.d ./App/ap/toy/ui/ui_game.o ./App/ap/toy/ui/ui_game.su ./App/ap/toy/ui/ui_time.cyclo ./App/ap/toy/ui/ui_time.d ./App/ap/toy/ui/ui_time.o ./App/ap/toy/ui/ui_time.su

.PHONY: clean-App-2f-ap-2f-toy-2f-ui

