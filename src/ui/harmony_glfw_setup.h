#pragma once

struct GLFWwindow;

/** @brief GLFW window hints for Harmony OpenGL 3.x + ImGui. */
void harmonyConfigureGlfwOpenGLHints();

/**
 * @brief Initialize the OpenGL function loader after @c glfwMakeContextCurrent.
 * @return false on failure (reserved; ImGui uses its built-in loader).
 */
bool harmonyInitOpenGLLoader();
