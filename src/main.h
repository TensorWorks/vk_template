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
} GlobalStorage;

static void render(GlobalStorage* g, VkCommandBuffer* cmd);
