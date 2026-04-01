################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/netif/ppp/auth.c \
../src/netif/ppp/ccp.c \
../src/netif/ppp/chap-md5.c \
../src/netif/ppp/chap-new.c \
../src/netif/ppp/chap_ms.c \
../src/netif/ppp/demand.c \
../src/netif/ppp/eap.c \
../src/netif/ppp/ecp.c \
../src/netif/ppp/eui64.c \
../src/netif/ppp/fsm.c \
../src/netif/ppp/ipcp.c \
../src/netif/ppp/ipv6cp.c \
../src/netif/ppp/lcp.c \
../src/netif/ppp/magic.c \
../src/netif/ppp/mppe.c \
../src/netif/ppp/multilink.c \
../src/netif/ppp/ppp.c \
../src/netif/ppp/pppapi.c \
../src/netif/ppp/pppcrypt.c \
../src/netif/ppp/pppoe.c \
../src/netif/ppp/pppol2tp.c \
../src/netif/ppp/pppos.c \
../src/netif/ppp/upap.c \
../src/netif/ppp/utils.c \
../src/netif/ppp/vj.c 

OBJS += \
./src/netif/ppp/auth.o \
./src/netif/ppp/ccp.o \
./src/netif/ppp/chap-md5.o \
./src/netif/ppp/chap-new.o \
./src/netif/ppp/chap_ms.o \
./src/netif/ppp/demand.o \
./src/netif/ppp/eap.o \
./src/netif/ppp/ecp.o \
./src/netif/ppp/eui64.o \
./src/netif/ppp/fsm.o \
./src/netif/ppp/ipcp.o \
./src/netif/ppp/ipv6cp.o \
./src/netif/ppp/lcp.o \
./src/netif/ppp/magic.o \
./src/netif/ppp/mppe.o \
./src/netif/ppp/multilink.o \
./src/netif/ppp/ppp.o \
./src/netif/ppp/pppapi.o \
./src/netif/ppp/pppcrypt.o \
./src/netif/ppp/pppoe.o \
./src/netif/ppp/pppol2tp.o \
./src/netif/ppp/pppos.o \
./src/netif/ppp/upap.o \
./src/netif/ppp/utils.o \
./src/netif/ppp/vj.o 

C_DEPS += \
./src/netif/ppp/auth.d \
./src/netif/ppp/ccp.d \
./src/netif/ppp/chap-md5.d \
./src/netif/ppp/chap-new.d \
./src/netif/ppp/chap_ms.d \
./src/netif/ppp/demand.d \
./src/netif/ppp/eap.d \
./src/netif/ppp/ecp.d \
./src/netif/ppp/eui64.d \
./src/netif/ppp/fsm.d \
./src/netif/ppp/ipcp.d \
./src/netif/ppp/ipv6cp.d \
./src/netif/ppp/lcp.d \
./src/netif/ppp/magic.d \
./src/netif/ppp/mppe.d \
./src/netif/ppp/multilink.d \
./src/netif/ppp/ppp.d \
./src/netif/ppp/pppapi.d \
./src/netif/ppp/pppcrypt.d \
./src/netif/ppp/pppoe.d \
./src/netif/ppp/pppol2tp.d \
./src/netif/ppp/pppos.d \
./src/netif/ppp/upap.d \
./src/netif/ppp/utils.d \
./src/netif/ppp/vj.d 


# Each subdirectory must supply rules for building sources it contributes
src/netif/ppp/%.o src/netif/ppp/%.su src/netif/ppp/%.cyclo: ../src/netif/ppp/%.c src/netif/ppp/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/ap" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App" -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/hw" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/hw/include" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/core" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw/driver/lcd" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/common/hw/include/lcd" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw/driver/lcd/driver" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/App/hw/driver/lcd/font" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/lwip" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/Drivers/BSP" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/lwip/app" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/lwip/target" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/src" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/src/include" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/system" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/system/arch" -I"C:/work/git/STM32-H5-KIT/firmware/H5_KIT_Test/src/core" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-src-2f-netif-2f-ppp

clean-src-2f-netif-2f-ppp:
	-$(RM) ./src/netif/ppp/auth.cyclo ./src/netif/ppp/auth.d ./src/netif/ppp/auth.o ./src/netif/ppp/auth.su ./src/netif/ppp/ccp.cyclo ./src/netif/ppp/ccp.d ./src/netif/ppp/ccp.o ./src/netif/ppp/ccp.su ./src/netif/ppp/chap-md5.cyclo ./src/netif/ppp/chap-md5.d ./src/netif/ppp/chap-md5.o ./src/netif/ppp/chap-md5.su ./src/netif/ppp/chap-new.cyclo ./src/netif/ppp/chap-new.d ./src/netif/ppp/chap-new.o ./src/netif/ppp/chap-new.su ./src/netif/ppp/chap_ms.cyclo ./src/netif/ppp/chap_ms.d ./src/netif/ppp/chap_ms.o ./src/netif/ppp/chap_ms.su ./src/netif/ppp/demand.cyclo ./src/netif/ppp/demand.d ./src/netif/ppp/demand.o ./src/netif/ppp/demand.su ./src/netif/ppp/eap.cyclo ./src/netif/ppp/eap.d ./src/netif/ppp/eap.o ./src/netif/ppp/eap.su ./src/netif/ppp/ecp.cyclo ./src/netif/ppp/ecp.d ./src/netif/ppp/ecp.o ./src/netif/ppp/ecp.su ./src/netif/ppp/eui64.cyclo ./src/netif/ppp/eui64.d ./src/netif/ppp/eui64.o ./src/netif/ppp/eui64.su ./src/netif/ppp/fsm.cyclo ./src/netif/ppp/fsm.d ./src/netif/ppp/fsm.o ./src/netif/ppp/fsm.su ./src/netif/ppp/ipcp.cyclo ./src/netif/ppp/ipcp.d ./src/netif/ppp/ipcp.o ./src/netif/ppp/ipcp.su ./src/netif/ppp/ipv6cp.cyclo ./src/netif/ppp/ipv6cp.d ./src/netif/ppp/ipv6cp.o ./src/netif/ppp/ipv6cp.su ./src/netif/ppp/lcp.cyclo ./src/netif/ppp/lcp.d ./src/netif/ppp/lcp.o ./src/netif/ppp/lcp.su ./src/netif/ppp/magic.cyclo ./src/netif/ppp/magic.d ./src/netif/ppp/magic.o ./src/netif/ppp/magic.su ./src/netif/ppp/mppe.cyclo ./src/netif/ppp/mppe.d ./src/netif/ppp/mppe.o ./src/netif/ppp/mppe.su ./src/netif/ppp/multilink.cyclo ./src/netif/ppp/multilink.d ./src/netif/ppp/multilink.o ./src/netif/ppp/multilink.su ./src/netif/ppp/ppp.cyclo ./src/netif/ppp/ppp.d ./src/netif/ppp/ppp.o ./src/netif/ppp/ppp.su ./src/netif/ppp/pppapi.cyclo ./src/netif/ppp/pppapi.d ./src/netif/ppp/pppapi.o ./src/netif/ppp/pppapi.su ./src/netif/ppp/pppcrypt.cyclo ./src/netif/ppp/pppcrypt.d ./src/netif/ppp/pppcrypt.o ./src/netif/ppp/pppcrypt.su ./src/netif/ppp/pppoe.cyclo ./src/netif/ppp/pppoe.d ./src/netif/ppp/pppoe.o ./src/netif/ppp/pppoe.su ./src/netif/ppp/pppol2tp.cyclo ./src/netif/ppp/pppol2tp.d ./src/netif/ppp/pppol2tp.o ./src/netif/ppp/pppol2tp.su ./src/netif/ppp/pppos.cyclo ./src/netif/ppp/pppos.d ./src/netif/ppp/pppos.o ./src/netif/ppp/pppos.su ./src/netif/ppp/upap.cyclo ./src/netif/ppp/upap.d ./src/netif/ppp/upap.o ./src/netif/ppp/upap.su ./src/netif/ppp/utils.cyclo ./src/netif/ppp/utils.d ./src/netif/ppp/utils.o ./src/netif/ppp/utils.su ./src/netif/ppp/vj.cyclo ./src/netif/ppp/vj.d ./src/netif/ppp/vj.o ./src/netif/ppp/vj.su

.PHONY: clean-src-2f-netif-2f-ppp

