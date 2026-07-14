#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <vector>

struct CameraData
{
    glm::mat4 viewProjection;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

struct App
{
    SDL_Window *window;
    SDL_GPUDevice *gpuDevice;

    SDL_GPUBuffer *gpuVertexBuffer;
    SDL_GPUBuffer *gpuIndexBuffer;   // gltf indices
    SDL_GPUBuffer *gpuUniformBuffer; // camera matrix

    SDL_GPUTexture *depthTexture; // z-buffer

    SDL_GPUGraphicsPipeline *gpuGraphicsPipeline;

    Uint32 indexCount; // how many indices to draw

    SDL_Window *CreateWindow();

    int CreateRenderer3D();
    int Render3D();

    bool LoadGLTF(const char *filePath,
                  std::vector<Vertex> &outVertices, std::vector<Uint32> &outIndices);
};

static App app{};

SDL_GPUShader *CreateShader(char *shaderFilePath,
                            SDL_GPUShaderFormat shaderFormat, SDL_GPUShaderStage shaderStage,
                            int num_sampler = 0, int num_storage_buffers = 0, int num_storage_textures = 0, int num_uniform_buffers = 0);
