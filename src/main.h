PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR   vkGetPhysicalDeviceSurfaceCapabilities;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR        vkGetPhysicalDeviceSurfaceFormats;

PFN_vkCreateSwapchainKHR    vkCreateSwapchain;
PFN_vkDestroySwapchainKHR   vkDestroySwapchain;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImages;
PFN_vkAcquireNextImageKHR   vkAcquireNextImage;
PFN_vkQueuePresentKHR       vkQueuePresent;

typedef struct GlobalStorage_ {
    VulkanContext vulkan;
    CImgui cimgui;

    uint32_t width;
    uint32_t height;
} GlobalStorage;

static void renderTriangle(GlobalStorage* g, VkRenderingInfoKHR* renderInfo, VkCommandBuffer cmd);
