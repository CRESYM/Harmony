#ifndef VIZ_INCLUDES_H
#define VIZ_INCLUDES_H

/**
 * @file VizIncludes.h
 * @brief GLFW / ImGui / ImPlot (include only from visualization translation units).
 */

#ifdef __APPLE__
    #define GLFW_INCLUDE_GLCOREARB
#endif

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#endif // VIZ_INCLUDES_H
