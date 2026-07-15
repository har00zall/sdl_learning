#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <vector>

#define INSTANCES 100

struct UniformBufferObject
{
    glm::mat4 viewProjection;
    glm::mat4 model[100];
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
    float orbitSpeed = 0.05f;
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
    SDL_GPUBuffer *gpuIndexBuffer;   // gltf indices
    SDL_GPUBufferCreateInfo gpuUniformBufferInfo;
    SDL_GPUBuffer *gpuUniformBuffer; // camera matrix

    SDL_GPUTexture *depthTexture; // z-buffer

    SDL_GPUGraphicsPipeline *gpuGraphicsPipeline;

    SDL_Window *CreateWindowImpl();

    UniformBufferObject uniformBufferObject;
    Mesh mesh;

    int CreateRenderer3D();
    int Render3D();
};

static App app{};
static CameraObject cameraObject{};

void OrbitCamera(float deltaTime, glm::mat4 &viewMatrix);
void LoadMesh(const char *filePath, Mesh &outMesh);

bool LoadGLTF(const char *filePath,
                  std::vector<Vertex> &outVertices, std::vector<Uint32> &outIndices);

SDL_GPUShader *CreateShader(char *shaderFilePath,
                            SDL_GPUShaderFormat shaderFormat, SDL_GPUShaderStage shaderStage,
                            int num_sampler = 0, int num_storage_buffers = 0, int num_storage_textures = 0, int num_uniform_buffers = 0);


