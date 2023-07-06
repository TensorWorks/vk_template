static int ext_init(GlobalStorage* g);
static void ext_destroy(GlobalStorage* g);

/*
 * Vulkan
 */

typedef struct VulkanTexture
{
    VkImage                 image;
    VkDeviceMemory          memory;
    VkImageView             view;
    VkDescriptorSet         desc;
    VkSampler               sampler;
    VkDeviceSize            size;
    uint32_t                width;
    uint32_t                height;
} VulkanTexture;

typedef struct VulkanContext_
{
    VkDevice            device;
    VkQueue             queue;
    VkCommandPool       pool;

    VkPhysicalDeviceMemoryProperties    memProps;
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
static VkCommandBuffer ext_vkQuickBufferBegin(
        VulkanContext* vulkan
);
static void ext_vkQuickBufferEnd(
        VulkanContext* vulkan,
        VkCommandBuffer buffer
);
static void ext_vkImageLayout(
        VulkanContext* vulkan,
        VkImage image,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout
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
