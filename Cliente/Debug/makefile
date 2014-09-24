################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Cliente

dependents:
	-cd /home/utnso/tp-2013-2c-allin/commons/Debug && $(MAKE) all
	-cd /home/utnso/workspace/commons2/Debug && $(MAKE) all

# Tool invocations
Cliente: $(OBJS) $(USER_OBJS) /home/utnso/tp-2013-2c-allin/commons/Debug/libcommons.so /home/utnso/workspace/commons2/Debug/libcommons2.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/workspace/commons2/Debug" -L"/home/utnso/tp-2013-2c-allin/commons/Debug" -o "Cliente" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Cliente
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/tp-2013-2c-allin/commons/Debug/libcommons.so:
/home/utnso/workspace/commons2/Debug/libcommons2.so:

-include ../makefile.targets
