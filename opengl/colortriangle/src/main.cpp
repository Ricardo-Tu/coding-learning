#define GL_GLEXT_PROTOTYPES
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GLES3/gl32.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <chrono>

#include "../colortriangle/utils.hpp"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

uint32_t gWinWidth = 800;
uint32_t gWinHeight = 600;

struct Vertex
{
    glm::vec3 position;
    glm::vec4 color;
};

struct alignas(16) MVP
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 project;
};

MVP mvp = {};

std::vector<Vertex> basevertex = {
    {{200, 200, 0}, {1, 0, 0, 1}},
    {{600, 200, 0}, {0, 1, 0, 1}},
    {{600, 400, 0}, {0, 0, 1, 1}},
    {{200, 200, 0}, {0, 1, 0, 1}},
    {{200, 400, 0}, {0, 0, 1, 1}},
    {{600, 400, 0}, {1, 1, 1, 1}}};

std::vector<Vertex> vertex = {
    {{200, 200, 0}, {1, 0, 0, 1}},
    {{600, 200, 0}, {0, 1, 0, 1}},
    {{600, 400, 0}, {0, 0, 1, 1}},
    {{200, 200, 0}, {0, 1, 0, 1}},
    {{200, 400, 0}, {0, 0, 1, 1}},
    {{600, 400, 0}, {1, 1, 1, 1}}};

GLuint indics[] = {0, 1, 2, 0, 4, 5};

typedef float t_mat4x4[16];
void UpdateUniformBuffer();

static inline void mat4x4_ortho(t_mat4x4 out, float left, float right, float bottom, float top, float znear, float zfar)
{
#define T(a, b) (a * 4 + b)

    out[T(0, 0)] = 2.0f / (right - left);
    out[T(0, 1)] = 0.0f;
    out[T(0, 2)] = 0.0f;
    out[T(0, 3)] = 0.0f;

    out[T(1, 1)] = 2.0f / (top - bottom);
    out[T(1, 0)] = 0.0f;
    out[T(1, 2)] = 0.0f;
    out[T(1, 3)] = 0.0f;

    out[T(2, 2)] = -2.0f / (zfar - znear);
    out[T(2, 0)] = 0.0f;
    out[T(2, 1)] = 0.0f;
    out[T(2, 3)] = 0.0f;

    out[T(3, 0)] = -(right + left) / (right - left);
    out[T(3, 1)] = -(top + bottom) / (top - bottom);
    out[T(3, 2)] = -(zfar + znear) / (zfar - znear);
    out[T(3, 3)] = 1.0f;
    
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            mvp.project[i][j] = out[T(i, j)];
        }
    }

#undef T

}

int main(int argc, char *argv[])
{
    bool isquit = false;
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window *window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, gWinWidth, gWinHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    GLuint vs, fs, program;

    vs = glCreateShader(GL_VERTEX_SHADER);
    fs = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vertexShader = readfile("./shader/colortriangle.vert");
    int length = vertexShader.length();
    const GLchar *ptr = vertexShader.c_str();
    glShaderSource(vs, 1, (const GLchar **)&ptr, nullptr);
    glCompileShader(vs);

    GLint status;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        throw std::runtime_error("vertex shader compilation failed");
        return 1;
    }

    std::string fragmentShader = readfile("./shader/colortriangle.frag");
    length = fragmentShader.length();
    ptr = fragmentShader.c_str();
    glShaderSource(fs, 1, (const GLchar **)&ptr, nullptr);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        throw std::runtime_error("fragment shader compilation failed");
        return 1;
    }

    program = glCreateProgram();
    UpdateUniformBuffer();
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "color");
    glLinkProgram(program);

    glUseProgram(program);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0.0, 0.0, 0.0);
    glViewport(0, 0, gWinWidth, gWinHeight);

    GLuint vao, vbo, ebo, uboBlock;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glGenBuffers(1, &uboBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MVP), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    const auto vpIndex = glGetUniformBlockIndex(program, "ubo"); // 获取着色器中的 uniform 缓冲对象位置索引
    glUniformBlockBinding(program, vpIndex, 0);         // 将其绑定到 0 绑定点
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBlock);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(sizeof(vertex[0].position)));

    glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(Vertex), vertex.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indics), indics, GL_STATIC_DRAW);

    t_mat4x4 projection_matrix;
    mat4x4_ortho(projection_matrix, 0.0f, (float)gWinWidth, (float)gWinHeight, 0.0f, 0.0f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(program, "u_projection_matrix"), 1, GL_FALSE, projection_matrix);

    SDL_Event event;
    while (!isquit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                isquit = true;
            if (event.type == SDL_WINDOWEVENT)
            {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    for (size_t i = 0; i < vertex.size(); i++)
                    {
                        SDL_GetWindowSize(window, (int *)&gWinWidth, (int *)&gWinHeight);
                        vertex[i].position.x = gWinWidth / WINDOW_WIDTH * basevertex[i].position.x;
                        vertex[i].position.y = gWinHeight / WINDOW_HEIGHT * basevertex[i].position.y;
                    }
                }
            }
        }
        UpdateUniformBuffer();
        glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &mvp.model);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &mvp.view);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4), &mvp.project);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glViewport(0, 0, gWinWidth, gWinHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, sizeof(indics), GL_UNSIGNED_INT, 0);
        SDL_GL_SwapWindow(window);
        SDL_Delay(1);
    }
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void UpdateUniformBuffer()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    mvp.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    mvp.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    // mvp.project = glm::perspective(glm::radians(45.0f), (float)gWinWidth / (float)gWinHeight, 0.1f, 10.0f);
    // mvp.model[1][1] *= -2;
    // mvp.view[1][1] *= 2;
    // mvp.project[1][1] *= 1;
    // mvp.project[0][0] = 2.0f / (gWinWidth - 0);
    // mvp.project[0][1] = 0.0f;
    // mvp.project[0][2] = 0.0f;
    // mvp.project[0][3] = 0.0f;

    // mvp.project[1][1] = 2.0f / (0 - gWinHeight);
    // mvp.project[1][0] = 0.0f;
    // mvp.project[1][2] = 0.0f;
    // mvp.project[1][3] = 0.0f;

    // mvp.project[2][2] = -2.0f / (100.0f - 0);
    // mvp.project[2][0] = 0.0f;
    // mvp.project[2][1] = 0.0f;
    // mvp.project[2][3] = 0.0f;

    // mvp.project[3][0] = -(gWinWidth + 0) / (gWinWidth - 0);
    // mvp.project[3][1] = -(0 + gWinHeight) / (0 - gWinHeight);
    // mvp.project[3][2] = -(100.0f + 0) / (100.0f - 0);
    // mvp.project[3][3] = 1.0f;
}