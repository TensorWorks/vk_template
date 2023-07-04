/* External Libraries */
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

/*#include <vulkan/vk_enum_string_helper.h>
 * FIXME Need to use validation layers for this, should only be in debug builds anyway */
char*
string_VkResult(VkResult r)
{
    return 0;
}

/* System Libraries */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Platform Layer */
#include "x11/main.h"
#include "x11/main.c"

/* Core Layer */
#include "main.h"

int
main(int argc, char* argv[])
{
    /* TODO Make configurable */
    const int width = 1280;
    const int height = 720;
    uint8_t* vertShaderSpv;
    const int vertShaderSpvSize = 1372;
    uint8_t* fragShaderSpv;
    const int fragShaderSpvSize = 500;

    /*
     * 41-step vulkan initialization based on https://github.com/jbendtsen/stuff/blob/master/triangle.c
     * 1) Create GLFW Window
     */

    glfwInit();
    /* Stopa: Haven't checked the list of all the hints, maybe we want different ones? */
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(width, height, "vk_template", 0, 0);
    if (!window) {
        const char* errstr;
        int code = glfwGetError(&errstr);
        fprintf(stderr, "Couldn't create a GLFW window, error code [%i]:\n%s\n", code, errstr);
        return 1;
    }

    /*
     * 2) Check for Vulkan by requesting extension list
     */

    unsigned int instanceExtensionCount = 0;
    const char** instanceExtensionRequest = glfwGetRequiredInstanceExtensions(&instanceExtensionCount);
    if (!instanceExtensionRequest) {
        const char* errstr;
        int code = glfwGetError(&errstr);
        fprintf(stderr, "Could not find any Vulkan extensions, error code [%i]:\n%s\n", code, errstr);
        return 2;
    }

    /*
     * 3) Vulkan lives! Create an instance
     */
    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo = {0};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = instanceExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensionRequest;

    VkInstance instance;
    VkResult vk_result = vkCreateInstance(&instanceCreateInfo, 0, &instance);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateInstance() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 3;
    }

    /*
     * 4) Get list of GPUs and pick one
     */

    int gpuCount = 0;
    vk_result = vkEnumeratePhysicalDevices(instance, &gpuCount, 0);
    if (gpuCount <= 0) {
        fprintf(stderr, "No graphics hardware found, physical device count = %i\n"
                "\tFailed with result code [%i]: %s\n", gpuCount, vk_result, string_VkResult(vk_result));
        return 4;
    }

    gpuCount = 1;
    VkPhysicalDevice gpu = {0};
    vk_result = vkEnumeratePhysicalDevices(instance, &gpuCount, &gpu);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumeratePhysicalDevices() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 4;
    }

    /*
     * 5) Pick a queue family
     */
    int queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueCount, 0);
    if (queueCount <= 0) {
        fprintf(stderr, "No Queue families found\n");
        return 5;
    }

    VkQueueFamilyProperties *qfp = malloc(queueCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueCount, qfp);

    int queueIndex = -1;
    for (int i = 0; i < queueCount; i++) {
        if (qfp[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueIndex = i;
            break;
        }
    }
    free(qfp);

    if (queueIndex < 0) {
        fprintf(stderr, "Could not find a queue with graphics support\n");
        return 5;
    }

    /*
     * 6) Check that the chosen queue family supports presentation
     */
    if (!glfwGetPhysicalDevicePresentationSupport(instance, gpu, queueIndex)) {
        fprintf(stderr, "The selected queue family does not support present mode\n");
        return 6;
    }

    /*
     * 7) Get all the Vulkan _device_ extensions (as opposed to the instance exceptions)
     */
    unsigned int deviceExtensionCount = 0;
    vk_result = vkEnumerateDeviceExtensionProperties(gpu, 0, &deviceExtensionCount, 0);
    if (deviceExtensionCount <= 0 || vk_result != VK_SUCCESS) {
        fprintf(stderr, "Could not find any Vulkan device extensions [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 7;
    }

    VkExtensionProperties* deviceExtensionProperties = calloc(deviceExtensionCount, sizeof(VkExtensionProperties));
    vk_result = vkEnumerateDeviceExtensionProperties(gpu, 0, &deviceExtensionCount, deviceExtensionProperties);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumerateDeviceExtensionProperties() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 7;
    }

    const char** deviceExtensions = calloc(deviceExtensionCount, sizeof(void*));
    for (int i = 0; i < deviceExtensionCount; i++) {
        deviceExtensions[i] = &deviceExtensionProperties[i].extensionName[0];
    }

    /*
     * 8) Create a virtual device for Vulkan. This is our primary interface between the program and the GPU
     */
    float priority = 0.0f;
    VkDeviceQueueCreateInfo queueInfo = {0};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = queueIndex;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;

    VkDeviceCreateInfo deviceCreateInfo = {0};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueInfo;
    deviceCreateInfo.enabledExtensionCount = deviceExtensionCount;
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

    VkDevice device = {0};
    vk_result = vkCreateDevice(gpu, &deviceCreateInfo, 0, &device);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateDevice() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 8;
    }

    /*
     * 9) Get implementation-specific function pointers.
     * This allows using specialized parts of the Vulkan API that aren't included by default
     */

    {
        const int fptrsRequested = 7;
        int fptrsLoaded = 0;

        vkGetPhysicalDeviceSurfaceCapabilities = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
            vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
        vkGetPhysicalDeviceSurfaceFormats = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)
            vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
        vkCreateSwapchain = (PFN_vkCreateSwapchainKHR) vkGetInstanceProcAddr(instance, "vkCreateSwapchainKHR");
        vkDestroySwapchain = (PFN_vkDestroySwapchainKHR) vkGetInstanceProcAddr(instance, "vkDestroySwapchainKHR");
        vkGetSwapchainImages = (PFN_vkGetSwapchainImagesKHR) vkGetInstanceProcAddr(instance, "vkGetSwapchainImagesKHR");
        vkAcquireNextImage = (PFN_vkAcquireNextImageKHR) vkGetInstanceProcAddr(instance, "vkAcquireNextImageKHR");
        vkQueuePresent = (PFN_vkQueuePresentKHR) vkGetInstanceProcAddr(instance, "vkQueuePresentKHR");

        fptrsLoaded += vkGetPhysicalDeviceSurfaceCapabilities != 0;
        fptrsLoaded += vkGetPhysicalDeviceSurfaceFormats != 0;
        fptrsLoaded += vkCreateSwapchain != 0;
        fptrsLoaded += vkDestroySwapchain != 0;
        fptrsLoaded += vkGetSwapchainImages != 0;
        fptrsLoaded += vkAcquireNextImage != 0;
        fptrsLoaded += vkQueuePresent != 0;

        if (fptrsLoaded != fptrsRequested) {
            fprintf(stderr, "Error loading KHR extension function pointers (found %i/%i)\n",
                    fptrsLoaded, fptrsRequested);
            return 9;
        }
    }

    /*
     * 10) Creating a window surface to attach the virtual device to
     */

    VkSurfaceKHR surface;
    vk_result = glfwCreateWindowSurface(instance, window, 0, &surface);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "glfwCreateWindowSurface() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 10;
    }

    /*
     * 11) Determine the color format
     */

    VkSurfaceFormatKHR colorFormat = {0};
    {
        int colorFormatCount = 0;
        vk_result = vkGetPhysicalDeviceSurfaceFormats(gpu, surface, &colorFormatCount, 0);
        if (colorFormatCount <= 0 || vk_result != VK_SUCCESS) {
            fprintf(stderr, "Could not find any color formats for the window surface, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 11;
        }

        VkSurfaceFormatKHR* colors = malloc(colorFormatCount * sizeof(VkSurfaceFormatKHR));
        vk_result = vkGetPhysicalDeviceSurfaceFormats(gpu, surface, &colorFormatCount, colors);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkGetPhysicalDeviceSurfaceFormats() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 11;
        }

        for (int i = 0; i < colorFormatCount; i++) {
            if (colors[i].format == VK_FORMAT_B8G8R8A8_UNORM) {
                colorFormat = colors[i];
                break;
            }
        }
        free(colors);

        if (colorFormat.format == VK_FORMAT_UNDEFINED) {
            fprintf(stderr, "The window surface does not define a B8G8R8A8 color format\n");
            return 11;
        }
    }

    /*
     * 12) Get information about OS-specific surface capabilities
     */

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vk_result = vkGetPhysicalDeviceSurfaceCapabilities(gpu, surface, &surfaceCapabilities);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkGetPhysicalDeviceSurfaceCapabilities() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 12;
    }

    if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF) {
        surfaceCapabilities.currentExtent.width = width;
        surfaceCapabilities.currentExtent.height = height;
    }

    /* TODO This is where you use vkGetPhysicalDeviceSurfacePresentModeKHR() to get a non-vsync presentation mode */

    /*
     * 13) Select the composite alpha format
     */

    VkCompositeAlphaFlagBitsKHR alphaFormat = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    {
        VkCompositeAlphaFlagBitsKHR alphaList[] = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
        };

        for (int i = 0; i < sizeof(alphaList) / sizeof(VkCompositeAlphaFlagBitsKHR); i++) {
            if (surfaceCapabilities.supportedCompositeAlpha & alphaList[i]) {
                alphaFormat = alphaList[i];
                break;
            }
        }
    }

    /*
     * 14) Create a swapchain.
     */

    VkSwapchainKHR swapchain;
    {
        int swapImageCount = surfaceCapabilities.minImageCount + 1;
        if (surfaceCapabilities.maxImageCount > 0 && swapImageCount > surfaceCapabilities.maxImageCount)
            swapImageCount = surfaceCapabilities.maxImageCount;

        VkImageUsageFlags imageUsageFlags =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            (surfaceCapabilities.supportedUsageFlags & (VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                                        VK_IMAGE_USAGE_TRANSFER_DST_BIT));

        VkSwapchainCreateInfoKHR swapchainCreateInfo = {0};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.pNext = 0;
        swapchainCreateInfo.surface = surface;
        swapchainCreateInfo.minImageCount = swapImageCount;
        swapchainCreateInfo.imageFormat = colorFormat.format;
        swapchainCreateInfo.imageColorSpace = colorFormat.colorSpace;
        swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
        swapchainCreateInfo.imageUsage = imageUsageFlags;
        swapchainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR) surfaceCapabilities.currentTransform;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = 0;
        swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        swapchainCreateInfo.oldSwapchain = 0;
        swapchainCreateInfo.clipped = VK_TRUE;
        swapchainCreateInfo.compositeAlpha = alphaFormat;

        vk_result = vkCreateSwapchain(device, &swapchainCreateInfo, 0, &swapchain);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateSwapchain() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 14;
        }
    }

    /*
     * 15) Get swapchain images for our framebuffers to write to
     */

    int imageCount = 0;
    vk_result = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, 0);
    if (imageCount <= 0 || vk_result != VK_SUCCESS) {
            fprintf(stderr, "Could not find any swapchain images\n");
            return 15;
    }

    VkImage* images = calloc(imageCount, sizeof(VkImage));
    vk_result = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkGetSwapchainImages() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 15;
    }

    /*
     * 16) Create image views for the swapchain
     */

    VkImageView* imageViews;
    {
        VkImageViewCreateInfo imageViewCreateInfo = {0};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = 0;
        imageViewCreateInfo.format = colorFormat.format;
        imageViewCreateInfo.components = (VkComponentMapping) {
            .r = VK_COMPONENT_SWIZZLE_R, /* Didn't realize explicit field initialization was in C99 already */
            .g = VK_COMPONENT_SWIZZLE_G,
            .b = VK_COMPONENT_SWIZZLE_B,
            .a = VK_COMPONENT_SWIZZLE_A,
        };
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.flags = 0;

        imageViews = calloc(imageCount, sizeof(VkImageView));
        for (int i = 0; i < imageCount; i++) {
            imageViewCreateInfo.image = images[i];
            vk_result = vkCreateImageView(device, &imageViewCreateInfo, 0, &imageViews[i]);
            if (vk_result != VK_SUCCESS) {
                fprintf(stderr, "vkCreateImageView() num %i failed, result code [%i]: %s\n",
                        i, vk_result, string_VkResult(vk_result));
                return 16;
            }
        }
    }

    /*
     * 17) Create a command pool
     */

    VkCommandPool commandPool;
    {
        VkCommandPoolCreateInfo cmdCreateInfo = {0};
        cmdCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdCreateInfo.queueFamilyIndex = queueIndex;
        cmdCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        vk_result = vkCreateCommandPool(device, &cmdCreateInfo, 0, &commandPool);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateCommandPool() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 17;
        }
    }

    /*
     * 18) Allocate command buffers
     */

    VkCommandBuffer* commandBuffers;
    {
        VkCommandBufferAllocateInfo cbufAllocInfo = {0};
        cbufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbufAllocInfo.commandPool = commandPool;
        cbufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbufAllocInfo.commandBufferCount = imageCount;

        commandBuffers = calloc(imageCount, sizeof(VkCommandBuffer));
        vk_result = vkAllocateCommandBuffers(device, &cbufAllocInfo, commandBuffers);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkAllocateCommandBuffers() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 18;
        }
    }

    /*
     * 19) Depth format
     */

    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    {
        VkFormat formats[] = {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM,
        };

        for (int i = 0; i < sizeof(formats) / sizeof(VkFormat); i++) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(gpu, formats[i], &props);
            if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                depthFormat = formats[i];
                break;
            }
        }

        if (depthFormat == VK_FORMAT_UNDEFINED) {
            fprintf(stderr, "Could not find a suitable depth format\n");
            return 19;
        }
    }

    /*
     * 20) Create depth stencil image
     */

    VkImage depthImage;
    {
        VkImageCreateInfo dsCreateInfo = {0};
        dsCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        dsCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        dsCreateInfo.format = depthFormat;
        dsCreateInfo.extent = (VkExtent3D) {
            .width = surfaceCapabilities.currentExtent.width,
            .height = surfaceCapabilities.currentExtent.height,
            .depth = 1
        };
        dsCreateInfo.mipLevels = 1;
        dsCreateInfo.arrayLayers = 1;
        dsCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        dsCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        dsCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        vk_result = vkCreateImage(device, &dsCreateInfo, 0, &depthImage);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateImage() failed for depth-stencil image, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 20;
        }
    }

    /*
     * 21) Allocate memory for the depth stencil
     */

    VkDeviceMemory depthMemory;
    VkPhysicalDeviceMemoryProperties gpuMemoryProperties;
    {
        vkGetPhysicalDeviceMemoryProperties(gpu, &gpuMemoryProperties);

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(device, depthImage, &memoryRequirements);

        int memoryTypeIndex = -1;
        for (int i = 0; i < gpuMemoryProperties.memoryTypeCount; i++) {
            if ((memoryRequirements.memoryTypeBits & (1 << i))
                && (gpuMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
            {
                memoryTypeIndex = i;
                break;
            }
        }

        if (memoryTypeIndex < 0) {
            fprintf(stderr, "Could not find suitable graphics memory\n");
            return 21;
        }

        VkMemoryAllocateInfo memoryAllocInfo = {0};
        memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocInfo.allocationSize = memoryRequirements.size;
        memoryAllocInfo.memoryTypeIndex = memoryTypeIndex;

        vk_result = vkAllocateMemory(device, &memoryAllocInfo, 0, &depthMemory);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkAllocateMemory() failed for depth-stencil image, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 21;
        }
    }

    /*
     * 22) Bind the newly allocated memroy to the depth stencil image
     */

    vk_result = vkBindImageMemory(device, depthImage, depthMemory, 0);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkBindImageMemory() failed for depth-stencil image, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 22;
    }

    /*
     * 23) Create depth stencil image view to be passed to each framebuffer
     */

    VkImageView depthView;
    {
        VkImageAspectFlagBits aspect = depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT ?
            VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
            VK_IMAGE_ASPECT_DEPTH_BIT;

        VkImageViewCreateInfo viewCreateInfo = {0};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.image = depthImage;
        viewCreateInfo.format = depthFormat;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;
        viewCreateInfo.subresourceRange.aspectMask = aspect;

        vk_result = vkCreateImageView(device, &viewCreateInfo, 0, &depthView);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateImageView() failed for depth-stencil image, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 23;
        }
    }

    /*
     * 24) Set up the render pass
     */

    VkRenderPass renderpass;
    {
        VkAttachmentDescription attachments[] = {
            { /* Color Attachment */
                .flags          = 0,
                .format         = colorFormat.format,
                .samples        = VK_SAMPLE_COUNT_1_BIT,
                .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            },
            { /* Depth Attachment */
                .flags          = 0,
                .format         = depthFormat,
                .samples        = VK_SAMPLE_COUNT_1_BIT,
                .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            }
        };

        VkAttachmentReference colorReference = {0};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthReference = {0};
        depthReference.attachment = 1;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {0};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorReference;
        subpass.pDepthStencilAttachment = &depthReference;

        VkSubpassDependency dependencies[] = {
            {
                .srcSubpass         = VK_SUBPASS_EXTERNAL,
                .dstSubpass         = 0,
                .srcStageMask       = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                .dstStageMask       = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask      = VK_ACCESS_MEMORY_READ_BIT,
                .dstAccessMask      = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dependencyFlags    = VK_DEPENDENCY_BY_REGION_BIT
            },
            {
                .srcSubpass         = 0,
                .dstSubpass         = VK_SUBPASS_EXTERNAL,
                .srcStageMask       = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask       = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                .srcAccessMask      = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dstAccessMask      = VK_ACCESS_MEMORY_READ_BIT,
                .dependencyFlags    = VK_DEPENDENCY_BY_REGION_BIT
            }
        };

        VkRenderPassCreateInfo renderpassCreateInfo = {0};
        renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpassCreateInfo.attachmentCount = 2;
        renderpassCreateInfo.pAttachments = attachments;
        renderpassCreateInfo.subpassCount = 1;
        renderpassCreateInfo.pSubpasses = &subpass;
        renderpassCreateInfo.dependencyCount = 2;
        renderpassCreateInfo.pDependencies = dependencies;

        vk_result = vkCreateRenderPass(device, &renderpassCreateInfo, 0, &renderpass);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateRenderPass() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 24;
        }
    }

    /*
     * 25) Create the frame buffers
     */

    VkFramebuffer* framebuffers;
    VkImageView framebufferViews[2];
    framebufferViews[1] = depthView;
    {
        VkFramebufferCreateInfo fbCreateInfo = {0};
        fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbCreateInfo.renderPass = renderpass;
        fbCreateInfo.attachmentCount = 2;
        fbCreateInfo.pAttachments = framebufferViews;
        fbCreateInfo.width = surfaceCapabilities.currentExtent.width;
        fbCreateInfo.height = surfaceCapabilities.currentExtent.height;
        fbCreateInfo.layers = 1;

        framebuffers = calloc(imageCount, sizeof(VkFramebuffer));
        for (int i = 0; i < imageCount; i++) {
            framebufferViews[0] = imageViews[i];
            vk_result = vkCreateFramebuffer(device, &fbCreateInfo, 0, &framebuffers[i]);
            if (vk_result != VK_SUCCESS) {
                fprintf(stderr, "vkCreateFramebuffer() failed, result code [%i]: %s\n",
                        vk_result, string_VkResult(vk_result));
                return 25;
            }
        }
    }

    /*
     * 26) Create semaphores for synchronizing draw commands and image presentation
     */

    VkSemaphore semaPresent, semaRender;
    {
        VkSemaphoreCreateInfo sCreateInfo = {0};
        sCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        vk_result = vkCreateSemaphore(device, &sCreateInfo, 0, &semaPresent);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateSemaphore() failed for presentation semaphore, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 26;
        }

        vk_result = vkCreateSemaphore(device, &sCreateInfo, 0, &semaRender);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateSemaphore() failed for rendering semaphore, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 26;
        }
    }

    /*
     * 27) Create wait fences, one for each image
     */

    VkFence* fences;
    {
        VkFenceCreateInfo info = {0};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        fences = calloc(imageCount, sizeof(VkFence));
        for (int i = 0; i < imageCount; i++) {
            vk_result = vkCreateFence(device, &info, 0, &fences[i]);
            if (vk_result != VK_SUCCESS) {
                fprintf(stderr, "vkCreateFence() failed for fence no. %i, result code [%i]: %s\n",
                        i, vk_result, string_VkResult(vk_result));
                return 27;
            }
        }
    }

    /*
     * 28) DEPLOY THE TRIANGLE
     */

        float vertexes[] = {
            /* Position XYZ      Color RGB */
            1.f,  1.f,  0.f,  1.f,  0.f,  0.f,
            -1.f,  1.f,  0.f,  0.f,  1.f,  0.f,
            0.f, -1.f,  0.f,  0.f,  0.f,  1.f,
        };
        int indexes[] = {
            0, 1, 2,
        };
        float matrixes[] = {
            /* Projection Matrix (60deg FOV, 3:2 aspect ratio, [1, 256] clipping plane range */
            1.155f,  0.000f,  0.000f,  0.000f,
            0.000f,  1.732f,  0.000f,  0.000f,
            0.000f,  0.000f, -1.008f, -1.000f,
            0.000f,  0.000f, -2.008f,  0.000f,
            /* Model Matrix (identity) */
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f,
            /* View Matrix (distance of 2.5) */
            1.0f,  0.0f,  0.0f,  0.0f,
            0.0f,  1.0f,  0.0f,  0.0f,
            0.0f,  0.0f,  1.0f,  0.0f,
            0.0f,  0.0f, -2.5f,  1.0f,
        };

        struct {
            void* bytes;
            int size;
            VkBufferUsageFlagBits usage;
            VkDeviceMemory memory;
            VkBuffer buffer;
        } data [] = {
            {(void*)vertexes,   18 * sizeof(float), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,  VK_NULL_HANDLE, VK_NULL_HANDLE},
            {(void*)indexes,     3 * sizeof(int),   VK_BUFFER_USAGE_INDEX_BUFFER_BIT,   VK_NULL_HANDLE, VK_NULL_HANDLE},
            {(void*)matrixes,   38 * sizeof(float), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_NULL_HANDLE, VK_NULL_HANDLE},
        };
    {
        for (int i = 0; i < 3; i++) {
            VkBufferCreateInfo info = {0};
            info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            info.size = data[i].size;
            info.usage = data[i].usage;

            vk_result = vkCreateBuffer(device, &info, 0, &data[i].buffer);
            if (vk_result != VK_SUCCESS) {
                fprintf(stderr, "vkCreateBuffer() no. %i failed, result code [%i]: %s\n",
                        i, vk_result, string_VkResult(vk_result));
                return 28;
            }

            VkMemoryRequirements memoryRequirements;
            vkGetBufferMemoryRequirements(device, data[i].buffer, &memoryRequirements);

            unsigned flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            int typeIndex = -1;
            for (int j = 0; j < gpuMemoryProperties.memoryTypeCount; j++) {
                if (memoryRequirements.memoryTypeBits & (1 << j)
                    && (gpuMemoryProperties.memoryTypes[j].propertyFlags & flags) == flags)
                {
                    typeIndex = j;
                    break;
                }
            }

            if (typeIndex < 0) {
                fprintf(stderr, "Could not find an appropriate memory type\n");
                return 28;
            }

            VkMemoryAllocateInfo allocInfo = {0};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memoryRequirements.size;
            allocInfo.memoryTypeIndex = typeIndex;

            vk_result = vkAllocateMemory(device, &allocInfo, 0, &data[i].memory);
            if (vk_result != VK_SUCCESS) {
                fprintf(stderr, "vkAllocateMemory() failed, result code [%i]: %s\n",
                        vk_result, string_VkResult(vk_result));
                return 28;
            }

            void* buffer;
            vk_result = vkMapMemory(device, data[i].memory, 0, allocInfo.allocationSize, 0, &buffer);
            if (vk_result != VK_SUCCESS) {
                fprintf(stderr, "vkMapMemory() no. %i failed, result code [%i]: %s\n",
                        i, vk_result, string_VkResult(vk_result));
                return 28;
            }

            memcpy(buffer, data[i].bytes, data[i].size);
            vkUnmapMemory(device, data[i].memory);

            vk_result = vkBindBufferMemory(device, data[i].buffer, data[i].memory, 0);
            if (vk_result != VK_SUCCESS) {
                fprintf(stderr, "vkBindBufferMemory() no. %i failed, result code [%i]: %s\n",
                        i, vk_result, string_VkResult(vk_result));
                return 28;
            }
        }
    }

    /*
     * 29) Describe the MVP-matrix to a uniform descriptor
     */

    VkDescriptorBufferInfo uniformInfo = {0};
    uniformInfo.buffer = data[2].buffer;
    uniformInfo.offset = 0;
    uniformInfo.range = data[2].size;

    /*
     * 30) Create descriptor set layout
     */

    VkDescriptorSetLayout descriptorSetLayout;
    {
        VkDescriptorSetLayoutBinding binding = {0};
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo info = {0};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = 1;
        info.pBindings = &binding;

        vk_result = vkCreateDescriptorSetLayout(device, &info, 0, &descriptorSetLayout);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateDescriptorSetLayout() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 30;
        }
    }

    /* TODO Continue with vk_triangle.c */
    /* TODO Write some custom shaders and compile to SPIR-V */

    return 0;
}
