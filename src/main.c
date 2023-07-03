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

    unsigned int vk_instanceExtensionCount = 0;
    const char** vk_instanceExtensionRequest = glfwGetRequiredInstanceExtensions(&vk_instanceExtensionCount);
    if (!vk_instanceExtensionRequest) {
        const char* errstr;
        int code = glfwGetError(&errstr);
        fprintf(stderr, "Could not find any Vulkan extensions, error code [%i]:\n%s\n", code, errstr);
        return 2;
    }

    /*
     * 3) Vulkan lives! Create an instance
     */
    VkApplicationInfo vk_appInfo = {0};
    vk_appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vk_appInfo.pApplicationName = "Hello Triangle";
    vk_appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    vk_appInfo.pEngineName = "No Engine";
    vk_appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    vk_appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo vk_createInfo = {0};
    vk_createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_createInfo.pApplicationInfo = &vk_appInfo;
    vk_createInfo.enabledExtensionCount = vk_instanceExtensionCount;
    vk_createInfo.ppEnabledExtensionNames = vk_instanceExtensionRequest;

    VkInstance vk_instance;
    VkResult vk_result = vkCreateInstance(&vk_createInfo, 0, &vk_instance);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateInstance() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 3;
    }

    /*
     * 4) Get list of GPUs and pick one
     */

    int vk_gpuCount = 0;
    vk_result = vkEnumeratePhysicalDevices(vk_instance, &vk_gpuCount, 0);
    if (vk_gpuCount <= 0) {
        fprintf(stderr, "No graphics hardware found, physical device count = %i\n"
                "\tFailed with result code [%i]: %s\n", vk_gpuCount, vk_result, string_VkResult(vk_result));
        return 4;
    }

    vk_gpuCount = 1;
    VkPhysicalDevice vk_gpu = {0};
    vk_result = vkEnumeratePhysicalDevices(vk_instance, &vk_gpuCount, &vk_gpu);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumeratePhysicalDevices() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 4;
    }

    /*
     * 5) Pick a queue family
     */
    int vk_queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vk_gpu, &vk_queueCount, 0);
    if (vk_queueCount <= 0) {
        fprintf(stderr, "No Queue families found\n");
        return 5;
    }

    VkQueueFamilyProperties *vk_qfp = malloc(vk_queueCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(vk_gpu, &vk_queueCount, vk_qfp);

    int vk_queueIndex = -1;
    for (int i = 0; i < vk_queueCount; i++) {
        if (vk_qfp[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            vk_queueIndex = i;
            break;
        }
    }
    free(vk_qfp);

    if (vk_queueIndex < 0) {
        fprintf(stderr, "Could not find a queue with graphics support\n");
        return 5;
    }

    /*
     * 6) Check that the chosen queue family supports presentation
     */
    if (!glfwGetPhysicalDevicePresentationSupport(vk_instance, vk_gpu, vk_queueIndex)) {
        fprintf(stderr, "The selected queue family does not support present mode\n");
        return 6;
    }

    /*
     * 7) Get all the Vulkan _device_ extensions (as opposed to the instance exceptions)
     */
    unsigned int vk_deviceExtensionCount = 0;
    vk_result = vkEnumerateDeviceExtensionProperties(vk_gpu, 0, &vk_deviceExtensionCount, 0);
    if (vk_deviceExtensionCount <= 0 || vk_result != VK_SUCCESS) {
        fprintf(stderr, "Could not find any Vulkan device extensions [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 7;
    }

    VkExtensionProperties* vk_deviceExtensionProperties = calloc(vk_deviceExtensionCount, sizeof(VkExtensionProperties));
    vk_result = vkEnumerateDeviceExtensionProperties(vk_gpu, 0, &vk_deviceExtensionCount, vk_deviceExtensionProperties);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkEnumerateDeviceExtensionProperties() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 7;
    }

    const char** vk_deviceExtensions = calloc(vk_deviceExtensionCount, sizeof(void*));
    for (int i = 0; i < vk_deviceExtensionCount; i++) {
        vk_deviceExtensions[i] = &vk_deviceExtensionProperties[i].extensionName[0];
    }

    /*
     * 8) Create a virtual device for Vulkan. This is our primary interface between the program and the GPU
     */
    float vk_priority = 0.0f;
    VkDeviceQueueCreateInfo vk_queueInfo = {0};
    vk_queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vk_queueInfo.queueFamilyIndex = vk_queueIndex;
    vk_queueInfo.queueCount = 1;
    vk_queueInfo.pQueuePriorities = &vk_priority;

    VkDeviceCreateInfo vk_deviceInfo = {0};
    vk_deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vk_deviceInfo.queueCreateInfoCount = 1;
    vk_deviceInfo.pQueueCreateInfos = &vk_queueInfo;
    vk_deviceInfo.enabledExtensionCount = vk_deviceExtensionCount;
    vk_deviceInfo.ppEnabledExtensionNames = vk_deviceExtensions;

    VkDevice vk_device = {0};
    vk_result = vkCreateDevice(vk_gpu, &vk_deviceInfo, 0, &vk_device);
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
            vkGetInstanceProcAddr(vk_instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
        vkGetPhysicalDeviceSurfaceFormats = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)
            vkGetInstanceProcAddr(vk_instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
        vkCreateSwapchain = (PFN_vkCreateSwapchainKHR) vkGetInstanceProcAddr(vk_instance, "vkCreateSwapchainKHR");
        vkDestroySwapchain = (PFN_vkDestroySwapchainKHR) vkGetInstanceProcAddr(vk_instance, "vkDestroySwapchainKHR");
        vkGetSwapchainImages = (PFN_vkGetSwapchainImagesKHR) vkGetInstanceProcAddr(vk_instance, "vkGetSwapchainImagesKHR");
        vkAcquireNextImage = (PFN_vkAcquireNextImageKHR) vkGetInstanceProcAddr(vk_instance, "vkAcquireNextImageKHR");
        vkQueuePresent = (PFN_vkQueuePresentKHR) vkGetInstanceProcAddr(vk_instance, "vkQueuePresentKHR");

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

    VkSurfaceKHR vk_surface;
    vk_result = glfwCreateWindowSurface(vk_instance, window, 0, &vk_surface);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "glfwCreateWindowSurface() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 10;
    }

    /*
     * 11) Determine the color format
     */

    {
        int colorFormatCount = 0;
        vk_result = vkGetPhysicalDeviceSurfaceFormats(vk_gpu, vk_surface, &colorFormatCount, 0);
        if (colorFormatCount <= 0 || vk_result != VK_SUCCESS) {
            fprintf(stderr, "Could not find any color formats for the window surface, result code [%i]: %s\n",
                    vk_result, string_VkResult(vk_result));
            return 11;
        }

        VkSurfaceFormatKHR* colors = malloc(colorFormatCount * sizeof(VkSurfaceFormatKHR));
        vk_result = vkGetPhysicalDeviceSurfaceFormats(vk_gpu, vk_surface, &colorFormatCount, colors);
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
    }

    /*
     * 12) Get information about OS-specific surface capabilities
     */

    VkSurfaceCapabilitiesKHR vk_surfaceCapabilities;
    vk_result = vkGetPhysicalDeviceSurfaceCapabilities(vk_gpu, vk_surface, &vk_surfaceCapabilities);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkGetPhysicalDeviceSurfaceCapabilities() failed, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 12;
    }

    if (vk_surfaceCapabilities.currentExtent.width == 0xFFFFFFFF) {
        vk_surfaceCapabilities.currentExtent.width = width;
        vk_surfaceCapabilities.currentExtent.height = height;
    }

    /* TODO This is where you use vkGetPhysicalDeviceSurfacePresentModeKHR() to get a non-vsync presentation mode */

    /*
     * 13) Select the composite alpha format
     */

    VkCompositeAlphaFlagBitsKHR vk_alphaFormat = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    {
        VkCompositeAlphaFlagBitsKHR alphaList[] = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
        };

        for (int i = 0; i < sizeof(alphaList) / sizeof(VkCompositeAlphaFlagBitsKHR); i++) {
            if (vk_surfaceCapabilities.supportedCompositeAlpha & alphaList[i]) {
                vk_alphaFormat = alphaList[i];
                break;
            }
        }
    }

    /*
     * 14) Create a swapchain.
     */

    VkSwapchainKHR vk_swapchain;
    {
        int swapImageCount = vk_surfaceCapabilities.minImageCount + 1;
        if (vk_surfaceCapabilities.maxImageCount > 0 && swapImageCount > vk_surfaceCapabilities.maxImageCount)
            swapImageCount = vk_surfaceCapabilities.maxImageCount;

        VkSwapchainCreateInfoKHR swapchainCreateInfo = {0};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        /* TODO Complete step 14 */
    }

    /* TODO Continue with vk_triangle.c */
    /* TODO Write some custom shaders and compile to SPIR-V */

    return 0;
}
