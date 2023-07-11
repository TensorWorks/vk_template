uint32_t cimgui_vertShaderSpv[];
uint32_t cimgui_fragShaderSpv[];

#ifdef GLFW_RESIZE_NESW_CURSOR
    #define GLFW_HAS_NEW_CURSORS (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 + GLFW_VERSION_REVISION >= 3400)
#else
    #define GLFW_HAS_NEW_CURSORS (0)
#endif

static int
ext_init(GlobalStorage* g)
{
    g->cimgui.context = igCreateContext(0);
    if (!g->cimgui.context) {
        fprintf(stderr, "Failed to create Imgui Context");
        return 1;
    }

    ImGuiIO* io = igGetIO();

    io->BackendPlatformName = "custom_impl_vulkan_cimgui";
    io->BackendRendererName = "custom_impl_vulkan_cimgui";
    io->BackendRendererUserData = g; /* Need access to both vulkan and cimgui */
    io->BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io->SetClipboardTextFn = ext_cimguiSetClipboard;
    io->GetClipboardTextFn = ext_cimguiGetClipboard;
    io->ClipboardUserData = 0;
    /* TODO Cursor position callback */

    /*
     * Cursor mapping
     */

    g->cimgui.cursorMap[ImGuiMouseCursor_Arrow]       = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g->cimgui.cursorMap[ImGuiMouseCursor_TextInput]   = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    g->cimgui.cursorMap[ImGuiMouseCursor_Hand]        = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    g->cimgui.cursorMap[ImGuiMouseCursor_ResizeEW]    = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    g->cimgui.cursorMap[ImGuiMouseCursor_ResizeNS]    = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);

    #if GLFW_HAS_NEW_CURSORS
        g->cimgui.cursorMap[ImGuiMouseCursor_ResizeAll]   = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
        g->cimgui.cursorMap[ImGuiMouseCursor_ResizeNESW]  = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
        g->cimgui.cursorMap[ImGuiMouseCursor_ResizeNWSE]  = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
        g->cimgui.cursorMap[ImGuiMouseCursor_NotAllowed]  = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
    #else
        g->cimgui.cursorMap[ImGuiMouseCursor_ResizeAll]   = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
        g->cimgui.cursorMap[ImGuiMouseCursor_ResizeNESW]  = g->cimgui.cursorMap[ImGuiMouseCursor_ResizeAll];
        g->cimgui.cursorMap[ImGuiMouseCursor_ResizeNWSE]  = g->cimgui.cursorMap[ImGuiMouseCursor_ResizeAll];
        g->cimgui.cursorMap[ImGuiMouseCursor_NotAllowed]  = g->cimgui.cursorMap[ImGuiMouseCursor_Arrow];
    #endif

    /*
     * Key mapping
     */

    g->cimgui.glfwKey[  0] = GLFW_KEY_APOSTROPHE;
    g->cimgui.glfwKey[  1] = GLFW_KEY_COMMA;
    g->cimgui.glfwKey[  2] = GLFW_KEY_MINUS;
    g->cimgui.glfwKey[  3] = GLFW_KEY_PERIOD;
    g->cimgui.glfwKey[  4] = GLFW_KEY_SLASH;
    g->cimgui.glfwKey[  5] = GLFW_KEY_0;
    g->cimgui.glfwKey[  6] = GLFW_KEY_1;
    g->cimgui.glfwKey[  7] = GLFW_KEY_2;
    g->cimgui.glfwKey[  8] = GLFW_KEY_3;
    g->cimgui.glfwKey[  9] = GLFW_KEY_4;
    g->cimgui.glfwKey[ 10] = GLFW_KEY_5;
    g->cimgui.glfwKey[ 11] = GLFW_KEY_6;
    g->cimgui.glfwKey[ 12] = GLFW_KEY_7;
    g->cimgui.glfwKey[ 13] = GLFW_KEY_8;
    g->cimgui.glfwKey[ 14] = GLFW_KEY_9;
    g->cimgui.glfwKey[ 15] = GLFW_KEY_SEMICOLON;
    g->cimgui.glfwKey[ 16] = GLFW_KEY_EQUAL;
    g->cimgui.glfwKey[ 17] = GLFW_KEY_A;
    g->cimgui.glfwKey[ 18] = GLFW_KEY_B;
    g->cimgui.glfwKey[ 19] = GLFW_KEY_C;
    g->cimgui.glfwKey[ 20] = GLFW_KEY_D;
    g->cimgui.glfwKey[ 21] = GLFW_KEY_E;
    g->cimgui.glfwKey[ 22] = GLFW_KEY_F;
    g->cimgui.glfwKey[ 23] = GLFW_KEY_G;
    g->cimgui.glfwKey[ 24] = GLFW_KEY_H;
    g->cimgui.glfwKey[ 25] = GLFW_KEY_I;
    g->cimgui.glfwKey[ 26] = GLFW_KEY_J;
    g->cimgui.glfwKey[ 27] = GLFW_KEY_K;
    g->cimgui.glfwKey[ 28] = GLFW_KEY_L;
    g->cimgui.glfwKey[ 29] = GLFW_KEY_M;
    g->cimgui.glfwKey[ 30] = GLFW_KEY_N;
    g->cimgui.glfwKey[ 31] = GLFW_KEY_O;
    g->cimgui.glfwKey[ 32] = GLFW_KEY_P;
    g->cimgui.glfwKey[ 33] = GLFW_KEY_Q;
    g->cimgui.glfwKey[ 34] = GLFW_KEY_R;
    g->cimgui.glfwKey[ 35] = GLFW_KEY_S;
    g->cimgui.glfwKey[ 36] = GLFW_KEY_T;
    g->cimgui.glfwKey[ 37] = GLFW_KEY_U;
    g->cimgui.glfwKey[ 38] = GLFW_KEY_V;
    g->cimgui.glfwKey[ 39] = GLFW_KEY_W;
    g->cimgui.glfwKey[ 40] = GLFW_KEY_X;
    g->cimgui.glfwKey[ 41] = GLFW_KEY_Y;
    g->cimgui.glfwKey[ 42] = GLFW_KEY_Z;
    g->cimgui.glfwKey[ 43] = GLFW_KEY_LEFT_BRACKET;
    g->cimgui.glfwKey[ 44] = GLFW_KEY_BACKSLASH;
    g->cimgui.glfwKey[ 45] = GLFW_KEY_RIGHT_BRACKET;
    g->cimgui.glfwKey[ 46] = GLFW_KEY_GRAVE_ACCENT;
    g->cimgui.glfwKey[ 47] = GLFW_KEY_SPACE;
    g->cimgui.glfwKey[ 48] = GLFW_KEY_ESCAPE;
    g->cimgui.glfwKey[ 49] = GLFW_KEY_ENTER;
    g->cimgui.glfwKey[ 50] = GLFW_KEY_TAB;
    g->cimgui.glfwKey[ 51] = GLFW_KEY_BACKSPACE;
    g->cimgui.glfwKey[ 52] = GLFW_KEY_INSERT;
    g->cimgui.glfwKey[ 53] = GLFW_KEY_DELETE;
    g->cimgui.glfwKey[ 54] = GLFW_KEY_RIGHT;
    g->cimgui.glfwKey[ 55] = GLFW_KEY_LEFT;
    g->cimgui.glfwKey[ 56] = GLFW_KEY_DOWN;
    g->cimgui.glfwKey[ 57] = GLFW_KEY_UP;
    g->cimgui.glfwKey[ 58] = GLFW_KEY_PAGE_UP;
    g->cimgui.glfwKey[ 59] = GLFW_KEY_PAGE_DOWN;
    g->cimgui.glfwKey[ 60] = GLFW_KEY_HOME;
    g->cimgui.glfwKey[ 61] = GLFW_KEY_END;
    g->cimgui.glfwKey[ 62] = GLFW_KEY_CAPS_LOCK;
    g->cimgui.glfwKey[ 63] = GLFW_KEY_SCROLL_LOCK;
    g->cimgui.glfwKey[ 64] = GLFW_KEY_NUM_LOCK;
    g->cimgui.glfwKey[ 65] = GLFW_KEY_PRINT_SCREEN;
    g->cimgui.glfwKey[ 66] = GLFW_KEY_PAUSE;
    g->cimgui.glfwKey[ 67] = GLFW_KEY_F1;
    g->cimgui.glfwKey[ 68] = GLFW_KEY_F2;
    g->cimgui.glfwKey[ 69] = GLFW_KEY_F3;
    g->cimgui.glfwKey[ 70] = GLFW_KEY_F4;
    g->cimgui.glfwKey[ 71] = GLFW_KEY_F5;
    g->cimgui.glfwKey[ 72] = GLFW_KEY_F6;
    g->cimgui.glfwKey[ 73] = GLFW_KEY_F7;
    g->cimgui.glfwKey[ 74] = GLFW_KEY_F8;
    g->cimgui.glfwKey[ 75] = GLFW_KEY_F9;
    g->cimgui.glfwKey[ 76] = GLFW_KEY_F10;
    g->cimgui.glfwKey[ 77] = GLFW_KEY_F11;
    g->cimgui.glfwKey[ 78] = GLFW_KEY_F12;
    g->cimgui.glfwKey[ 79] = GLFW_KEY_LEFT_SHIFT;
    g->cimgui.glfwKey[ 80] = GLFW_KEY_LEFT_CONTROL;
    g->cimgui.glfwKey[ 81] = GLFW_KEY_LEFT_ALT;
    g->cimgui.glfwKey[ 82] = GLFW_KEY_LEFT_SUPER;
    g->cimgui.glfwKey[ 83] = GLFW_KEY_RIGHT_SHIFT;
    g->cimgui.glfwKey[ 84] = GLFW_KEY_RIGHT_CONTROL;
    g->cimgui.glfwKey[ 85] = GLFW_KEY_RIGHT_ALT;
    g->cimgui.glfwKey[ 86] = GLFW_KEY_RIGHT_SUPER;
    g->cimgui.glfwKey[ 87] = GLFW_KEY_MENU;
    g->cimgui.glfwKey[ 88] = GLFW_KEY_KP_0;
    g->cimgui.glfwKey[ 89] = GLFW_KEY_KP_1;
    g->cimgui.glfwKey[ 90] = GLFW_KEY_KP_2;
    g->cimgui.glfwKey[ 91] = GLFW_KEY_KP_3;
    g->cimgui.glfwKey[ 92] = GLFW_KEY_KP_4;
    g->cimgui.glfwKey[ 93] = GLFW_KEY_KP_5;
    g->cimgui.glfwKey[ 94] = GLFW_KEY_KP_6;
    g->cimgui.glfwKey[ 95] = GLFW_KEY_KP_7;
    g->cimgui.glfwKey[ 96] = GLFW_KEY_KP_8;
    g->cimgui.glfwKey[ 97] = GLFW_KEY_KP_9;
    g->cimgui.glfwKey[ 98] = GLFW_KEY_KP_DECIMAL;
    g->cimgui.glfwKey[ 99] = GLFW_KEY_KP_DIVIDE;
    g->cimgui.glfwKey[100] = GLFW_KEY_KP_MULTIPLY;
    g->cimgui.glfwKey[101] = GLFW_KEY_KP_SUBTRACT;
    g->cimgui.glfwKey[102] = GLFW_KEY_KP_ADD;
    g->cimgui.glfwKey[103] = GLFW_KEY_KP_ENTER;
    g->cimgui.glfwKey[104] = GLFW_KEY_KP_EQUAL;

    g->cimgui.imguiKey[  0] = ImGuiKey_Apostrophe;
    g->cimgui.imguiKey[  1] = ImGuiKey_Comma;
    g->cimgui.imguiKey[  2] = ImGuiKey_Minus;
    g->cimgui.imguiKey[  3] = ImGuiKey_Period;
    g->cimgui.imguiKey[  4] = ImGuiKey_Slash;
    g->cimgui.imguiKey[  5] = ImGuiKey_0;
    g->cimgui.imguiKey[  6] = ImGuiKey_1;
    g->cimgui.imguiKey[  7] = ImGuiKey_2;
    g->cimgui.imguiKey[  8] = ImGuiKey_3;
    g->cimgui.imguiKey[  9] = ImGuiKey_4;
    g->cimgui.imguiKey[ 10] = ImGuiKey_5;
    g->cimgui.imguiKey[ 11] = ImGuiKey_6;
    g->cimgui.imguiKey[ 12] = ImGuiKey_7;
    g->cimgui.imguiKey[ 13] = ImGuiKey_8;
    g->cimgui.imguiKey[ 14] = ImGuiKey_9;
    g->cimgui.imguiKey[ 15] = ImGuiKey_Semicolon;
    g->cimgui.imguiKey[ 16] = ImGuiKey_Equal;
    g->cimgui.imguiKey[ 17] = ImGuiKey_A;
    g->cimgui.imguiKey[ 18] = ImGuiKey_B;
    g->cimgui.imguiKey[ 19] = ImGuiKey_C;
    g->cimgui.imguiKey[ 20] = ImGuiKey_D;
    g->cimgui.imguiKey[ 21] = ImGuiKey_E;
    g->cimgui.imguiKey[ 22] = ImGuiKey_F;
    g->cimgui.imguiKey[ 23] = ImGuiKey_G;
    g->cimgui.imguiKey[ 24] = ImGuiKey_H;
    g->cimgui.imguiKey[ 25] = ImGuiKey_I;
    g->cimgui.imguiKey[ 26] = ImGuiKey_J;
    g->cimgui.imguiKey[ 27] = ImGuiKey_K;
    g->cimgui.imguiKey[ 28] = ImGuiKey_L;
    g->cimgui.imguiKey[ 29] = ImGuiKey_M;
    g->cimgui.imguiKey[ 30] = ImGuiKey_N;
    g->cimgui.imguiKey[ 31] = ImGuiKey_O;
    g->cimgui.imguiKey[ 32] = ImGuiKey_P;
    g->cimgui.imguiKey[ 33] = ImGuiKey_Q;
    g->cimgui.imguiKey[ 34] = ImGuiKey_R;
    g->cimgui.imguiKey[ 35] = ImGuiKey_S;
    g->cimgui.imguiKey[ 36] = ImGuiKey_T;
    g->cimgui.imguiKey[ 37] = ImGuiKey_U;
    g->cimgui.imguiKey[ 38] = ImGuiKey_V;
    g->cimgui.imguiKey[ 39] = ImGuiKey_W;
    g->cimgui.imguiKey[ 40] = ImGuiKey_X;
    g->cimgui.imguiKey[ 41] = ImGuiKey_Y;
    g->cimgui.imguiKey[ 42] = ImGuiKey_Z;
    g->cimgui.imguiKey[ 43] = ImGuiKey_LeftBracket;
    g->cimgui.imguiKey[ 44] = ImGuiKey_Backslash;
    g->cimgui.imguiKey[ 45] = ImGuiKey_RightBracket;
    g->cimgui.imguiKey[ 46] = ImGuiKey_GraveAccent;
    g->cimgui.imguiKey[ 47] = ImGuiKey_Space;
    g->cimgui.imguiKey[ 48] = ImGuiKey_Escape;
    g->cimgui.imguiKey[ 49] = ImGuiKey_Enter;
    g->cimgui.imguiKey[ 50] = ImGuiKey_Tab;
    g->cimgui.imguiKey[ 51] = ImGuiKey_Backspace;
    g->cimgui.imguiKey[ 52] = ImGuiKey_Insert;
    g->cimgui.imguiKey[ 53] = ImGuiKey_Delete;
    g->cimgui.imguiKey[ 54] = ImGuiKey_RightArrow;
    g->cimgui.imguiKey[ 55] = ImGuiKey_LeftArrow;
    g->cimgui.imguiKey[ 56] = ImGuiKey_DownArrow;
    g->cimgui.imguiKey[ 57] = ImGuiKey_UpArrow;
    g->cimgui.imguiKey[ 58] = ImGuiKey_PageUp;
    g->cimgui.imguiKey[ 59] = ImGuiKey_PageDown;
    g->cimgui.imguiKey[ 60] = ImGuiKey_Home;
    g->cimgui.imguiKey[ 61] = ImGuiKey_End;
    g->cimgui.imguiKey[ 62] = ImGuiKey_CapsLock;
    g->cimgui.imguiKey[ 63] = ImGuiKey_ScrollLock;
    g->cimgui.imguiKey[ 64] = ImGuiKey_NumLock;
    g->cimgui.imguiKey[ 65] = ImGuiKey_PrintScreen;
    g->cimgui.imguiKey[ 66] = ImGuiKey_Pause;
    g->cimgui.imguiKey[ 67] = ImGuiKey_F1;
    g->cimgui.imguiKey[ 68] = ImGuiKey_F2;
    g->cimgui.imguiKey[ 69] = ImGuiKey_F3;
    g->cimgui.imguiKey[ 70] = ImGuiKey_F4;
    g->cimgui.imguiKey[ 71] = ImGuiKey_F5;
    g->cimgui.imguiKey[ 72] = ImGuiKey_F6;
    g->cimgui.imguiKey[ 73] = ImGuiKey_F7;
    g->cimgui.imguiKey[ 74] = ImGuiKey_F8;
    g->cimgui.imguiKey[ 75] = ImGuiKey_F9;
    g->cimgui.imguiKey[ 76] = ImGuiKey_F10;
    g->cimgui.imguiKey[ 77] = ImGuiKey_F11;
    g->cimgui.imguiKey[ 78] = ImGuiKey_F12;
    g->cimgui.imguiKey[ 79] = ImGuiKey_LeftShift;
    g->cimgui.imguiKey[ 80] = ImGuiKey_LeftCtrl;
    g->cimgui.imguiKey[ 81] = ImGuiKey_LeftAlt;
    g->cimgui.imguiKey[ 82] = ImGuiKey_LeftSuper;
    g->cimgui.imguiKey[ 83] = ImGuiKey_RightShift;
    g->cimgui.imguiKey[ 84] = ImGuiKey_RightCtrl;
    g->cimgui.imguiKey[ 85] = ImGuiKey_RightAlt;
    g->cimgui.imguiKey[ 86] = ImGuiKey_RightSuper;
    g->cimgui.imguiKey[ 87] = ImGuiKey_Menu;
    g->cimgui.imguiKey[ 88] = ImGuiKey_Keypad0;
    g->cimgui.imguiKey[ 89] = ImGuiKey_Keypad1;
    g->cimgui.imguiKey[ 90] = ImGuiKey_Keypad2;
    g->cimgui.imguiKey[ 91] = ImGuiKey_Keypad3;
    g->cimgui.imguiKey[ 92] = ImGuiKey_Keypad4;
    g->cimgui.imguiKey[ 93] = ImGuiKey_Keypad5;
    g->cimgui.imguiKey[ 94] = ImGuiKey_Keypad6;
    g->cimgui.imguiKey[ 95] = ImGuiKey_Keypad7;
    g->cimgui.imguiKey[ 96] = ImGuiKey_Keypad8;
    g->cimgui.imguiKey[ 97] = ImGuiKey_Keypad9;
    g->cimgui.imguiKey[ 98] = ImGuiKey_KeypadDecimal;
    g->cimgui.imguiKey[ 99] = ImGuiKey_KeypadDivide;
    g->cimgui.imguiKey[100] = ImGuiKey_KeypadMultiply;
    g->cimgui.imguiKey[101] = ImGuiKey_KeypadSubtract;
    g->cimgui.imguiKey[102] = ImGuiKey_KeypadAdd;
    g->cimgui.imguiKey[103] = ImGuiKey_KeypadEnter;
    g->cimgui.imguiKey[104] = ImGuiKey_KeypadEqual;

    /* Font */

    unsigned char* pixels = 0;
    int width, height;
    ImFontAtlas_AddFontDefault(io->Fonts, 0);
    ImFontAtlas_GetTexDataAsRGBA32(io->Fonts, &pixels, &width, &height, 0);

    VkResult res = ext_vkCreateTexture(&g->vulkan, &g->cimgui.fontTexture, pixels, width, height);
    if (res != VK_SUCCESS) {
        return 2;
    }
    io->Fonts->TexID = &g->cimgui.fontTexture;

    /* Shaders */

    VkShaderModule vertShader, fragShader;

    VkShaderModuleCreateInfo shaderInfo = {0};
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = sizeof(cimgui_vertShaderSpv);
    shaderInfo.pCode = cimgui_vertShaderSpv;
    vk_result = vkCreateShaderModule(g->vulkan.device, &shaderInfo, 0, &vertShader);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateShaderModule() failed for imgui vertex shader, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 3;
    }

    shaderInfo.codeSize = sizeof(cimgui_fragShaderSpv);
    shaderInfo.pCode = cimgui_fragShaderSpv;
    vk_result = vkCreateShaderModule(g->vulkan.device, &shaderInfo, 0, &fragShader);
    if (vk_result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateShaderModule() failed for imgui fragment shader, result code [%i]: %s\n",
                vk_result, string_VkResult(vk_result));
        return 4;
    }

    /* TODO Pipeline */
    VkPipelineShaderStageCreateInfo shaderStage[2] = {0};
    shaderStage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStage[0].module = vertShader;
    shaderStage[0].pName = "main";
    shaderStage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStage[1].module = fragShader;
    shaderStage[1].pName = "main";

    VkVertexInputBindingDescription binding = {0};
    binding.stride = sizeof(ImDrawVert);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributes[3] = {0};
    attributes[0].location = 0;
    attributes[0].binding = binding.binding;
    attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributes[0].offset = offsetof(ImDrawVert, pos);
    attributes[1].location = 1;
    attributes[1].binding = binding.binding;
    attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributes[1].offset = offsetof(ImDrawVert, uv);
    attributes[2].location = 2;
    attributes[2].binding = binding.binding;
    attributes[2].format = VK_FORMAT_R8G8B8A8_UNORM;
    attributes[2].offset = offsetof(ImDrawVert, col);

    VkPipelineVertexInputStateCreateInfo vertexInfo = {0};
    vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInfo.vertexBindingDescriptionCount = 1;
    vertexInfo.pVertexBindingDescriptions = &binding;
    vertexInfo.vertexAttributeDescriptionCount = 3;
    vertexInfo.pVertexAttributeDescriptions = attributes;

    VkPipelineInputAssemblyStateCreateInfo assembly = {0};
    assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewport = {0};
    viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport.viewportCount = 1;
    viewport.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo raster = {0};
    raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.cullMode = VK_CULL_MODE_NONE;
    raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo msaa = {0};
    msaa.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    msaa.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color = {0};
    color.blendEnable = VK_TRUE;
    color.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color.colorBlendOp = VK_BLEND_OP_ADD;
    color.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color.alphaBlendOp = VK_BLEND_OP_ADD;
    color.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                         | VK_COLOR_COMPONENT_G_BIT
                         | VK_COLOR_COMPONENT_B_BIT
                         | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineDepthStencilStateCreateInfo depth = {0};
    depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    VkPipelineColorBlendStateCreateInfo blend = {0};
    blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend.attachmentCount = 1;
    blend.pAttachments = &color;

    VkDynamicStates dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamics = {0};
    dynamics.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamics.dynamicStateCount = sizeof(dynamicStates) / sizeof(*dynamicStates);
    dynamics.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = 2;
    createInfo.pStages = shaderStage;
    createInfo.pVertexInputState = &vertexInfo;
    createInfo.pInputAssemblyState = &assembly;
    createInfo.pViewportState = &viewport;
    createInfo.pRasterizationState = &raster;
    createInfo.pMultisampleState = &msaa;
    createInfo.pDepthStencilState = &depth;
    createInfo.pColorBlendState = &blend;
    createInfo.pDynamicState = &dynamics;
    /* TODO Do we need a custom layout? */
    createInfo.layout = g->vulkan.layout;
    /* TODO Need custom renderpass */
    /* TODO Do we also need custom semaphores? */

    /* Cleanup */

    vkDestroyShaderModule(vertShader);
    vkDestroyShaderModule(fragShader);

    return 0;
}

static void
ext_destroy(GlobalStorage* g)
{
    ext_vkDestroyTexture(&g->vulkan, &g->cimgui.fontTexture);
    glfwDestroyCursor(g->cimgui.cursorMap[ImGuiMouseCursor_Arrow]);
    glfwDestroyCursor(g->cimgui.cursorMap[ImGuiMouseCursor_TextInput]);
    glfwDestroyCursor(g->cimgui.cursorMap[ImGuiMouseCursor_Hand]);
    glfwDestroyCursor(g->cimgui.cursorMap[ImGuiMouseCursor_ResizeAll]);
    glfwDestroyCursor(g->cimgui.cursorMap[ImGuiMouseCursor_ResizeEW]);
    glfwDestroyCursor(g->cimgui.cursorMap[ImGuiMouseCursor_ResizeNS]);

    #if GLFW_HAS_NEW_CURSORS
        glfwDestroyCursor(g->cimgui.cursorMap[ImGuiMouseCursor_ResizeNESW]);
        glfwDestroyCursor(g->cimgui.cursorMap[ImGuiMouseCursor_ResizeNWSE]);
        glfwDestroyCursor(g->cimgui.cursorMap[ImGuiMouseCursor_NotAllowed]);
    #endif

    igDestroyContext(g->cimgui.context);
}

/*
 * Vulkan
 */

static int
ext_vkFindMemoryType(VkPhysicalDeviceMemoryProperties* gpuMemoryProperties,
                     uint32_t memoryTypeBits,
                     VkMemoryPropertyFlags properties)
{
    for (int i = 0; i < gpuMemoryProperties->memoryTypeCount; i++) {
        if (memoryTypeBits & (1 << i)
            && (gpuMemoryProperties->memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    return -1;
}

static VkResult
ext_vkCreateBuffer(VulkanContext* vulkan, void* data, VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* memory)
{
    VkResult res;

    VkBufferCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = size;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    res = vkCreateBuffer(vulkan->device, &createInfo, 0, buffer);
    if (res != VK_SUCCESS) {
        fprintf(stderr, "vkCreateBuffer(), result code [%i]: %s\n", res, string_VkResult(res));
        return res;
    }

    VkMemoryRequirements reqs = {0};
    vkGetBufferMemoryRequirements(vulkan->device, *buffer, &reqs);

    int memoryTypeIndex = ext_vkFindMemoryType(&vulkan->memProps, reqs.memoryTypeBits, properties);
    if (memoryTypeIndex < 0) {
        fprintf(stderr, "Could not find an appropriate memory type\n");
        return VK_ERROR_UNKNOWN;
    }

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = reqs.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    res = vkAllocateMemory(vulkan->device, &allocInfo, 0, memory);
    if (res != VK_SUCCESS) {
        fprintf(stderr, "vkAllocateMemory() failed, result code [%i]: %s\n", res, string_VkResult(res));
        return res;
    }

    void* mapping;
    res = vkMapMemory(vulkan->device, *memory, 0, size, 0, &mapping);
    if (res != VK_SUCCESS) {
        fprintf(stderr, "vkMapMemory() failed, result code [%i]: %s\n", res, string_VkResult(res));
        return res;
    }

    memcpy(mapping, data, size);
    vkUnmapMemory(vulkan->device, *memory);
    res = vkBindBufferMemory(vulkan->device, *buffer, *memory, 0);
    if (res != VK_SUCCESS) {
        fprintf(stderr, "vkBindBufferMemory() failed, result code [%i]: %s\n", res, string_VkResult(res));
        return res;
    }

    return res;
}

static VkCommandBuffer
ext_vkQuickCommandBegin(VulkanContext* vulkan)
{
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vulkan->pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer buffer;
    vkAllocateCommandBuffers(vulkan->device, &allocInfo, &buffer);

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(buffer, &beginInfo);

    return buffer;
}

static void
ext_vkQuickCommandEnd(VulkanContext* vulkan, VkCommandBuffer buffer)
{
    vkEndCommandBuffer(buffer);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;
    vkQueueSubmit(vulkan->queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkan->queue);
    vkFreeCommandBuffers(vulkan->device, vulkan->pool, 1, &buffer);
}

static VkResult
ext_vkImageLayout(VulkanContext* vulkan, VkImage image, VkFormat format,
                    VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer cmd = ext_vkQuickCommandBegin(vulkan);

    VkImageMemoryBarrier barrier = {0};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage, dstStage;
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
        && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
               && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;


    } else {
        fprintf(stderr, "vkImageLayout() unsupported (or unimplemented) layout change, %i to %i\n",
                oldLayout, newLayout);
        return VK_ERROR_UNKNOWN;
    }

    vkCmdPipelineBarrier(
        cmd,
        srcStage, dstStage, /* Pipeline stages, before/after */
        0, /* Region-specific operation bit */
        0, 0, /* Memory Barrier */
        0, 0, /* Buffer Barrier */
        1, &barrier /* Image Barrier */
    );

    ext_vkQuickCommandEnd(vulkan, cmd);
    return VK_SUCCESS;
}

static void
ext_vkCopyBufferToImage(VulkanContext* vulkan, VkBuffer from, VkImage to, uint32_t width, uint32_t height)
{
    VkCommandBuffer cmd = ext_vkQuickCommandBegin(vulkan);

    VkBufferImageCopy region = {0};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(cmd, from, to, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    ext_vkQuickCommandEnd(vulkan, cmd);
}

static VkResult
ext_vkCreateTexture(VulkanContext* vulkan, VulkanTexture* texture,
                    uint8_t* pixels, uint32_t width, uint32_t height)
{
    texture->width  = width;
    texture->height = height;
    texture->size   = width * height * 4;

    VkResult res;
    VkBuffer buffer;
    VkDeviceMemory memory;
    res = ext_vkCreateBuffer(vulkan, pixels, texture->size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            &buffer, &memory);
    if (res != VK_SUCCESS) {
        return res;
    }

    /* Actual image object */
    VkImageCreateInfo imageInfo = {0};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    res = vkCreateImage(vulkan->device, &imageInfo, 0, &texture->image);
    if (res != VK_SUCCESS) {
        fprintf(stderr, "vkCreateImage() failed for texture, result code [%i]: %s\n",
                res, string_VkResult(res));
        return res;
    }

    VkMemoryRequirements reqs;
    vkGetImageMemoryRequirements(vulkan->device, texture->image, &reqs);

    int imageMemType = ext_vkFindMemoryType(&vulkan->memProps, reqs.memoryTypeBits,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (imageMemType < 0) {
        fprintf(stderr, "Could not find an appropriate memory type for texture\n");
        return VK_ERROR_UNKNOWN;
    }

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = reqs.size;
    allocInfo.memoryTypeIndex = imageMemType;

    res = vkAllocateMemory(vulkan->device, &allocInfo, 0, &texture->memory);
    if (res != VK_SUCCESS) {
        fprintf(stderr, "vkAllocateMemory() failed for texture, result code [%i]: %s\n",
                res, string_VkResult(res));
        return res;
    }

    res = vkBindImageMemory(vulkan->device, texture->image, texture->memory, 0);
    if (res != VK_SUCCESS) {
        fprintf(stderr, "vkBindImageMemory() failed for texture, result code [%i]: %s\n",
                res, string_VkResult(res));
        return res;
    }

    res = ext_vkImageLayout(vulkan, texture->image, imageInfo.format,
                      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    if (res != VK_SUCCESS)
        return res;

    ext_vkCopyBufferToImage(vulkan, buffer, texture->image, width, height);

    res = ext_vkImageLayout(vulkan, texture->image, imageInfo.format,
                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    if (res != VK_SUCCESS)
        return res;

    vkDestroyBuffer(vulkan->device, buffer, 0);
    vkFreeMemory(vulkan->device, memory, 0);

    VkImageViewCreateInfo viewInfo = {0};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = imageInfo.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    res = vkCreateImageView(vulkan->device, &viewInfo, 0, &texture->view);
    if (res != VK_SUCCESS)
        fprintf(stderr, "Failed to create image view for texture\n");
        return res;

    return VK_SUCCESS;
}

static void
ext_vkDestroyTexture(VulkanContext* vulkan, VulkanTexture* texture)
{
    vkDestroyImageView(vulkan->device, texture->view, 0);
    vkDestroyImage(vulkan->device, texture->image, 0);
    vkFreeMemory(vulkan->device, texture->memory, 0);
}

/*
 * CImgui
 */

static const char*
ext_cimguiGetClipboard(void* context)
{
    /* TODO */
    printf("Warning, %s not yet implemented", __func__);
    return "";
}

static void
ext_cimguiSetClipboard(void* context, const char* text)
{
    /* TODO */
    printf("Warning, %s not yet implemented", __func__);
}

/* Shades sourced from imgui/backends/imgui_impl_vulkan.cpp */

// glsl_shader.vert, compiled with:
// # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
/*
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

void main()
{
    Out.Color = aColor;
    Out.UV = aUV;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
*/
uint32_t cimgui_vertShaderSpv[] =
{
    0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
    0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
    0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
    0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
    0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
    0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
    0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
    0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
    0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
    0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
    0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
    0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
    0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
    0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
    0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
    0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
    0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
    0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
    0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
    0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
    0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
    0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
    0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
    0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
    0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
    0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
    0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
    0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
    0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
    0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
    0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
    0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
    0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
    0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
    0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
    0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
    0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
    0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
    0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
    0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
    0x0000002d,0x0000002c,0x000100fd,0x00010038
};

// glsl_shader.frag, compiled with:
// # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag
/*
#version 450 core
layout(location = 0) out vec4 fColor;
layout(set=0, binding=0) uniform sampler2D sTexture;
layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
void main()
{
    fColor = In.Color * texture(sTexture, In.UV.st);
}
*/
uint32_t cimgui_fragShaderSpv[] =
{
    0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
    0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
    0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
    0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
    0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
    0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
    0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
    0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
    0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
    0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
    0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
    0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
    0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
    0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
    0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
    0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
    0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
    0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
    0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
    0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
    0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
    0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
    0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
    0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
    0x00010038
};

