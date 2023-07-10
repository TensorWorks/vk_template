static int ext_init(GlobalStorage* g);
static void ext_destroy(GlobalStorage* g);

/*
 * Vulkan
 * Enable validation by setting the following environment variable:
 *  export VK_LOADER_LAYERS_ENABLE=*api_dump,*validation
 */

typedef struct VulkanTexture
{
    VkImage                 image;
    VkDeviceMemory          memory;
    VkImageView             view;

    VkDescriptorSet         desc;
    VkDeviceSize            size;
    uint32_t                width;
    uint32_t                height;
} VulkanTexture;

typedef struct VulkanContext_
{
    VkDevice            device;
    VkQueue             queue;
    VkCommandPool       pool;

    VkSampler           nearestSampler;
    VkSampler           linearSampler;

    VkPhysicalDeviceMemoryProperties    memProps;
    VkPhysicalDeviceProperties          devProps;
    VkPhysicalDevice                    gpu;
} VulkanContext;

static int ext_vkFindMemoryType(
        VkPhysicalDeviceMemoryProperties* gpuMemoryProperties,
        uint32_t memoryTypeBits,
        VkMemoryPropertyFlags properties
);
static VkResult ext_vkCreateBuffer(
        VulkanContext* vulkan,
        void* data,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer* out_buffer,
        VkDeviceMemory* out_memory
);
static VkCommandBuffer ext_vkQuickCommandBegin(
        VulkanContext* vulkan
);
static void ext_vkQuickCommandEnd(
        VulkanContext* vulkan,
        VkCommandBuffer buffer
);
static VkResult ext_vkImageLayout(
        VulkanContext* vulkan,
        VkImage image,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout
);
static void ext_vkCopyBufferToImage(
        VulkanContext* vulkan,
        VkBuffer from,
        VkImage to,
        uint32_t width,
        uint32_t height
);
static VkResult ext_vkCreateTexture(
        VulkanContext* vulkan,
        VulkanTexture* texture,
        uint8_t* pixels,
        uint32_t width,
        uint32_t height
);
static void ext_vkDestroyTexture(
        VulkanContext* vulkan,
        VulkanTexture* texture
);

/*
 * CImgui
 */

#define CIMGUI_KEYMAP_COUNT 105

typedef struct CImgui_ {
    ImGuiContext*       context;

    VulkanTexture       fontTexture;
    ImGuiMouseCursor    cursor;
    GLFWcursor*         cursorMap   [ImGuiMouseCursor_COUNT];
    int                 glfwKey     [CIMGUI_KEYMAP_COUNT];
    ImGuiKey            imguiKey    [CIMGUI_KEYMAP_COUNT];
    char                keyDown     [CIMGUI_KEYMAP_COUNT];
} CImgui;

static const char* ext_cimguiGetClipboard(void* context);
static void ext_cimguiSetClipboard(void* context, const char* text);
