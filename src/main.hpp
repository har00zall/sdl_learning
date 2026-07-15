#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <vector>

#define INSTANCES 100

struct StorageBufferObject
{
    glm::mat4 viewProjection;
    glm::mat4 model[INSTANCES];
};

struct FragmentUniformBufferData
{
    glm::vec3 viewPosition;
};

struct Transform
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    glm::mat4 GetModelMatrix();
};

struct CameraObject
{
    float orbitAngle = 0.f;
    float orbitSpeed = 0.01f;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<Uint32> indices;
    Uint32 GetIndexCount();
    void Render(SDL_GPURenderPass *renderPass, Uint32 num_instances = 1, Uint32 first_index = 0, Sint32 vertex_offset = 0, Uint32 first_instance = 0);
};

struct App
{
    SDL_Window *window;
    SDL_GPUDevice *gpuDevice;

    SDL_GPUBufferCreateInfo gpuVertexBufferInfo;
    SDL_GPUBuffer *gpuVertexBuffer;
    SDL_GPUBufferCreateInfo gpuIndexBufferInfo;
    SDL_GPUBuffer *gpuIndexBuffer;
    SDL_GPUBufferCreateInfo gpuStorageBufferInfo;
    SDL_GPUBuffer *gpuStorageBuffer;
    SDL_GPUTransferBufferCreateInfo gpuStorageTransferBufferInfo;
    SDL_GPUTransferBuffer *gpuStorageTransferBuffer;

    SDL_GPUTexture *depthTexture; // z-buffer

    SDL_GPUGraphicsPipeline *gpuGraphicsPipeline;

    SDL_Window *CreateWindowImpl();

    StorageBufferObject storageBufferObject;
    Mesh mesh;

    int CreateRenderer3D();
    int Render3D();
};

static App app{};
static CameraObject cameraObject{};

void OrbitCamera(float deltaTime, glm::mat4 &viewMatrix, glm::vec3 &cameraPosition);
void LoadMesh(const char *filePath, Mesh &outMesh);

bool LoadGLTF(const char *filePath,
              std::vector<Vertex> &outVertices, std::vector<Uint32> &outIndices);

SDL_GPUShader *CreateShader(char *shaderFilePath,
                            SDL_GPUShaderFormat shaderFormat, SDL_GPUShaderStage shaderStage,
                            Uint32 num_sampler = 0, Uint32 num_storage_buffers = 0, Uint32 num_storage_textures = 0, Uint32 num_uniform_buffers = 0);
