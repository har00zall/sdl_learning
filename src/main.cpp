#define TYNYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <tiny_gltf.h>
#include <glm/gtc/matrix_transform.hpp>
#include "main.hpp"

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    /* Create the window */
    if (!app.CreateWindow())
        return SDL_APP_FAILURE;

    app.CreateRenderer3D();
    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_KEY_DOWN ||
        event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS; /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    app.Render3D();

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    // release buffers
    SDL_ReleaseGPUBuffer(app.gpuDevice, app.gpuVertexBuffer);

    // release the pipeline
    SDL_ReleaseGPUGraphicsPipeline(app.gpuDevice, app.gpuGraphicsPipeline);

    // destroy the GPU device
    SDL_DestroyGPUDevice(app.gpuDevice);

    // destroy the window
    SDL_DestroyWindow(app.window);
}

SDL_Window *App::CreateWindow()
{
    app.window = SDL_CreateWindow(
        "Sandbox",
        1280, 720,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    if (!app.window)
    {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return NULL;
    }

    return app.window;
}

SDL_GPUShader *CreateShader(const char *shaderFilePath, SDL_GPUShaderFormat shaderFormat, SDL_GPUShaderStage shaderStage,
                            int num_sampler = 0, int num_storage_buffers = 0, int num_storage_textures = 0, int num_uniform_buffers = 0)
{
    size_t shaderCodeSize;
    void *shaderCode = SDL_LoadFile(shaderFilePath, &shaderCodeSize);
    SDL_GPUShaderCreateInfo shaderCreateInfo{};
    shaderCreateInfo.code = (Uint8 *)shaderCode;
    shaderCreateInfo.code_size = shaderCodeSize;
    shaderCreateInfo.entrypoint = "main";
    shaderCreateInfo.format = shaderFormat;
    shaderCreateInfo.num_samplers = num_sampler;
    shaderCreateInfo.num_storage_buffers = num_storage_buffers;
    shaderCreateInfo.num_storage_textures = num_storage_textures;
    shaderCreateInfo.num_uniform_buffers = num_uniform_buffers;
    shaderCreateInfo.stage = shaderStage;

    SDL_GPUShader *shader = SDL_CreateGPUShader(app.gpuDevice, &shaderCreateInfo);
    SDL_free(shaderCode);

    return shader;
}

int App::CreateRenderer3D()
{
    // create GPU device
    app.gpuDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    SDL_ClaimWindowForGPUDevice(app.gpuDevice, app.window);
    int w, h;
    SDL_GetWindowSizeInPixels(app.window, &w, &h);

    SDL_GPUShader *vertexShader = CreateShader("shaders/base.vert.spv", SDL_GPU_SHADERFORMAT_SPIRV, SDL_GPU_SHADERSTAGE_VERTEX, 0, 0, 0, 1);
    SDL_GPUShader *fragmentShader = CreateShader("shaders/base.frag.spv", SDL_GPU_SHADERFORMAT_SPIRV, SDL_GPU_SHADERSTAGE_FRAGMENT);

    // create the graphics pipeline
    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.vertex_shader = vertexShader;
    pipelineInfo.fragment_shader = fragmentShader;
    pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    // create depth texture
    SDL_GPUTextureCreateInfo depthTextureCreateInfo{};
    depthTextureCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
    depthTextureCreateInfo.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    depthTextureCreateInfo.width = w;
    depthTextureCreateInfo.height = h;
    depthTextureCreateInfo.layer_count_or_depth = 1;
    depthTextureCreateInfo.num_levels = 1;
    depthTextureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    SDL_GPUTexture *depthTexture = SDL_CreateGPUTexture(app.gpuDevice, &depthTextureCreateInfo);

    pipelineInfo.depth_stencil_state.enable_depth_test = true;
    pipelineInfo.depth_stencil_state.enable_depth_write = true;
    pipelineInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
    pipelineInfo.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    pipelineInfo.target_info.has_depth_stencil_target = true;

    // describe the vertex buffers
    SDL_GPUVertexBufferDescription vertexBufferDesctiptions[1];
    vertexBufferDesctiptions[0].slot = 0;
    vertexBufferDesctiptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertexBufferDesctiptions[0].instance_step_rate = 0;
    vertexBufferDesctiptions[0].pitch = sizeof(Vertex);

    pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
    pipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDesctiptions;

    // describe the vertex attribute
    SDL_GPUVertexAttribute vertexAttributes[2];

    // a_position
    vertexAttributes[0].buffer_slot = 0;
    vertexAttributes[0].location = 0;
    vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    vertexAttributes[0].offset = 0;

    // a_color
    vertexAttributes[1].buffer_slot = 0;
    vertexAttributes[1].location = 1;
    vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    vertexAttributes[1].offset = sizeof(float) * 3;

    pipelineInfo.vertex_input_state.num_vertex_attributes = 2;
    pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;

    // describe the color target
    SDL_GPUColorTargetDescription colorTargetDescriptions[1];
    colorTargetDescriptions[0] = {};
    colorTargetDescriptions[0].blend_state.enable_blend = true;
    colorTargetDescriptions[0].blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
    colorTargetDescriptions[0].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
    colorTargetDescriptions[0].blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    colorTargetDescriptions[0].blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    colorTargetDescriptions[0].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    colorTargetDescriptions[0].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(app.gpuDevice, app.window);

    pipelineInfo.target_info.num_color_targets = 1;
    pipelineInfo.target_info.color_target_descriptions = colorTargetDescriptions;

    // create the pipeline
    app.gpuGraphicsPipeline = SDL_CreateGPUGraphicsPipeline(app.gpuDevice, &pipelineInfo);

    // we don't need to store the shaders after creating the pipeline
    SDL_ReleaseGPUShader(app.gpuDevice, vertexShader);
    SDL_ReleaseGPUShader(app.gpuDevice, fragmentShader);

    // load geometry
    std::vector<Vertex> vertices;
    std::vector<Uint32> indices;
    app.LoadGLTF("assets/cube.glb", vertices, indices);
    app.indexCount = indices.size();

    // create the vertex buffer
    SDL_GPUBufferCreateInfo vertexBufferInfo{};
    vertexBufferInfo.size = (Uint32)(vertices.size() * sizeof(Vertex));
    vertexBufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    app.gpuVertexBuffer = SDL_CreateGPUBuffer(app.gpuDevice, &vertexBufferInfo);

    // create the index buffer
    SDL_GPUBufferCreateInfo indexBufferInfo{};
    indexBufferInfo.size = (Uint32)(indices.size() * sizeof(Uint32));
    indexBufferInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    app.gpuIndexBuffer = SDL_CreateGPUBuffer(app.gpuDevice, &indexBufferInfo);

    // create uniform buffer
    SDL_GPUBufferCreateInfo uniformBufferInfo{};
    uniformBufferInfo.size = sizeof(CameraData);
    uniformBufferInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
    app.gpuUniformBuffer = SDL_CreateGPUBuffer(app.gpuDevice, &uniformBufferInfo);

    // create a transfer buffer to upload to the vertex buffer
    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.size = (Uint32)(vertexBufferInfo.size + indexBufferInfo.size);
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    SDL_GPUTransferBuffer *gpuTransferBuffer = SDL_CreateGPUTransferBuffer(app.gpuDevice, &transferInfo);

    // fill the transfer buffer
    Uint8 *mapped = (Uint8 *)SDL_MapGPUTransferBuffer(app.gpuDevice, gpuTransferBuffer, false);

    // mapped[0] = vertices[0];
    // mapped[1] = vertices[1];
    // mapped[2] = vertices[2];
    // mapped[3] = indices[0];
    // mapped[4] = indices[1];
    // mapped[5] = indices[2];
    SDL_memcpy(mapped, vertices.data(), vertexBufferInfo.size);
    SDL_memcpy(mapped + vertexBufferInfo.size, indices.data(), indexBufferInfo.size);
    SDL_UnmapGPUTransferBuffer(app.gpuDevice, gpuTransferBuffer);

    // start a copy pass
    SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(app.gpuDevice);
    SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    // where is the data
    SDL_GPUTransferBufferLocation location{};
    location.transfer_buffer = gpuTransferBuffer;
    location.offset = 0;

    // where to upload the data
    SDL_GPUBufferRegion region{};
    region.buffer = app.gpuVertexBuffer;
    region.size = vertexBufferInfo.size;
    region.offset = 0;

    // upload the vertex data
    SDL_UploadToGPUBuffer(copyPass, &location, &region, false);

    // update the data for index buffer
    location.offset = vertexBufferInfo.size;
    region.buffer = app.gpuIndexBuffer;
    region.size = indexBufferInfo.size;

    // upload the index data
    SDL_UploadToGPUBuffer(copyPass, &location, &region, false);

    // end the copy pass
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);
    SDL_ReleaseGPUTransferBuffer(app.gpuDevice, gpuTransferBuffer);

    return SDL_APP_CONTINUE;
}

int App::Render3D()
{
    // acquire the command buffer
    SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(app.gpuDevice);

    // get the swapchain texture
    SDL_GPUTexture *swapchainTexture;
    Uint32 width, height;
    SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, app.window, &swapchainTexture, &width, &height);

    // end the frame early if a swapchain texture is not available
    if (swapchainTexture == NULL)
    {
        // you must always submit the command buffer
        SDL_SubmitGPUCommandBuffer(commandBuffer);
        return SDL_APP_CONTINUE;
    }

    // camera matrices
    CameraData camera;
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    // converting OpenGL (glm default) axis direction into Vulkan axis direction
    projection[1][1] *= 1;
    camera.viewProjection = projection * view;

    // push uniform data to buffer
    SDL_PushGPUVertexUniformData(commandBuffer, 0, &camera, sizeof(CameraData));

    // create the color target (now +depth)
    SDL_GPUColorTargetInfo colorTargetInfo{};
    colorTargetInfo.clear_color = {0.1f, 0.1f, 0.1f, 1.0f};
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorTargetInfo.texture = swapchainTexture;

    SDL_GPUDepthStencilTargetInfo depthTargetInfo{};
    depthTargetInfo.clear_depth = 1.0f;
    depthTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    depthTargetInfo.store_op = SDL_GPU_STOREOP_DONT_CARE;
    depthTargetInfo.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
    depthTargetInfo.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
    depthTargetInfo.texture = app.depthTexture;
    depthTargetInfo.cycle = true;

    // begin a render pass
    SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, &depthTargetInfo);

    // draw calls go here
    SDL_BindGPUGraphicsPipeline(renderPass, app.gpuGraphicsPipeline);

    // binding vertex buffer
    SDL_GPUBufferBinding vertexBufferBindings[1];
    vertexBufferBindings[0].buffer = app.gpuVertexBuffer;
    vertexBufferBindings[0].offset = 0;
    SDL_BindGPUVertexBuffers(renderPass, 0, vertexBufferBindings, 1);

    // binding index buffer
    SDL_GPUBufferBinding indexBufferBindings;
    indexBufferBindings.buffer = app.gpuIndexBuffer;
    indexBufferBindings.offset = 0;
    SDL_BindGPUIndexBuffer(renderPass, &indexBufferBindings, SDL_GPU_INDEXELEMENTSIZE_32BIT);

    // issue a draw call
    SDL_DrawGPUPrimitives(renderPass, app.indexCount, 1, 0, 0);

    // end the render pass
    SDL_EndGPURenderPass(renderPass);

    // submit the command buffer
    SDL_SubmitGPUCommandBuffer(commandBuffer);

    return SDL_APP_CONTINUE;
}

bool App::LoadGLTF(const char *filePath,
                   std::vector<Vertex> &outVertices, std::vector<Uint32> &outIndices)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    if (!loader.LoadASCIIFromFile(&model, &err, &warn, filePath))
        return false;

    // grab first mesh and primitive
    const tinygltf::Mesh &mesh = model.meshes[0];
    const tinygltf::Primitive &primitive = mesh.primitives[0];

    // read indices
    const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
    const tinygltf::BufferView &indexBufferView = model.bufferViews[indexAccessor.bufferView];
    const tinygltf::Buffer &indexBuffer = model.buffers[indexBufferView.buffer];

    const uint16_t *indicesData =
        SDL_reinterpret_cast(const uint16_t *,
                             &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset]);

    for (size_t i = 0; i < indexAccessor.count; ++i)
    {
        outIndices.push_back(indicesData[i]);
    }

    // read position and normal
    const tinygltf::Accessor positionAccessor = model.accessors[primitive.attributes.at("POSITION")];
    const tinygltf::BufferView positionBufferView = model.bufferViews[positionAccessor.bufferView];
    const float *positionData =
        SDL_reinterpret_cast(const float *,
                             &model.buffers[positionBufferView.buffer].data[positionBufferView.byteOffset + positionAccessor.byteOffset]);

    const tinygltf::Accessor normalAccessor = model.accessors[primitive.attributes.at("NORMAL")];
    const tinygltf::BufferView normalBufferView = model.bufferViews[normalAccessor.bufferView];
    const float *normalData =
        SDL_reinterpret_cast(const float *,
                             &model.buffers[normalBufferView.buffer].data[normalBufferView.byteOffset + normalAccessor.byteOffset]);

    for (size_t i = 0; i < positionAccessor.count; ++i)
    {
        Vertex vertex;
        vertex.position = glm::vec3(positionData[i * 3 + 0], positionData[i * 3 + 1], positionData[i * 3 + 2]);
        vertex.normal = glm::vec3(normalData[i * 3 + 0], normalData[i * 3 + 1], normalData[i * 3 + 2]);
        outVertices.push_back(vertex);
    }

    return true;
}
