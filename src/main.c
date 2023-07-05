/* External Libraries */
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

/*#include <vulkan/vk_enum_string_helper.h>
 * FIXME Need to use validation layers for this, should only be in debug builds anyway
 * TODO Investigate validation layers in general, they will surely be needed eventually.
 * TODO CreateInfo objects should be able to have a restricted lifetime */
char*
string_VkResult(VkResult r)
{
    return 0;
}

/* System Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Core Layer */
#include "main.h"

/* TODO Make configurable */
const int width = 1280;
const int height = 720;
unsigned char vertShaderSpv[];
const int vertShaderSpvSize = 1372;
unsigned char fragShaderSpv[];
const int fragShaderSpvSize = 500;

int
main(int argc, char* argv[])
{
    /*
     * 41-step vulkan initialization based on https://github.com/jbendtsen/stuff/blob/master/triangle.c
     * 1) Create GLFW Window
     */

    glfwInit();
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
     * 3a) Query validation layers
     */

    unsigned int instanceLayerCount;
    VkResult vk_result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, 0);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumerateInstanceLayerProperties() failed to detect layer count, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 3;
    }

    VkLayerProperties* instanceLayers = calloc(instanceLayerCount, sizeof(VkLayerProperties));
    vk_result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayers);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumerateInstanceLayerProperties() failed to retrieve layer properties, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 3;
    }

    printf("Validation layers detected:\n");
    for (int i = 0; i < instanceLayerCount; i++) {
        printf("\t%s\n", instanceLayers[i].layerName);
    }

    /*
     * 3) Vulkan lives! Create an instance
     */

    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Vulkan";
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
    vk_result = vkCreateInstance(&instanceCreateInfo, 0, &instance);
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
     * 7) Get all the Vulkan _device_ extensions (as opposed to instance extensions)
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

    VkSurfaceFormatKHR colorFormat = {0};
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

    /*
     * 14) Create a swapchain.
     */

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

    VkSwapchainKHR swapchain;
    vk_result = vkCreateSwapchain(device, &swapchainCreateInfo, 0, &swapchain);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateSwapchain() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 14;
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

    VkImageView* imageViews = calloc(imageCount, sizeof(VkImageView));
    for (int i = 0; i < imageCount; i++) {
        imageViewCreateInfo.image = images[i];
        vk_result = vkCreateImageView(device, &imageViewCreateInfo, 0, &imageViews[i]);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateImageView() num %i failed, result code [%i]: %s\n",
                    i, vk_result, string_VkResult(vk_result));
            return 16;
        }
    }

    /*
     * 17) Create a command pool
     */

    VkCommandPoolCreateInfo commandPoolCreateInfo = {0};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = queueIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool commandPool;
    vk_result = vkCreateCommandPool(device, &commandPoolCreateInfo, 0, &commandPool);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateCommandPool() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 17;
    }

    /*
     * 18) Allocate command buffers
     */

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {0};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = imageCount;

    VkCommandBuffer* commandBuffers = calloc(imageCount, sizeof(VkCommandBuffer));
    vk_result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkAllocateCommandBuffers() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 18;
    }

    /*
     * 19) Depth format
     */

    VkFormat formats[] = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM,
    };

    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
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

    /*
     * 20) Create depth stencil image
     */

    VkImageCreateInfo depthStencilImageCreateInfo = {0};
    depthStencilImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthStencilImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    depthStencilImageCreateInfo.format = depthFormat;
    depthStencilImageCreateInfo.extent = (VkExtent3D) {
        .width = surfaceCapabilities.currentExtent.width,
        .height = surfaceCapabilities.currentExtent.height,
        .depth = 1
    };
    depthStencilImageCreateInfo.mipLevels = 1;
    depthStencilImageCreateInfo.arrayLayers = 1;
    depthStencilImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthStencilImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthStencilImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImage depthImage;
    vk_result = vkCreateImage(device, &depthStencilImageCreateInfo, 0, &depthImage);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateImage() failed for depth-stencil image, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 20;
    }

    /*
     * 21) Allocate memory for the depth stencil
     */

    VkPhysicalDeviceMemoryProperties gpuMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(gpu, &gpuMemoryProperties);

    VkMemoryRequirements depthMemoryRequirements;
    vkGetImageMemoryRequirements(device, depthImage, &depthMemoryRequirements);

    int memoryTypeIndex = -1;
    for (int i = 0; i < gpuMemoryProperties.memoryTypeCount; i++) {
        if ((depthMemoryRequirements.memoryTypeBits & (1 << i))
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

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = depthMemoryRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    VkDeviceMemory depthMemory;
    vk_result = vkAllocateMemory(device, &allocInfo, 0, &depthMemory);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkAllocateMemory() failed for depth-stencil image, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 21;
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

    VkImageAspectFlagBits depthAspect = depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT ?
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT :
        VK_IMAGE_ASPECT_DEPTH_BIT;

    VkImageViewCreateInfo depthViewCreateInfo = {0};
    depthViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthViewCreateInfo.image = depthImage;
    depthViewCreateInfo.format = depthFormat;
    depthViewCreateInfo.subresourceRange.baseMipLevel = 0;
    depthViewCreateInfo.subresourceRange.levelCount = 1;
    depthViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    depthViewCreateInfo.subresourceRange.layerCount = 1;
    depthViewCreateInfo.subresourceRange.aspectMask = depthAspect;

    VkImageView depthView;
    vk_result = vkCreateImageView(device, &depthViewCreateInfo, 0, &depthView);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateImageView() failed for depth-stencil image, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 23;
    }

    /*
     * 24) Set up the render pass
     */

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

    VkSubpassDependency subpassDependencies[] = {
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
    renderpassCreateInfo.pDependencies = subpassDependencies;

    VkRenderPass renderpass;
    vk_result = vkCreateRenderPass(device, &renderpassCreateInfo, 0, &renderpass);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateRenderPass() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 24;
    }

    /*
     * 25) Create the frame buffers
     */

    VkImageView framebufferViews[2];
    framebufferViews[1] = depthView;

    VkFramebufferCreateInfo framebufferCreateInfo = {0};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = renderpass;
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.pAttachments = framebufferViews;
    framebufferCreateInfo.width = surfaceCapabilities.currentExtent.width;
    framebufferCreateInfo.height = surfaceCapabilities.currentExtent.height;
    framebufferCreateInfo.layers = 1;

    VkFramebuffer* framebuffers = calloc(imageCount, sizeof(VkFramebuffer));
    for (int i = 0; i < imageCount; i++) {
        framebufferViews[0] = imageViews[i];
        vk_result = vkCreateFramebuffer(device, &framebufferCreateInfo, 0, &framebuffers[i]);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateFramebuffer() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 25;
        }
    }

    /*
     * 26) Create semaphores for synchronizing draw commands and image presentation
     */

    VkSemaphoreCreateInfo semaphoreCreateInfo = {0};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaPresent;
    vk_result = vkCreateSemaphore(device, &semaphoreCreateInfo, 0, &semaPresent);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateSemaphore() failed for presentation semaphore, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 26;
    }

    VkSemaphore semaRender;
    vk_result = vkCreateSemaphore(device, &semaphoreCreateInfo, 0, &semaRender);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateSemaphore() failed for rendering semaphore, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 26;
    }

    /*
     * 27) Create wait fences, one for each image
     */

    VkFenceCreateInfo fenceCreateInfo = {0};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence* fences = calloc(imageCount, sizeof(VkFence));
    for (int i = 0; i < imageCount; i++) {
        vk_result = vkCreateFence(device, &fenceCreateInfo, 0, &fences[i]);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateFence() failed for fence no. %i, result code [%i]: %s\n",
                    i, vk_result, string_VkResult(vk_result));
            return 27;
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
        {(void*)matrixes,   48 * sizeof(float), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_NULL_HANDLE, VK_NULL_HANDLE},
    };

    for (int i = 0; i < 3; i++) {
        VkBufferCreateInfo bufferCreateInfo = {0};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = data[i].size;
        bufferCreateInfo.usage = data[i].usage;

        vk_result = vkCreateBuffer(device, &bufferCreateInfo, 0, &data[i].buffer);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkCreateBuffer() no. %i failed, result code [%i]: %s\n",
                    i, vk_result, string_VkResult(vk_result));
            return 28;
        }

        VkMemoryRequirements bufferMemoryRequirements;
        vkGetBufferMemoryRequirements(device, data[i].buffer, &bufferMemoryRequirements);

        unsigned int flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        int typeIndex = -1;
        for (int j = 0; j < gpuMemoryProperties.memoryTypeCount; j++) {
            if (bufferMemoryRequirements.memoryTypeBits & (1 << j)
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

        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = bufferMemoryRequirements.size;
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

    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {0};
    descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorSetLayoutBinding.descriptorCount = 1;
    descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {0};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 1;
    descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

    VkDescriptorSetLayout descriptorSetLayout;
    vk_result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, 0, &descriptorSetLayout);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateDescriptorSetLayout() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 30;
    }

    /*
     * 31) Create pipeline layout
     */

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {0};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

    VkPipelineLayout pipelineLayout;
    vk_result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, 0, &pipelineLayout);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreatePipelineLayout() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 31;
    }

    /*
     * 32) Prepare Shaders
     * TODO Write them as GLSL code and compile into SPIR-V
     */

    VkShaderModuleCreateInfo shaderCreateInfo = {0};
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    VkShaderModule vertShader;
    shaderCreateInfo.codeSize = vertShaderSpvSize;
    shaderCreateInfo.pCode = (unsigned int *)vertShaderSpv;
    vk_result = vkCreateShaderModule(device, &shaderCreateInfo, 0, &vertShader);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateShaderModule() failed for vertex shader, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 32;
    }

    VkShaderModule fragShader;
    shaderCreateInfo.codeSize = fragShaderSpvSize;
    shaderCreateInfo.pCode = (unsigned int *)fragShaderSpv;
    vk_result = vkCreateShaderModule(device, &shaderCreateInfo, 0, &fragShader);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateShaderModule() failed for fragment shader, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 32;
    }

    /*
     * 33) Create graphics pipeline. This where everything comes together and the magic happens.
     */

    VkPipelineInputAssemblyStateCreateInfo asmInfo = {0};
    asmInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    asmInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rasterInfo = {0};
    rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterInfo.lineWidth = 1.f;

    VkPipelineColorBlendAttachmentState blendAttachment = {0};
    blendAttachment.colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo blendInfo = {0};
    blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendInfo.attachmentCount = 1;
    blendInfo.pAttachments = &blendAttachment;

    VkPipelineViewportStateCreateInfo vpInfo = {0};
    vpInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vpInfo.viewportCount = 1;
    vpInfo.scissorCount = 1;

    VkDynamicState dynStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynInfo = {0};
    dynInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynInfo.pDynamicStates = dynStates;
    dynInfo.dynamicStateCount = 2;

    VkPipelineDepthStencilStateCreateInfo depthInfo = {0};
    depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthInfo.depthTestEnable = VK_TRUE;
    depthInfo.depthWriteEnable = VK_TRUE;
    depthInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthInfo.back.failOp = VK_STENCIL_OP_KEEP;
    depthInfo.back.passOp = VK_STENCIL_OP_KEEP;
    depthInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthInfo.front = depthInfo.back;

    VkPipelineMultisampleStateCreateInfo msaaInfo = {0};
    msaaInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    msaaInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkVertexInputBindingDescription vboInfo = {0};
    vboInfo.binding = 0;
    vboInfo.stride = 6 * sizeof(float); // position XYZ, color RGB
    vboInfo.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vboAtt[] = {
        { /* Position */
            .binding    = 0,
            .location   = 0,
            .format     = VK_FORMAT_R32G32B32_SFLOAT,
            .offset     = 0,
        },
        { /* Color */
            .binding    = 0,
            .location   = 1,
            .format     = VK_FORMAT_R32G32B32_SFLOAT,
            .offset     = 3 * sizeof(float),
        },
    };

    VkPipelineVertexInputStateCreateInfo vertInfo = {0};
    vertInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertInfo.vertexBindingDescriptionCount = 1;
    vertInfo.pVertexBindingDescriptions = &vboInfo;
    vertInfo.vertexAttributeDescriptionCount = 2;
    vertInfo.pVertexAttributeDescriptions = vboAtt;

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertShader,
            .pName  = "main",
        },
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragShader,
            .pName  = "main",
        },
    };

    VkGraphicsPipelineCreateInfo pipeInfo = {0};
    pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeInfo.layout = pipelineLayout;
    pipeInfo.stageCount = 2;
    pipeInfo.pStages = shaderStages;
    pipeInfo.pVertexInputState = &vertInfo;
    pipeInfo.pInputAssemblyState = &asmInfo;
    pipeInfo.pRasterizationState = &rasterInfo;
    pipeInfo.pColorBlendState = &blendInfo;
    pipeInfo.pMultisampleState = &msaaInfo;
    pipeInfo.pViewportState = &vpInfo;
    pipeInfo.pDepthStencilState = &depthInfo;
    pipeInfo.renderPass = renderpass;
    pipeInfo.pDynamicState = &dynInfo;

    /* Note: Without validation layers this will fail with Error Unknown
     * if you give it uninitialized shaders */
    VkPipeline pipeline;
    vk_result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1 , &pipeInfo, 0, &pipeline);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateGraphicsPipelines() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 33;
    }

    /*
     * 34) Destroy shader modules (not needed anymore)
     */
    vkDestroyShaderModule(device, vertShader, 0);
    vkDestroyShaderModule(device, fragShader, 0);

    /*
     * 35) Create a descriptor pool for the descriptor set
     */
    VkDescriptorPoolSize poolSizeInfo = {0};
    poolSizeInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizeInfo.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolCreateInfo = {0};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = 1;
    poolCreateInfo.pPoolSizes = &poolSizeInfo;
    poolCreateInfo.maxSets = 1;

    VkDescriptorPool descriptorPool;
    vk_result = vkCreateDescriptorPool(device, &poolCreateInfo, 0, &descriptorPool);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateDescriptorPool() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 35;
    }

    /*
     * 36) Allocate a descriptor set to allow passing additional data into shaders
     */
    VkDescriptorSetAllocateInfo descriptorAlloc = {0};
    descriptorAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorAlloc.descriptorPool = descriptorPool;
    descriptorAlloc.descriptorSetCount = 1;
    descriptorAlloc.pSetLayouts = &descriptorSetLayout;

    VkDescriptorSet descriptorSet;
    vk_result = vkAllocateDescriptorSets(device, &descriptorAlloc, &descriptorSet);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkAllocateDescriptorSets() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 36;
    }

    /*
     * 37) Set up the descriptor set
     */
    VkWriteDescriptorSet writeInfo = {0};
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet = descriptorSet;
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeInfo.pBufferInfo = &uniformInfo;
    writeInfo.dstBinding = 0;

    vkUpdateDescriptorSets(device, 1, &writeInfo, 0, 0);

    /*
     * 38) Construct the command buffers
     */

    VkCommandBufferBeginInfo cmdInfo = {0};
    cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkClearValue clearValues[] = {
        { /* Color Buffer */
            .color = { { 0.f, 0.f, 0.2f, 1.f } }
        },
        { /* Depth/Stencil Buffer */
            .depthStencil = { 1.f, 0 }
        },
    };

    VkRenderPassBeginInfo rpInfo = {0};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = renderpass;
    rpInfo.renderArea.offset.x = 0;
    rpInfo.renderArea.offset.y = 0;
    rpInfo.renderArea.extent = surfaceCapabilities.currentExtent;
    rpInfo.clearValueCount = 2;
    rpInfo.pClearValues = clearValues;

    VkRect2D* scissor = &rpInfo.renderArea;

    VkViewport viewport = {0};
    viewport.width = (float) surfaceCapabilities.currentExtent.width;
    viewport.height = (float) surfaceCapabilities.currentExtent.height;
    viewport.minDepth = 0.f;
    viewport.minDepth = 1.f;

    for (int i = 0; i < imageCount; i++) {
        vk_result = vkBeginCommandBuffer(commandBuffers[i], &cmdInfo);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkBeginCommandBuffer() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 38;
        }

        rpInfo.framebuffer = framebuffers[i];
        vkCmdBeginRenderPass(commandBuffers[i], &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[i], 0, 1, scissor);

        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout, 0, 1, &descriptorSet, 0, 0);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &data[0].buffer, &offset);
        vkCmdBindIndexBuffer(commandBuffers[i], data[1].buffer, 0, VK_INDEX_TYPE_UINT32);

        const int indexCount = 3;
        vkCmdDrawIndexed(commandBuffers[i], indexCount, 1, 0, 0, 1);

        vkCmdEndRenderPass(commandBuffers[i]);
        vk_result = vkEndCommandBuffer(commandBuffers[i]);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkEndCommandBuffer() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 38;
        }
    }

    /*
     * 39) Prepare Main Loop
     */
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.pWaitSemaphores = &semaPresent;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaRender;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.commandBufferCount = 1;

    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pWaitSemaphores = &semaRender;
    presentInfo.waitSemaphoreCount = 1;

    VkQueue queue;
    vkGetDeviceQueue(device, queueIndex, 0, &queue);

    /*
     * 40) Main Loop
     */

    unsigned long long max64 = -1;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        int index;

        vk_result = vkAcquireNextImage(device, swapchain, max64, semaPresent, 0, &index);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkAcquireNextImage() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 40;
        }

        vk_result = vkWaitForFences(device, 1, &fences[index], VK_TRUE, max64);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkWaitForFences() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 40;
        }

        vk_result = vkResetFences(device, 1, &fences[index]);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkResetFences() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 40;
        }

        submitInfo.pCommandBuffers = &commandBuffers[index];
        vk_result = vkQueueSubmit(queue, 1, &submitInfo, fences[index]);
        if (vk_result != VK_SUCCESS) {
            fprintf(stderr, "vkQueueSubmit() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 40;
        }

        presentInfo.pImageIndices = &index;
        vk_result = vkQueuePresent(queue, &presentInfo);
        if (vk_result != VK_SUCCESS && vk_result != VK_SUBOPTIMAL_KHR) {
            fprintf(stderr, "vkQueuePresent() failed, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 40;
        }
    }

    /*
     * 41) Clean up
     * Reminder: Most OSes will automatically do all of this the moment you quit. In all desktop situations
     * this is only necessary for conserving resources over the lifetime of the program, not its end.
     */
    for (int i = 0; i < 3; i++) {
        vkDestroyBuffer(device, data[i].buffer, 0);
        vkFreeMemory(device, data[i].memory, 0);
    }

    vkDestroyImageView(device, depthView, 0);
    vkDestroyImage(device, depthImage, 0);
    vkFreeMemory(device, depthMemory, 0);

    vkDestroySemaphore(device, semaPresent, 0);
    vkDestroySemaphore(device, semaRender, 0);

    for (int i = 0; i < imageCount; i++)
        vkDestroyFence(device, fences[i], 0);
    free(fences);

    for (int i = 0; i < imageCount; i++)
        vkDestroyFramebuffer(device, framebuffers[i], 0);
    free(framebuffers);

    vkDestroyCommandPool(device, commandPool, 0);
    free(commandBuffers);

    vkDestroyDescriptorPool(device, descriptorPool, 0);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, 0);

    vkDestroyPipelineLayout(device, pipelineLayout, 0);
    vkDestroyPipeline(device, pipeline, 0);
    vkDestroyRenderPass(device, renderpass, 0);

    for (int i = 0; i < imageCount; i++)
        vkDestroyImageView(device, imageViews[i], 0);
    free(imageViews);

    free(images);

    vkDestroySwapchain(device, swapchain, 0);
    vkDestroyDevice(device, 0);

    vkDestroySurfaceKHR(instance, surface, 0);
    vkDestroyInstance(instance, 0);

    free(deviceExtensions);
    free(deviceExtensionProperties);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

/* TODO Compile and pass in shaders manually, remembering to run the SPIR-V compiler first
 * as part of the Make process */

// Fragment Shader (https://github.com/SaschaWillems/Vulkan/data/shaders/triangle/triangle.frag)
/*
#version 450

layout (location = 0) in vec3 inColor;

layout (location = 0) out vec4 outFragColor;

void main()
{
    outFragColor = vec4(inColor, 1.0);
}
*/
unsigned char fragShaderSpv[] = {
    0x03,0x02,0x23,0x07,0x00,0x00,0x01,0x00,0x07,0x00,0x08,0x00,0x13,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x11,0x00,0x02,0x00,
    0x01,0x00,0x00,0x00,0x0b,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0x47,0x4c,0x53,0x4c,0x2e,0x73,0x74,0x64,0x2e,0x34,0x35,0x30,
    0x00,0x00,0x00,0x00,0x0e,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x0f,0x00,0x07,0x00,0x04,0x00,0x00,0x00,
    0x04,0x00,0x00,0x00,0x6d,0x61,0x69,0x6e,0x00,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x0c,0x00,0x00,0x00,0x10,0x00,0x03,0x00,
    0x04,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x03,0x00,0x03,0x00,0x02,0x00,0x00,0x00,0xc2,0x01,0x00,0x00,0x05,0x00,0x04,0x00,
    0x04,0x00,0x00,0x00,0x6d,0x61,0x69,0x6e,0x00,0x00,0x00,0x00,0x05,0x00,0x06,0x00,0x09,0x00,0x00,0x00,0x6f,0x75,0x74,0x46,
    0x72,0x61,0x67,0x43,0x6f,0x6c,0x6f,0x72,0x00,0x00,0x00,0x00,0x05,0x00,0x04,0x00,0x0c,0x00,0x00,0x00,0x69,0x6e,0x43,0x6f,
    0x6c,0x6f,0x72,0x00,0x47,0x00,0x04,0x00,0x09,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x0c,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x21,0x00,0x03,0x00,
    0x03,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x16,0x00,0x03,0x00,0x06,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x17,0x00,0x04,0x00,
    0x07,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x20,0x00,0x04,0x00,0x08,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
    0x07,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,0x08,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x17,0x00,0x04,0x00,
    0x0a,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x20,0x00,0x04,0x00,0x0b,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
    0x0a,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,0x0b,0x00,0x00,0x00,0x0c,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,
    0x06,0x00,0x00,0x00,0x0e,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,0x36,0x00,0x05,0x00,0x02,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0xf8,0x00,0x02,0x00,0x05,0x00,0x00,0x00,0x3d,0x00,0x04,0x00,0x0a,0x00,0x00,0x00,
    0x0d,0x00,0x00,0x00,0x0c,0x00,0x00,0x00,0x51,0x00,0x05,0x00,0x06,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x0d,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x51,0x00,0x05,0x00,0x06,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x0d,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
    0x51,0x00,0x05,0x00,0x06,0x00,0x00,0x00,0x11,0x00,0x00,0x00,0x0d,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x50,0x00,0x07,0x00,
    0x07,0x00,0x00,0x00,0x12,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x11,0x00,0x00,0x00,0x0e,0x00,0x00,0x00,
    0x3e,0x00,0x03,0x00,0x09,0x00,0x00,0x00,0x12,0x00,0x00,0x00,0xfd,0x00,0x01,0x00,0x38,0x00,0x01,0x00,
};

// Vertex Shader (https://github.com/SaschaWillems/Vulkan/data/shaders/triangle/triangle.vert)
/*
#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

layout (binding = 0) uniform UBO
{
    mat4 projectionMatrix;
    mat4 modelMatrix;
    mat4 viewMatrix;
} ubo;

layout (location = 0) out vec3 outColor;

out gl_PerVertex
{
    vec4 gl_Position;
};


void main()
{
    outColor = inColor;
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);
}
*/
unsigned char vertShaderSpv[] = {
    0x03,0x02,0x23,0x07,0x00,0x00,0x01,0x00,0x07,0x00,0x08,0x00,0x2c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x11,0x00,0x02,0x00,
    0x01,0x00,0x00,0x00,0x0b,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0x47,0x4c,0x53,0x4c,0x2e,0x73,0x74,0x64,0x2e,0x34,0x35,0x30,
    0x00,0x00,0x00,0x00,0x0e,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x0f,0x00,0x09,0x00,0x00,0x00,0x00,0x00,
    0x04,0x00,0x00,0x00,0x6d,0x61,0x69,0x6e,0x00,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x10,0x00,0x00,0x00,
    0x22,0x00,0x00,0x00,0x03,0x00,0x03,0x00,0x02,0x00,0x00,0x00,0xc2,0x01,0x00,0x00,0x05,0x00,0x04,0x00,0x04,0x00,0x00,0x00,
    0x6d,0x61,0x69,0x6e,0x00,0x00,0x00,0x00,0x05,0x00,0x05,0x00,0x09,0x00,0x00,0x00,0x6f,0x75,0x74,0x43,0x6f,0x6c,0x6f,0x72,
    0x00,0x00,0x00,0x00,0x05,0x00,0x04,0x00,0x0b,0x00,0x00,0x00,0x69,0x6e,0x43,0x6f,0x6c,0x6f,0x72,0x00,0x05,0x00,0x06,0x00,
    0x0e,0x00,0x00,0x00,0x67,0x6c,0x5f,0x50,0x65,0x72,0x56,0x65,0x72,0x74,0x65,0x78,0x00,0x00,0x00,0x00,0x06,0x00,0x06,0x00,
    0x0e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x67,0x6c,0x5f,0x50,0x6f,0x73,0x69,0x74,0x69,0x6f,0x6e,0x00,0x05,0x00,0x03,0x00,
    0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x03,0x00,0x14,0x00,0x00,0x00,0x55,0x42,0x4f,0x00,0x06,0x00,0x08,0x00,
    0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x72,0x6f,0x6a,0x65,0x63,0x74,0x69,0x6f,0x6e,0x4d,0x61,0x74,0x72,0x69,0x78,
    0x00,0x00,0x00,0x00,0x06,0x00,0x06,0x00,0x14,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x6d,0x6f,0x64,0x65,0x6c,0x4d,0x61,0x74,
    0x72,0x69,0x78,0x00,0x06,0x00,0x06,0x00,0x14,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x76,0x69,0x65,0x77,0x4d,0x61,0x74,0x72,
    0x69,0x78,0x00,0x00,0x05,0x00,0x03,0x00,0x16,0x00,0x00,0x00,0x75,0x62,0x6f,0x00,0x05,0x00,0x04,0x00,0x22,0x00,0x00,0x00,
    0x69,0x6e,0x50,0x6f,0x73,0x00,0x00,0x00,0x47,0x00,0x04,0x00,0x09,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x47,0x00,0x04,0x00,0x0b,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x48,0x00,0x05,0x00,0x0e,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x47,0x00,0x03,0x00,0x0e,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
    0x48,0x00,0x04,0x00,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x48,0x00,0x05,0x00,0x14,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x23,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x48,0x00,0x05,0x00,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x07,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x48,0x00,0x04,0x00,0x14,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x05,0x00,0x00,0x00,
    0x48,0x00,0x05,0x00,0x14,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x23,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x48,0x00,0x05,0x00,
    0x14,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x48,0x00,0x04,0x00,0x14,0x00,0x00,0x00,
    0x02,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x48,0x00,0x05,0x00,0x14,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x23,0x00,0x00,0x00,
    0x80,0x00,0x00,0x00,0x48,0x00,0x05,0x00,0x14,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x10,0x00,0x00,0x00,
    0x47,0x00,0x03,0x00,0x14,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x47,0x00,0x04,0x00,0x16,0x00,0x00,0x00,0x22,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x47,0x00,0x04,0x00,0x16,0x00,0x00,0x00,0x21,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x47,0x00,0x04,0x00,
    0x22,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x21,0x00,0x03,0x00,
    0x03,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x16,0x00,0x03,0x00,0x06,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x17,0x00,0x04,0x00,
    0x07,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x20,0x00,0x04,0x00,0x08,0x00,0x00,0x00,0x03,0x00,0x00,0x00,
    0x07,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,0x08,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x20,0x00,0x04,0x00,
    0x0a,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,0x0a,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,
    0x01,0x00,0x00,0x00,0x17,0x00,0x04,0x00,0x0d,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x1e,0x00,0x03,0x00,
    0x0e,0x00,0x00,0x00,0x0d,0x00,0x00,0x00,0x20,0x00,0x04,0x00,0x0f,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x0e,0x00,0x00,0x00,
    0x3b,0x00,0x04,0x00,0x0f,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x15,0x00,0x04,0x00,0x11,0x00,0x00,0x00,
    0x20,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,0x11,0x00,0x00,0x00,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x18,0x00,0x04,0x00,0x13,0x00,0x00,0x00,0x0d,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x1e,0x00,0x05,0x00,0x14,0x00,0x00,0x00,
    0x13,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x20,0x00,0x04,0x00,0x15,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
    0x14,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,0x15,0x00,0x00,0x00,0x16,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x20,0x00,0x04,0x00,
    0x17,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x13,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,0x11,0x00,0x00,0x00,0x1a,0x00,0x00,0x00,
    0x02,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,0x11,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x3b,0x00,0x04,0x00,
    0x0a,0x00,0x00,0x00,0x22,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x2b,0x00,0x04,0x00,0x06,0x00,0x00,0x00,0x24,0x00,0x00,0x00,
    0x00,0x00,0x80,0x3f,0x20,0x00,0x04,0x00,0x2a,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x0d,0x00,0x00,0x00,0x36,0x00,0x05,0x00,
    0x02,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0xf8,0x00,0x02,0x00,0x05,0x00,0x00,0x00,
    0x3d,0x00,0x04,0x00,0x07,0x00,0x00,0x00,0x0c,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x3e,0x00,0x03,0x00,0x09,0x00,0x00,0x00,
    0x0c,0x00,0x00,0x00,0x41,0x00,0x05,0x00,0x17,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x16,0x00,0x00,0x00,0x12,0x00,0x00,0x00,
    0x3d,0x00,0x04,0x00,0x13,0x00,0x00,0x00,0x19,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x41,0x00,0x05,0x00,0x17,0x00,0x00,0x00,
    0x1b,0x00,0x00,0x00,0x16,0x00,0x00,0x00,0x1a,0x00,0x00,0x00,0x3d,0x00,0x04,0x00,0x13,0x00,0x00,0x00,0x1c,0x00,0x00,0x00,
    0x1b,0x00,0x00,0x00,0x92,0x00,0x05,0x00,0x13,0x00,0x00,0x00,0x1d,0x00,0x00,0x00,0x19,0x00,0x00,0x00,0x1c,0x00,0x00,0x00,
    0x41,0x00,0x05,0x00,0x17,0x00,0x00,0x00,0x1f,0x00,0x00,0x00,0x16,0x00,0x00,0x00,0x1e,0x00,0x00,0x00,0x3d,0x00,0x04,0x00,
    0x13,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x1f,0x00,0x00,0x00,0x92,0x00,0x05,0x00,0x13,0x00,0x00,0x00,0x21,0x00,0x00,0x00,
    0x1d,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x3d,0x00,0x04,0x00,0x07,0x00,0x00,0x00,0x23,0x00,0x00,0x00,0x22,0x00,0x00,0x00,
    0x51,0x00,0x05,0x00,0x06,0x00,0x00,0x00,0x25,0x00,0x00,0x00,0x23,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x51,0x00,0x05,0x00,
    0x06,0x00,0x00,0x00,0x26,0x00,0x00,0x00,0x23,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x51,0x00,0x05,0x00,0x06,0x00,0x00,0x00,
    0x27,0x00,0x00,0x00,0x23,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x50,0x00,0x07,0x00,0x0d,0x00,0x00,0x00,0x28,0x00,0x00,0x00,
    0x25,0x00,0x00,0x00,0x26,0x00,0x00,0x00,0x27,0x00,0x00,0x00,0x24,0x00,0x00,0x00,0x91,0x00,0x05,0x00,0x0d,0x00,0x00,0x00,
    0x29,0x00,0x00,0x00,0x21,0x00,0x00,0x00,0x28,0x00,0x00,0x00,0x41,0x00,0x05,0x00,0x2a,0x00,0x00,0x00,0x2b,0x00,0x00,0x00,
    0x10,0x00,0x00,0x00,0x12,0x00,0x00,0x00,0x3e,0x00,0x03,0x00,0x2b,0x00,0x00,0x00,0x29,0x00,0x00,0x00,0xfd,0x00,0x01,0x00,
    0x38,0x00,0x01,0x00,
};
