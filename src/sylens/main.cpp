
#include "sylens.hpp"

#include <iostream>
#include <print>

int main(){
    sylens::glfw glfw;

    auto window = sylens::window::builder(glfw)
        .hint(sylens::window_hint{GLFW_CLIENT_API, GLFW_NO_API})
        .hint(sylens::window_hint{GLFW_RESIZABLE, GLFW_TRUE})
        .title("Vulkan window")
        .size(sylens::screen_size{800, 600})
        .make();

    sylens::application app{glfw};

    sylens::VulkanApp vulkan(&window);
    {
        
        vulkan.dump(std::cout);
        vulkan.createSurface1();
        vulkan.createDevice();

        vulkan.createSwapchain();
        vulkan.createRenderPass();
        vulkan.createPipeline();
        vulkan.createFramebuffers();
        vulkan.createCommandPool();
        vulkan.createCommandBuffer();
        vulkan.createSyncObjects();

    }
#if 0
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::cout << extensionCount << " extensions supported\n";

    glm::mat4 matrix{};
    glm::vec4 vec{};
    auto test = matrix * vec;
#endif
    app.loop(window, [&](){vulkan.drawFrame();});
    vulkan.waitOnIdle();
    
    return 0;
}
