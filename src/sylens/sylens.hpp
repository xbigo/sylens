#pragma once

#include "vulkan.hpp"
#include "utils.hpp"

#include <ape/estl/utility.hpp>

#include <vector>
#include <functional>
#include <optional>

namespace sylens
{
    struct screen_size
    {
        int width{800}, height{600};
    };
    struct window_hint
    {
        int hint, value;
    };


    class glfw 
    {
        public:
        glfw();
        ~glfw();
        
    };

    class window 
    {
        ape::own_ptr<GLFWwindow> handle;
        public:
        explicit window(ape::own_ptr<GLFWwindow>&& );
        ~window();

        GLFWwindow* raw_handle() const noexcept
        {
            return handle.get();
        }

        class builder{
            std::vector<window_hint> hint_;
            std::string title_;
            screen_size size_;
            GLFWmonitor* monitor_{nullptr};
            GLFWwindow* share_{nullptr};
        public:
            explicit builder(glfw&);
            builder& hint(window_hint h) { hint_.emplace_back(h); return *this;}
            builder& size(screen_size s) { size_ = s; return *this;}
            builder& title(std::string t) { title_ = std::move(t); return *this;}
            builder& monitor(GLFWmonitor* m) { monitor_ = m; return *this;}
            builder& monitor(GLFWwindow* s) { share_ = s; return *this;}

            window make();

        };
        std::function<void(GLFWwindow* window, int width, int height)> onResize;
    };

    using DebugMessenger_t = std::function<VkBool32(
        vk::DebugUtilsMessageSeverityFlagBitsEXT,
        vk::DebugUtilsMessageTypeFlagsEXT,
        const vk::DebugUtilsMessengerCallbackDataEXT *)>;

    class VulkanDevice // move only
    {
        vkr::Device device_;
        public:
        VulkanDevice();

        class builder
        {
            public:
            explicit builder(vkr::PhysicalDevice& physicalDevice);
            VulkanDevice make();

        };

    };

    class Surface
    {
        VkSurfaceKHR surface_{};

    };


    inline constexpr int max_frame_in_flight = 3;
    class VulkanApp
    {
        window* window_{nullptr};
        vkr::Context  context_;
        vkr::Instance instance_{nullptr};
        vkr::DebugUtilsMessengerEXT debug_messenger_{nullptr};
        DebugMessenger_t callback_;
        bool enable_debug_ {true};
        vkr::SurfaceKHR surface_{nullptr};
        vkr::Device device_{nullptr};
        vkr::Queue graphicsQueue_{nullptr};
        vkr::Queue presentQueue_{nullptr};
        
        vkr::SwapchainKHR swapChain_{nullptr};
        std::vector<vk::Image> images_;
        std::vector<vkr::ImageView> imageViews_;
        vk::Extent2D extent_{};
        //vk::Format swapChainImageFormat_{};
        vk::SurfaceFormatKHR surfaceFormat_{};

        vkr::ShaderModule vertShaderModule{nullptr}, fragShaderModule{nullptr};
        vkr::PipelineLayout pipelineLayout_{nullptr};
        vkr::RenderPass renderPass_{nullptr};
        std::vector<vkr::Pipeline> graphicsPipeline_;

        std::vector<vkr::Framebuffer> swapChainFramebuffers_;
        vkr::CommandPool commandPool_{nullptr};
        std::vector<vkr::CommandBuffer> commandBuffer_;


        std::vector<vkr::Semaphore> imageAvailableSemaphore_, renderFinishedSemaphore_;
        std::vector<vkr::Fence> inFlightFence_;

        std::size_t currentFrame_{0};
        bool framebufferResized_ = false;
    public:
        explicit VulkanApp(window* w = nullptr, 
            const std::string& appname = "vulkanApp",
            const std::string& engine_name = "vulkan",
            bool enable_debug = true,
            uint32_t api_version = VK_API_VERSION_1_3);
        ~VulkanApp();

        
        bool debugEnabled() const noexcept { return enable_debug_;}
        void setDebugMessenger(DebugMessenger_t f);
        DebugMessenger_t& getCallback() noexcept { return callback_;}
        const DebugMessenger_t& getCallback() const noexcept { return callback_;}

        vkr::PhysicalDevices getAllPhysicals() const;

        void dump(std::ostream& os);

    //private:
        void createDevice();
        void createSurface1();
        void createSurface2();

        void createSwapchain();
        void createRenderPass();
        void createPipeline();
        void createFramebuffers();
        void createCommandPool();
        void createCommandBuffer();
        void createSyncObjects();

        void recordCommandBuffer(vkr::CommandBuffer& buffer, uint32_t imageIndex);

        void drawFrame();

        void waitOnIdle();

        void recreateSwapChain();
    };

    constexpr uint32_t K_DefaultWidth = 800;
    constexpr uint32_t K_DefaultHeight = 600;

    bool isGraphicsDevice(const vkr::PhysicalDevice& d);

    class application
    {
        glfw& glfw_;
        public:
        explicit application(glfw& g);
        ~application();
        void loop(window& w, std::function<void()> draw);

        private:
        void init_();
        void cleanup_();

    };
}