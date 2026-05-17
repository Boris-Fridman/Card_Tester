################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/boris/Documents/RealTimeColedge/ARM_Embedded_Systems/Card_Tester/Common_Libs/LinuxSpecific/Network.c 

OBJS += \
./Common_Libs/LinuxSpecific/Network.o 

C_DEPS += \
./Common_Libs/LinuxSpecific/Network.d 


# Each subdirectory must supply rules for building sources it contributes
Common_Libs/LinuxSpecific/Network.o: /home/boris/Documents/RealTimeColedge/ARM_Embedded_Systems/Card_Tester/Common_Libs/LinuxSpecific/Network.c Common_Libs/LinuxSpecific/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Core/Inc -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I../Drivers/CMSIS/Include -I../../Common_Libs -I../../Common_Libs/ARMSpecific -I../UserLibs -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Common_Libs-2f-LinuxSpecific

clean-Common_Libs-2f-LinuxSpecific:
	-$(RM) ./Common_Libs/LinuxSpecific/Network.cyclo ./Common_Libs/LinuxSpecific/Network.d ./Common_Libs/LinuxSpecific/Network.o ./Common_Libs/LinuxSpecific/Network.su

.PHONY: clean-Common_Libs-2f-LinuxSpecific

