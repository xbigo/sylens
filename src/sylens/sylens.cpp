#include "sylens.hpp"
#include <vulkan/vulkan_enums.hpp>

#include <vector>
#include <iostream>
#include <cstring>
#include <print>
#include <algorithm>
#include <ranges>
#include <sstream>
#include <filesystem>
#include <fstream>

#define BOOST_PFR_CORE_NAME_ENABLED 1
#include <boost/pfr.hpp>
#include <boost/pfr/core_name.hpp>

const char* const shader_fragment = R"(
#version 450
layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;
void main() {
    outColor = vec4(fragColor, 1.0);
}
)";

const char* const shader_vertex = R"(
#version 450

layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
)";

namespace sylens
{
    namespace details {

        std::ostream& operator<<(std::ostream& os, const VkExtent2D& v2d)
        {
            std::print(os, "{{ width: {},\n height: {}\n}}", v2d.width, v2d.height);
            return os;
        }
        std::ostream& operator<<(std::ostream& os, const VkExtent3D& v3d)
        {
            std::print(os, "{{ width: {},\n height: {},\n depth: {}\n }}", v3d.width, v3d.height, v3d.depth);
            return os;
        }

        template <class, class = void> struct has_native_type : std::false_type {};
        template <class T>
        struct has_native_type<T, std::void_t<typename T::NativeType>> : std::true_type {};
        template <typename T> using has_native_type_v = has_native_type<T>::value;

        template <typename T>
        std::string do_to_string(const T &t);


        template<typename T>
        std::string to_string(const T& v)
        {
            std::stringstream sstr;
            const char* dim = "";
            constexpr auto names_array = boost::pfr::names_as_array<T>();
            sstr << "{";
            boost::pfr::for_each_field(v, [&](const auto& field, std::size_t i) { 
                using field_t = std::remove_cvref_t<decltype(field)>;
                if constexpr (std::is_array_v<field_t>)
                {
                    if constexpr(std::is_same_v<char, std::remove_all_extents_t<field_t> >)
                    {
                        sstr << std::exchange(dim, ",\n     ") << names_array[i] << ": " << std::string(field);
                    }
                    else
                    {
                        sstr << std::exchange(dim, ",\n     ") << names_array[i] << ": [";
                        const char *dim2 = "";
                        for (auto &item : field)
                            sstr << std::exchange(dim2, ", ") << item;
                        sstr << "]";
                    }
                }
                else if constexpr (has_native_type<field_t>::value)
                {
                    sstr << std::exchange(dim, ",\n     ") << names_array[i] << ": " << do_to_string(field);
                }
                else if constexpr (!has_native_type<field_t>::value) {
                    sstr << std::exchange(dim, ",\n     ") << names_array[i] << ": " << field;
                }
            });
            sstr << "}";
            return sstr.str();
        }

        template <typename T>
        std::string do_to_string(const T &t)
        {
            using native_t = typename T::NativeType;
            const native_t &v = t;
            return details::to_string(v);
        }
    }

    template<typename T>
    std::string to_string(const T& t)
    {
        using native_t = typename T::NativeType;
        const native_t& v = t;
        return details::to_string(v);
    }

    std::string to_string(const vk::LayerProperties& t)
    {
        std::stringstream os;
        std::print(os, "{{ layerName: {},\n specVersion: {},\n implementationVersion: {},\n description: {}\n }}",
             t.layerName, t.specVersion, t.implementationVersion, t.description);
        return os.str();
    }
    std::string to_string(const vk::ExtensionProperties& t)
    {
        std::stringstream os;
        std::print(os, "{{ extensionName: {},\n specVersion: {}\n }}",
             t.extensionName, t.specVersion);
        return os.str();
    }

    std::string to_string2(const VkPhysicalDeviceProperties& p)
    {
        return std::format("{{apiVersion: {},\n driverVersion: {},\n venderID: {},\n deviceID: {},\n deviceType: {},\n deviceName: {},\n piplineCacheUUID: (none),\n "
            "limits: {},\n sparseProperties: {}\n}}",
            p.apiVersion, p.driverVersion, p.vendorID, p.deviceID, int(p.deviceType), std::string(p.deviceName), 
            //details::to_string(p.limits),
            "none",
            details::to_string(p.sparseProperties)
        ) ;      
    }
    //auto print_array = [](const char *name, const auto &arr)
    auto print_array(const char *name, const auto &arr)
    {
        std::println(" {}:", name);
        int idx = 0;
        for (auto &item : arr)
        {
            std::println("  {:2} {}", idx++, to_string(item));
        }
    };

    glfw::glfw()
    {
        glfwInit();
    }
    glfw::~glfw()
    {
        glfwTerminate();
    }

    window::window(ape::own_ptr<GLFWwindow>&& pw)
        : handle(std::move(pw))
    {
    }
    window::~window(){
        glfwDestroyWindow(handle.get());
        handle.release();
    }

    window::builder::builder(glfw&){}

    window window::builder::make()
    {
        for(auto&v : hint_)
            glfwWindowHint(v.hint, v.value);
        return window{ape::own_ptr<GLFWwindow>{glfwCreateWindow(size_.width, size_.height, title_.c_str(), monitor_, share_)}};
    }

#if 0
    VulkanDevice VulkanDevice::builder::make()
    {
        //VkDeviceQueueCreateInfo info{};
        vk::DeviceQueueCreateInfo info{};
        vk::DeviceQueueInfo2 info2{};
        return {};
    }
#endif

    template<typename T>
    VKAPI_ATTR VkBool32 VKAPI_CALL callMessenger(
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
        const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
    {
        auto p =  static_cast<T*>(pUserData);
        auto& callback = p->getCallback();
        if (callback)
            return callback(messageSeverity, messageTypes, pCallbackData);

        // if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        std::println(std::cerr, "VALIDATION {:<11} {:<10} - {}", 
            vk::to_string(vk::DebugUtilsMessageSeverityFlagsEXT(messageSeverity)),
            vk::to_string(vk::DebugUtilsMessageTypeFlagsEXT(messageTypes)),
            pCallbackData->pMessage
            );
        return VK_FALSE;
    }

    VulkanApp::~VulkanApp(){
    }
    VulkanApp::VulkanApp(const std::string& appname, const std::string& engine_name, bool enable_debug, uint32_t api_version)
    : enable_debug_(enable_debug)
    {
        vk::ApplicationInfo applicationInfo( appname.c_str(), 1, engine_name.c_str(), 1, api_version );
        std::vector<std::string> layers, extensions;
        if (enable_debug)
        {
            layers.emplace_back(K_Layer_validation);
            extensions.emplace_back(K_Ext_debug_utils);
        }

        auto arr_layers =  gatherLayers( layers, context_.enumerateInstanceLayerProperties() );
        auto arr_extensions = gatherExtensions(extensions, context_.enumerateInstanceExtensionProperties());
        instance_ = context_.createInstance({{}, &applicationInfo, arr_layers, arr_extensions});

        if (enable_debug)
            setDebugMessenger({});
    }

    void VulkanApp::setDebugMessenger(DebugMessenger_t f)
    {
        APE_Expects(debugEnabled());
        if (debug_messenger_.getInstance() == nullptr)
        {
            debug_messenger_ = instance_.createDebugUtilsMessengerEXT({ {},
               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError 
               | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
               vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                 vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding,
               &callMessenger<VulkanApp>, this});
        }
        callback_ = f;
    }
    vkr::PhysicalDevices VulkanApp::getAllPhysicals() const 
    {
        return vkr::PhysicalDevices(instance_);
    }

    void VulkanApp::dump(std::ostream &os)
    {

        std::println("==========================Vulkan Dump2=========================");
        print_array("Layers", context_.enumerateInstanceLayerProperties());
        print_array("Extension", context_.enumerateInstanceExtensionProperties());

        for(auto& d : getAllPhysicals())
        {
            auto features = d.getFeatures();
            auto properties = d.getProperties();
            std::println(os, "Features: {}\n Properties: {}", to_string(features), to_string2(properties));

            for(auto& f : d.getQueueFamilyProperties2())
            {
                std::println(os, "Family {} ", to_string(f.queueFamilyProperties));
                //if ((f.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
            }
        }

    }

    template<typename Func>
    static std::vector<std::tuple<vkr::PhysicalDevice, uint32_t, float>> qualifiedDevice(vkr::Instance& instance, Func pred)
    {
        std::vector<std::tuple<vkr::PhysicalDevice, uint32_t, float>> res;
        for (auto &d : vkr::PhysicalDevices(instance))
        {
            uint32_t idx = 0;
            for (auto &f : d.getQueueFamilyProperties2())
            {
                if (pred(d, f.queueFamilyProperties, idx))
                {
                    res.emplace_back(d, idx, 1.0f);
                }
                ++idx;
            }
        }

        return res;
    }

    static std::vector<std::tuple<vkr::PhysicalDevice, uint32_t, uint32_t, float>> findFirstPhysical(vkr::Instance& instance, vkr::SurfaceKHR& surface)
    {
        std::vector<std::tuple<vkr::PhysicalDevice, uint32_t, uint32_t, float>> res;
        for (auto &d : vkr::PhysicalDevices(instance))
        {
            std::optional<uint32_t> graphicsIdx, representIdx;

            uint32_t idx = 0;
            for (auto &f : d.getQueueFamilyProperties2())
            {
                if (!graphicsIdx.has_value() && f.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics)
                {
                    graphicsIdx = idx;
                }

                if (!representIdx.has_value() && d.getSurfaceSupportKHR(idx, surface))
                {
                    representIdx = idx;
                }


                ++idx;
            }

            if (graphicsIdx.has_value() && representIdx.has_value())
            {
                res.emplace_back(d, *graphicsIdx, *representIdx, 1.0f);
                break;
            }
        }

        return res;
    }

    //std::vector<vk::DeviceQueueCreateInfo> getQueueInfo(const std::tuple<vkr::PhysicalDevice, uint32_t, uint32_t, float> & info)

    template<std::ranges::range R>
    std::vector<vk::DeviceQueueCreateInfo> getQueueInfo(const R& indices)
    {
        std::vector<vk::DeviceQueueCreateInfo> res;
        for(auto idx : indices)
            res.emplace_back(
                vk::DeviceQueueCreateFlagBits{}, //::eProtected,
                idx,
                1,
                nullptr);

        return res;
    }

    void setFeatures(vk::PhysicalDeviceFeatures &features)
    {
    }

#if 0
    void init_xcb()
    {
        xcb_window_t xcb_window = 0;
        xcb_screen_t *screen = nullptr;
        xcb_connection_t *connection = nullptr;
        xcb_intern_atom_reply_t *atom_wm_delete_window = nullptr;

        const xcb_setup_t *setup;
        xcb_screen_iterator_t iter;
        int scr;

        const char *display_envar = getenv("DISPLAY");
        if (display_envar == nullptr || display_envar[0] == '\0')
        {
            printf("Environment variable DISPLAY requires a valid value.\nExiting ...\n");
            fflush(stdout);
            exit(1);
        }

        connection = xcb_connect(nullptr, &scr);
        if (xcb_connection_has_error(connection) > 0)
        {
            printf("Cannot connect to XCB.\nExiting ...\n");
            fflush(stdout);
            exit(1);
        }

        setup = xcb_get_setup(connection);
        iter = xcb_setup_roots_iterator(setup);
        while (scr-- > 0)
            xcb_screen_next(&iter);

        screen = iter.data;
    }

#endif

    void VulkanApp::createSurface1(const window& w)
    {
        //vk::SurfaceKHR surface;
        VkSurfaceKHR surface;
        auto result = glfwCreateWindowSurface(
            *instance_,
            w.raw_handle(),
            nullptr,
            &surface
        );
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create surface.");
        }

        surface_ = {instance_, surface};
    }
    void VulkanApp::createSurface2()
    {
        //vk::XcbSurfaceCreateInfoKHR surface{ };
    }

    void VulkanApp::createDevice()
    {
        std::vector<std::string> layers, extensions;

        auto arrQualifiedPhysical = qualifiedDevice(instance_, [](auto&& ,auto&& prop, uint32_t) {
            return prop.queueFlags & vk::QueueFlagBits::eGraphics;
        });
        auto arrSurfacePhysical = qualifiedDevice(instance_, [this](auto&& physicalDevice, auto&& prop, uint32_t familyIdx) {
            bool supportPresent = physicalDevice.getSurfaceSupportKHR(familyIdx, surface_);
            return supportPresent;
        });

        if (arrQualifiedPhysical.empty() || arrSurfacePhysical.empty())
        {
            throw std::runtime_error("Not found qualified physical device.");
        }

        // can't use resize(1) here, becasue vkr::PhysicalDevice has no default ctor.
        arrQualifiedPhysical.erase(arrQualifiedPhysical.begin() + 1, arrQualifiedPhysical.end());
        arrSurfacePhysical.erase(arrSurfacePhysical.begin() + 1, arrSurfacePhysical.end());
        arrQualifiedPhysical.emplace_back(arrSurfacePhysical.front());

        ///////////

        auto arrQualifiedPhysical2 = findFirstPhysical(instance_, surface_);
        if (arrQualifiedPhysical2.empty())
        {
            throw std::runtime_error("Not found qualified physical device.");
        }
        auto &[physicalDevice, graphicsIdx, presentIdx, ignore_2] = arrQualifiedPhysical2.front();
        std::vector<uint32_t> familyIndices {graphicsIdx, presentIdx};
        // VUID-VkDeviceCreateInfo-queueFamilyIndex-02802:
        // The queueFamilyIndex member of each element of pQueueCreateInfos must be unique
        if (familyIndices.front() == familyIndices.back())
            familyIndices.resize(1);


        auto arr_queueInfo = getQueueInfo(familyIndices);
        const float queuePriority = 1.0f;
        for (auto& info : arr_queueInfo)
        {
            info.pQueuePriorities = &queuePriority;
        }
        // std::vector<char const *> arr_layers ;
        std::vector<char const *> arr_extensions{vk::KHRSwapchainExtensionName};

        vk::PhysicalDeviceFeatures deviceFeatures{};
        setFeatures(deviceFeatures);

        vk::DeviceCreateInfo createInfo{
            vk::DeviceCreateFlagBits{},
            arr_queueInfo,
            {}, // arr_layers,
            arr_extensions,
            //&deviceFeatures
        };

        device_ = physicalDevice.createDevice(createInfo);
        if (!*device_)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        graphicsQueue_ = device_.getQueue(graphicsIdx, 0);
        presentQueue_ = device_.getQueue(presentIdx, 0);

        createSwapchain();

    }
    void VulkanApp::createSwapchain()
    {
        auto arrQualifiedPhysical2 = findFirstPhysical(instance_, surface_);
        if (arrQualifiedPhysical2.empty())
        {
            throw std::runtime_error("Not found qualified physical device.");
        }
        auto &[device, graphicsIdx, presentIdx, ignore_2] = arrQualifiedPhysical2.front();

        auto capabilities = device.getSurfaceCapabilitiesKHR(surface_);
        std::println(std::cout, "Surface capabilities {} ", to_string(capabilities));

        auto formats = device.getSurfaceFormatsKHR(surface_);
        print_array("surface formats", formats);

        auto presentModes = device.getSurfacePresentModesKHR(surface_);  
        print_array("Present mode {} ", presentModes);

        extent_ = capabilities.currentExtent;
        if (extent_.width == std::numeric_limits<uint32_t>::max()) {
            extent_.width =  std::clamp(K_DefaultWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent_.height =  std::clamp(K_DefaultHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        surfaceFormat_ = formats.front();


        std::array<vk::PresentModeKHR, 2> orderedModes = {
            vk::PresentModeKHR::eMailbox,
            vk::PresentModeKHR::eImmediate
            //vk::PresentModeKHR::eFifo,
            //vk::PresentModeKHR::eFifoRelaxed
            };
        auto mode = vk::PresentModeKHR::eFifo;
        for (auto m : orderedModes)
        {
            if (std::ranges::find(presentModes, m) != presentModes.end())  {
                mode = m;
                break;
            }
        }

        uint32_t maxImageCount = capabilities.maxImageCount == 0 ? 3 : capabilities.maxImageCount;
        uint32_t imageCount = std::clamp(uint32_t(3), capabilities.minImageCount, maxImageCount);

        bool sameQueueFamily = graphicsIdx == presentIdx;
        std::array<uint32_t, 2> familyIndices {graphicsIdx, presentIdx};
        vk::SwapchainCreateInfoKHR createInfo{
            {}, // flags: SwapchainCreateFlagsKHR
            surface_,
            imageCount,    // minImageCount
            surfaceFormat_.format, // imageFormat
            surfaceFormat_.colorSpace,
            extent_, // imageExtent
            1,      // imageArrayLayers
            vk::ImageUsageFlagBits::eColorAttachment, // imageUsage: can be eTransferDst if need post process
            sameQueueFamily? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent, // imageSharingMode
            sameQueueFamily? uint32_t{0} : uint32_t{2}, // queueFamilyIndexCount
            sameQueueFamily? nullptr : familyIndices.data(), // pQueueFamilyIndices
            capabilities.currentTransform, // preTransform: 
            vk::CompositeAlphaFlagBitsKHR::eOpaque,    //compositeAlpha:
            mode, //presentMode:
            VK_TRUE, //clipped:
            swapChain_ // old swap chain:
        };
        swapChain_ =  device_.createSwapchainKHR(createInfo);

        images_ = swapChain_.getImages();
        imageViews_.clear();
        imageViews_.reserve(images_.size());
        for(auto& image : images_)
        {
            vk::ImageViewCreateInfo info{
                {}, //flags
                image,
                vk::ImageViewType::e2D,
                surfaceFormat_.format,
                {}, //components: VK_COMPONENT_SWIZZLE_IDENTITY
                {
                    vk::ImageAspectFlagBits::eColor,
                    0, //baseMipLevel
                    1, //levelCount
                    0, //baseArrayLayer
                    1  //layerCount

                }  //ImageSubresourceRange 
            };
            imageViews_.emplace_back(device_.createImageView(info));
        }
    }

    std::vector<char> readFile(const std::string& p)
    {
        auto size = std::filesystem::file_size(p);
        std::vector<char> buffer;
        buffer.resize(size);

        std::ifstream ifs(p.c_str(), std::ios::binary);
        ifs.read(buffer.data(), size);

        return buffer;
    }
    vkr::ShaderModule creatShaderModule(vkr::Device& device_, const std::string& p){
        auto code = readFile(p);
        vk::ShaderModuleCreateInfo createInfo(vk::ShaderModuleCreateFlags{}, 
            code.size(), reinterpret_cast<const uint32_t*>(code.data()));
        
        return device_.createShaderModule(createInfo);
    }
    void VulkanApp::createRenderPass()
    {
        vk::AttachmentDescription colorAttachment({},
            surfaceFormat_.format,
            vk::SampleCountFlagBits::e1,        //samples_
            vk::AttachmentLoadOp::eClear,       //loadOp_
            vk::AttachmentStoreOp::eStore,      //storeOp_
            vk::AttachmentLoadOp::eDontCare,    //stencilLoadOp_
            vk::AttachmentStoreOp::eDontCare,   //stencilStoreOp_
            vk::ImageLayout::eUndefined,        //initialLayout_
            vk::ImageLayout::ePresentSrcKHR     //finalLayout_
        );

        vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
        vk::SubpassDescription subpass({},
            vk::PipelineBindPoint::eGraphics,   //pipelineBindPoint_
            0,                                  //inputAttachmentCount_
            nullptr,                            //pInputAttachments_
            1,                                  //colorAttachmentCount
            &colorAttachmentRef                 //pColorAttachments_
        );

        vk::SubpassDependency dependency(vk::SubpassExternal, 
            0,          //dstSubpass
            vk::PipelineStageFlagBits::eColorAttachmentOutput, //srcStageMask_
            vk::PipelineStageFlagBits::eColorAttachmentOutput, //dstStageMask_
            vk::AccessFlagBits::eNone,                      //srcAccessMask
            vk::AccessFlagBits::eColorAttachmentWrite   //dstAccessMask
        );

        vk::RenderPassCreateInfo renderPassInfo({},
            1,                                  //attachmentCount_
            &colorAttachment,                   //pAttachments_
            1,                                  //subpassCount_
            &subpass,                           //pSubpasses_
            1,
            &dependency                         //pDependencies_
        );
        renderPass_ = device_.createRenderPass(renderPassInfo, nullptr);

       
    }

    void VulkanApp::createPipeline()
    {
        vertShaderModule = creatShaderModule(device_, "D:/code/xbigo/sylens/src/sylens/vert.spv");
        fragShaderModule =  creatShaderModule(device_, "D:/code/xbigo/sylens/src/sylens/frag.spv");

        vk::PipelineShaderStageCreateInfo vertInfo({}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main");
        vk::PipelineShaderStageCreateInfo fragInfo({}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main");

        vk::PipelineShaderStageCreateInfo shaderStages[] = {
            vertInfo, fragInfo
        };

        vk::DynamicState state[] = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };
        vk::PipelineDynamicStateCreateInfo dynamicState({}, 2, state);

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList);

        vk::Viewport viewport(0.f, 0.f, float(extent_.width), float(extent_.height), 0.f, 1.f);
        vk::Rect2D scissor({0, 0}, extent_);

        vk::PipelineViewportStateCreateInfo viewportState({}, 1, &viewport, 1, &scissor);

        vk::PipelineRasterizationStateCreateInfo rasterizer({}, 
            VK_FALSE,   //depthClampEnable_
            VK_FALSE,   //rasterizerDiscardEnable_
            vk::PolygonMode::eFill, 
            vk::CullModeFlagBits::eBack,
            vk::FrontFace::eClockwise,
            VK_FALSE,   //depthBiasEnable_
            0.0f,       //depthBiasConstantFactor_
            0.0f,       //depthBiasClamp_
            0.0f,       //depthBiasSlopeFactor_
            1.0f        //lineWidth_
        );
        vk::PipelineMultisampleStateCreateInfo multisampling({}, vk::SampleCountFlagBits::e1, 
            VK_FALSE,   //sampleShadingEnable_
            1.0f,       //minSampleShading_
            nullptr,    //pSampleMask_
            VK_FALSE,   //alphaToCoverageEnable_
            VK_FALSE    //alphaToOneEnable_
        );
        vk::PipelineColorBlendAttachmentState colorBlendAttachment(
            VK_FALSE,       //blendEnable
            vk::BlendFactor::eOne,  //srcColorBlendFactor_
            vk::BlendFactor::eZero, //dstColorBlendFactor_
            vk::BlendOp::eAdd,      //colorBlendOp_
            vk::BlendFactor::eOne,  //srcAlphaBlendFactor_
            vk::BlendFactor::eZero, //dstAlphaBlendFactor_
            vk::BlendOp::eAdd,      //alphaBlendOp_
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |vk::ColorComponentFlagBits::eA  //colorWriteMask_
        );
        vk::PipelineColorBlendStateCreateInfo colorBlending({},
            VK_FALSE,           //logicOpEnable_
            vk::LogicOp::eCopy, //logicOp_
            1,                  //attachmentCount_
            &colorBlendAttachment
            //{{0.f, 0.f, 0.f, 0.f}} //blendConstants_
        );

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayout_ = device_.createPipelineLayout(pipelineLayoutInfo, nullptr);

        vk::GraphicsPipelineCreateInfo pipelineInfo({},
            2,                                  //stageCount_
            shaderStages,                       //pStages_
            &vertexInputInfo,                   //pVertexInputState_
            &inputAssembly,
            nullptr,                            //pTessellationState_
            &viewportState,
            &rasterizer, 
            &multisampling,                     //pMultisampleState_
            nullptr,                            //pDepthStencilState_
            &colorBlending,
            &dynamicState,
            pipelineLayout_,
            renderPass_,
            0                                   //subpass_
        );

        graphicsPipeline_ = device_.createGraphicsPipelines(nullptr, pipelineInfo);

    }

    void VulkanApp::createFramebuffers()
    {
        //swapChainFramebuffers_.resize(imageViews_.size());

        //for(std::size_t i = 0; i < imageViews_.size(); ++i)
        for(auto& attachments : imageViews_)
        {
            //vk::ImageView attachments[] = { imageViews_[i] };
            vk::FramebufferCreateInfo framebufferInfo({},
                renderPass_,
                1,              //attachmentCount_
                &*attachments,
                extent_.width,
                extent_.height,
                1               //layers_
            );
            swapChainFramebuffers_.emplace_back(device_.createFramebuffer(framebufferInfo));
        }
    }
    void VulkanApp::createCommandPool()
    {
        auto arrQualifiedPhysical2 = findFirstPhysical(instance_, surface_);
        if (arrQualifiedPhysical2.empty())
        {
            throw std::runtime_error("Not found qualified physical device.");
        }
        auto &[physicalDevice, graphicsIdx, presentIdx, ignore_2] = arrQualifiedPhysical2.front();

        vk::CommandPoolCreateInfo poolInfo(
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,    //flags
            graphicsIdx         // queueFamilyIndex_
        );
        commandPool_ = device_.createCommandPool(poolInfo);

    }
    void VulkanApp::createCommandBuffer()
    {
        vk::CommandBufferAllocateInfo allocInfo(
            commandPool_,
            vk::CommandBufferLevel::ePrimary,       //level
            max_frame_in_flight               //commandBufferCount_
        );
        commandBuffer_ = device_.allocateCommandBuffers(allocInfo);
    }
    
    void VulkanApp::recordCommandBuffer(vkr::CommandBuffer& buffer, uint32_t imageIndex)
    {
        vk::CommandBufferBeginInfo beginInfo;
        buffer.begin(beginInfo);

        vk::ClearValue clearColor(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f));
        // clearColor.setColor({0.0f, 0.0f, 0.0f, 1.0f});

        vk::RenderPassBeginInfo renderPassInfo(
            renderPass_,
            swapChainFramebuffers_[imageIndex],    //framebuffer_
            vk::Rect2D({0, 0}, extent_), //renderArea_
            1,
            &clearColor 
        );
        buffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline_.front());

        vk::Viewport viewport(0.f, 0.f, float(extent_.width), float(extent_.height), 0.f, 1.f);
        vk::Rect2D scissor({0, 0}, extent_);
        buffer.setViewport(0, viewport);
        buffer.setScissor(0, scissor);
        buffer.draw(3, 1, 0, 0);
        buffer.endRenderPass();
        buffer.end();

    }

    void VulkanApp::createSyncObjects()
    {
        vk::SemaphoreCreateInfo semaphoreInfo;
        vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);

        for(int i = 0; i < max_frame_in_flight; ++i)
        {
            imageAvailableSemaphore_.emplace_back(device_.createSemaphore(semaphoreInfo));
            renderFinishedSemaphore_.emplace_back(device_.createSemaphore(semaphoreInfo));
            inFlightFence_.emplace_back(device_.createFence(fenceInfo));
        }
    }

    void VulkanApp::drawFrame()
    {
        auto result1 = device_.waitForFences(*inFlightFence_[currentFrame_], vk::True, UINT64_MAX);
        if (vk::Result::eSuccess != result1)
        {
            throw std::runtime_error("device_.waitForFences");
        }
        device_.resetFences(*inFlightFence_[currentFrame_]);

        auto [result, imageIndex] = swapChain_.acquireNextImage(UINT64_MAX, imageAvailableSemaphore_[currentFrame_],  VK_NULL_HANDLE);

        auto& buffer = commandBuffer_[currentFrame_];
        buffer.reset();
        recordCommandBuffer(buffer, imageIndex);

        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
            
        vk::SubmitInfo submitInfo(
            1,
            &*imageAvailableSemaphore_[currentFrame_],
            waitStages,
            1,
            &*buffer,
            1,
            &*renderFinishedSemaphore_[currentFrame_]            
        );

        graphicsQueue_.submit(submitInfo, inFlightFence_[currentFrame_]);

        vk::PresentInfoKHR presentInfo(
            1,
            &*renderFinishedSemaphore_[currentFrame_],
            1,
            &*swapChain_,
            &imageIndex
        );
        auto result2 = presentQueue_.presentKHR(presentInfo);

        currentFrame_ = (currentFrame_ + 1) % max_frame_in_flight;
    }
    void VulkanApp::waitOnIdle()
    {
        //vkDeviceWaitIdle(device_);
        device_.waitIdle();
    }

    bool isGraphicsDevice(const vkr::PhysicalDevice &d)
    {
        return true;
    }

    application::application(glfw &g)
        : glfw_(g)
    {
    }
    application::~application()
    {
    }
    void application::loop(window &w, std::function<void()> draw)
    {
        while (!glfwWindowShouldClose(w.raw_handle()))
        {
            glfwPollEvents();
            draw();
        }
    }
}