/* These are used for allocating buffers for vertexes and so on.
 * Represents the multiple of instances, not the actual size */
const VkDeviceSize IG_VERTEX_SIZE = 20; // Pos (2f), UV (2f), Color (4u)
const VkDeviceSize IG_INDEX_SIZE = 2;
#define SMALL_PAGE 1024
#define BIG_PAGE (1024 * 1024)

static int ext_init(GlobalStorage* g);
static void ext_destroy(GlobalStorage* g);

/*
 * Vulkan
 * Enable validation by setting the following environment variable:
 *  export VK_LOADER_LAYERS_ENABLE=*api_dump,*validation
 */

typedef struct VulkanTexture
{
    /* TODO Note that imgui/backends/imgui_impl_vulkan.cpp uses the VkDescriptorSet itself as its texture
     * handle within imgui. That would most certainly be the way to do it to avoid issues later */
    VkImage                 image;
    VkDeviceMemory          memory;
    VkImageView             view;
    VkImageLayout           layout;

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
    VkPipelineLayout    layout;

    VkRenderPass        renderpass; /* FIXME Not necessary with Dynamic Rendering */
    VkSampler           nearestSampler;
    VkSampler           linearSampler;

    VkPhysicalDeviceMemoryProperties    memProps;
    VkPhysicalDeviceProperties          devProps;
    VkPhysicalDevice                    gpu;
    VkDescriptorPool                    descriptorPool;
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

/* Note that this will not resize the buffer, allocate one big enough for your needs at the outset */
static VkResult ext_vkUpdateBuffer(
        VulkanContext* vulkan,
        void* data,
        VkDeviceSize size, // FIXME: This is the _current_ size which must match up.
        VkBuffer* buffer,
        VkDeviceMemory* memory
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
int                 glfwKey     [CIMGUI_KEYMAP_COUNT];
ImGuiKey            imguiKey    [CIMGUI_KEYMAP_COUNT];

typedef struct {
    VkDeviceSize index;
    VkDeviceSize indexCount;
    VkRect2D scissor; // FIXME Scissor needs to be applied per triangle
} CImguiDrawCall;

typedef struct CImgui_ {
    ImGuiContext*       context;

    ImGuiMouseCursor    cursor;
    GLFWcursor*         cursorMap   [ImGuiMouseCursor_COUNT];

    /* Vulkan Integration */
    VulkanTexture               fontTexture;
    VkSampler                   fontSampler;
    VkDescriptorPool            fontDescriptorPool;
    VkPipeline                  pipeline;
    VkPipelineLayout            pipelineLayout;
    struct {
        VkBuffer        buffer;
        VkDeviceMemory  memory;
        VkDeviceSize    size;
        uint8_t*        writeBuffer;
    } vertex;
    struct {
        VkBuffer        buffer;
        VkDeviceMemory  memory;
        VkDeviceSize    size;
        uint16_t*       writeBuffer;
    } index;
    struct {
        VkBuffer        buffer;
        VkDeviceMemory  memory;
        VkDeviceSize    size;
    } uniform;
    CImguiDrawCall*     calls;
    size_t              callsSize;
} CImgui;

static const char* ext_cimguiGetClipboard(void* context);
static void ext_cimguiSetClipboard(void* context, const char* text);

static void ext_cimguiKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void ext_cimguiCharacterCallback(GLFWwindow* window, unsigned codepoint);
static void ext_cimguiMousePositionCallback(GLFWwindow* window, double x, double y);
static void ext_cimguiMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
static void ext_cimguiMouseWheelCallback(GLFWwindow* window, double x, double y);
