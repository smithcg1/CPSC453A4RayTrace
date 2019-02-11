################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../middleware/glm-0.9.8.2/glm/detail/glm.cpp 

OBJS += \
./middleware/glm-0.9.8.2/glm/detail/glm.o 

CPP_DEPS += \
./middleware/glm-0.9.8.2/glm/detail/glm.d 


# Each subdirectory must supply rules for building sources it contributes
middleware/glm-0.9.8.2/glm/detail/%.o: ../middleware/glm-0.9.8.2/glm/detail/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/uga/smithcg/Desktop/CPSC455A4/middleware/glfw/include" -I"/home/uga/smithcg/Desktop/CPSC455A4/middleware/glad/include" -I"/home/uga/smithcg/Desktop/CPSC455A4/middleware/glm-0.9.8.2" -I"/home/uga/smithcg/Desktop/CPSC455A4/middleware/stb" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


