/*
 * Program.cpp
 *
 *  Created on: Sep 10, 2018
 *      Author: John Hall
 */

#include "Program.h"
#include <iostream>

//**Must include glad and GLFW in this order or it breaks**
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "RayTracer.h"

RayTracer* currentRayTracer;

Program::Program() {
    setupWindow();
}

Program::~Program() {
  
}

void Program::start() {
	RayTracer* rayTracer = new RayTracer();
	currentRayTracer = rayTracer;

	rayTracer->generateObjects(1);
	rayTracer->createImage();

	//Main render loop
	while(!glfwWindowShouldClose(window)) {
		rayTracer->image.Render();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}



void Program::setupWindow() {
	//Initialize the GLFW windowing system
	if (!glfwInit()) {
		std::cout << "ERROR: GLFW failed to initialize, TERMINATING" << std::endl;
		return;
	}

	//Set the custom error callback function
	//Errors will be printed to the console
	glfwSetErrorCallback(ErrorCallback);

	//Attempt to create a window with an OpenGL 4.1 core profile context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	int width = 1024;
	int height = 1024;
	window = glfwCreateWindow(width, height, "CPSC 453 OpenGL Boilerplate", 0, 0);
	if (!window) {
		std::cout << "Program failed to create GLFW window, TERMINATING" << std::endl;
		glfwTerminate();
		return;
	}

	//Set the custom function that tracks key presses
	glfwSetKeyCallback(window, KeyCallback);

	//Bring the new window to the foreground (not strictly necessary but convenient)
	glfwMakeContextCurrent(window);

	//Intialize GLAD (finds appropriate OpenGL configuration for your system)
	if (!gladLoadGL()) {
		std::cout << "GLAD init failed" << std::endl;
		return;
	}

	//Query and print out information about our OpenGL environment
	QueryGLVersion();
}



void Program::QueryGLVersion() {
	// query opengl version and renderer information
	std::string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	std::string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	std::string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

	std::cout << "OpenGL [ " << version << " ] "
		<< "with GLSL [ " << glslver << " ] "
		<< "on renderer [ " << renderer << " ]" << std::endl;
}

void ErrorCallback(int error, const char* description) {
	std::cout << "GLFW ERROR " << error << ":" << std::endl;
	std::cout << description << std::endl;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	//allows this method to access required methods
	Program* controller = (Program*) glfwGetWindowUserPointer(window);// @suppress("Invalid arguments")

	//Key codes are often prefixed with GLFW_KEY_ and can be found on the GLFW website
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		currentRayTracer->generateObjects(1);
		currentRayTracer->createImage();
	}
	else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		currentRayTracer->generateObjects(2);
		currentRayTracer->createImage();
	}
	else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		currentRayTracer->generateObjects(3);
		currentRayTracer->createImage();
	}
	else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		currentRayTracer->changeFOV(10);
		currentRayTracer->createImage();
	}
	else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		currentRayTracer->changeFOV(-10);
		currentRayTracer->createImage();
	}
}
