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
all: libnivel-gui.so

dependents:
	-cd /home/utnso/tp-2013-2c-allin/commons/Debug && $(MAKE) clean all

# Tool invocations
libnivel-gui.so: $(OBJS) $(USER_OBJS) /home/utnso/tp-2013-2c-allin/commons/Debug/libcommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/tp-2013-2c-allin/commons/Debug" -shared -o "libnivel-gui.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(LIBRARIES) libnivel-gui.so
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/tp-2013-2c-allin/commons/Debug/libcommons.so:

-include ../makefile.targets
