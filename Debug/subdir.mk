################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../glad.c 

CPP_SRCS += \
../Light.cpp \
../Object.cpp \
../Plane.cpp \
../Program.cpp \
../Ray.cpp \
../RayTracer.cpp \
../Sphere.cpp \
../Triangle.cpp \
../imagebuffer.cpp \
../main.cpp 

OBJS += \
./Light.o \
./Object.o \
./Plane.o \
./Program.o \
./Ray.o \
./RayTracer.o \
./Sphere.o \
./Triangle.o \
./glad.o \
./imagebuffer.o \
./main.o 

C_DEPS += \
./glad.d 

CPP_DEPS += \
./Light.d \
./Object.d \
./Plane.d \
./Program.d \
./Ray.d \
./RayTracer.d \
./Sphere.d \
./Triangle.d \
./imagebuffer.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/uga/smithcg/Desktop/CPSC455A4/middleware/glfw/include" -I"/home/uga/smithcg/Desktop/CPSC455A4/middleware/glad/include" -I"/home/uga/smithcg/Desktop/CPSC455A4/middleware/glm-0.9.8.2" -I"/home/uga/smithcg/Desktop/CPSC455A4/middleware/stb" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/uga/smithcg/Desktop/CPSC455A4/middleware/glad/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


