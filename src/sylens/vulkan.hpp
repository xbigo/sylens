#pragma once

#define VK_USE_PLATFORM_XCB_KHR
//#define VK_USE_PLATFORM_XLIB_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_X11

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan_raii.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <ape/config.hpp>

namespace sylens {
    namespace vkr = vk::raii;
    
}
