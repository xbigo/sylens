#pragma once

#include "vulkan.hpp"

namespace sylens
{
    extern const std::string K_Layer_validation; // = "VK_LAYER_KHRONOS_validation";
    extern const std::string K_Ext_debug_utils; // = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    std::vector<char const *> gatherLayers(std::vector<std::string> const &layers,
                                           std::vector<vk::LayerProperties> const &layerProperties);
    std::vector<char const *> gatherExtensions(std::vector<std::string> const &extensions,
                                               std::vector<vk::ExtensionProperties> const &extensionProperties );
}