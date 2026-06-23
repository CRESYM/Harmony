/**
 * @file harmony_glfw_setup.cpp
 * @brief Shared GLFW / OpenGL setup for HarmonyUI and visualization.
 */
#include "harmony_glfw_setup.h"

#include <iostream>

#include <GLFW/glfw3.h>

#if defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>
#endif


void harmonyConfigureGlfwOpenGLHints() {
#ifdef __APPLE__
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
}


bool harmonyInitOpenGLLoader() {
#if defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	glewExperimental = GL_TRUE;
	const GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << "Harmony: GLEW init failed: "
			<< reinterpret_cast<const char*>(glewGetErrorString(err)) << '\n';
		return false;
	}
#endif
	return true;
}
