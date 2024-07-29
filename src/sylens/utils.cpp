#include "./utils.hpp"
#include <gsl/gsl>

namespace sylens
{
    const std::string K_Layer_validation = "VK_LAYER_KHRONOS_validation";
    const std::string K_Ext_debug_utils = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    std::vector<char const *> gatherLayers(std::vector<std::string> const &layers,
                                           std::vector<vk::LayerProperties> const &layerProperties)
    {
        std::vector<char const *> enabledLayers;
        for (auto const &layer : layers)
        {
            APE_Expects(std::any_of(layerProperties.begin(), layerProperties.end(), [layer](vk::LayerProperties const &lp)
                               { return layer == lp.layerName; }));
            enabledLayers.push_back(layer.data());
        }

        return enabledLayers;
    }

    std::vector<char const *> gatherExtensions(std::vector<std::string> const &extensions,
                                               std::vector<vk::ExtensionProperties> const &extensionProperties)
    {
        std::vector<char const *> enabledExtensions;
        for (auto const &ext : extensions)
        {
            APE_Expects(std::any_of(
                extensionProperties.begin(), extensionProperties.end(), [ext](vk::ExtensionProperties const &ep)
                { return ext == ep.extensionName; }));
            enabledExtensions.push_back(ext.data());
        }

        uint32_t count = 0;
        auto requiredExts = glfwGetRequiredInstanceExtensions(&count);
        for(uint32_t i = 0; i < count; ++i)
            enabledExtensions.push_back(requiredExts[i]);

        return enabledExtensions;
    }
}
