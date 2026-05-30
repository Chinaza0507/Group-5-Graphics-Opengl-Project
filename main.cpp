#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow* window);

// ─── Settings 
const unsigned int SCR_WIDTH  = 800;
const unsigned int SCR_HEIGHT = 600;

// ─── Shaders 
//  a per-vertex colour AND a 4x4 transform matrix uniform

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 ourColor;
uniform mat4 transform;
void main()

{

    gl_Position = transform * vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in  vec3 ourColor;
out vec4 FragColor;
void main()
{
FragColor = vec4(ourColor, 1.0);

}
)";



struct Mat4 {

    float d[16];

    // Initialise to identity

    Mat4() {
        for (int i = 0; i < 16; i++) d[i] = 0.0f;
        d[0] = d[5] = d[10] = d[15] = 1.0f;
}

};

// Rotation around the Y axis

Mat4 rotateY(float angle)
{
  Mat4 m;
  m.d[0]  =  cosf(angle);   // col0 row0
  m.d[2]  = -sinf(angle);   // col0 row2
  m.d[8]  =  sinf(angle);   // col2 row0
  m.d[10] =  cosf(angle);   // col2 row2
return m;
}

// Rotation around the X axis
Mat4 rotateX(float angle)

{
Mat4 m;
  m.d[5]  =  cosf(angle);   // col1 row1
  m.d[6]  =  sinf(angle);   // col1 row2
  m.d[9]  = -sinf(angle);   // col2 row1
  m.d[10] =  cosf(angle);   // col2 row2
return m;
}

// Column-major matrix multiply:  result = a * b
Mat4 mul(const Mat4& a, const Mat4& b)
{

    Mat4 r;
   for (int col = 0; col < 4; col++)
      for (int row = 0; row < 4; row++) {
       r.d[col * 4 + row] = 0.0f;
        for (int k = 0; k < 4; k++)
        r.d[col * 4 + row] += a.d[k * 4 + row] * b.d[col * 4 + k];
}

    return r;

}

// ─── Main 

int main()

{

    // ── GLFW init 

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

#endif
   GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, 
                                         "3D Nonagon", NULL, NULL);
if (!window) {
     std::cout << "Failed to create GLFW window" << std::endl;
     glfwTerminate();
     return -1;
}

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialise GLAD" << std::endl;
  return -1;
}

    // ── Build & compile shaders 

    int  success;
    char infoLog[512];
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
}

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
}

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
      glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
}

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ──  nonagonal prism geometry 

    //    Each vertex: [x, y, z,  r, g, b]   (stride = 6 floats)

    const int   N  = 9;           // 9 sides  → nonagon
    const float R  = 0.35f;        // radius
    const float H  = 0.35f;       // half-height
    const float PI = 3.14159265358979f;

    //  purple colour palette 

    float topCol[3]  = { 0.75f, 0.45f, 0.95f };   // light purple top

    float botCol[3]  = { 0.50f, 0.20f, 0.75f };   // deeper purple bottom

    float sideCol[9][3] = {

        { 0.72f, 0.40f, 0.92f },   // 0
        { 0.55f, 0.18f, 0.80f },   // 1
        { 0.78f, 0.48f, 0.95f },   // 2
        { 0.50f, 0.15f, 0.75f },   // 3
        { 0.68f, 0.35f, 0.90f },   // 4
        { 0.60f, 0.22f, 0.82f },   // 5
        { 0.80f, 0.50f, 0.96f },   // 6
        { 0.48f, 0.12f, 0.72f },   // 7
        { 0.70f, 0.30f, 0.88f },   // 8
};

    std::vector<float> verts;
    verts.reserve((9 + 9 + 18) * 3 * 6);   // 36 triangles × 3 verts × 6 floats
    auto push = [&](float x, float y, float z,
    float r, float g, float b) {
    verts.insert(verts.end(), { x, y, z, r, g, b });

    };

    // ── Top face: fan of 9 triangles ──

    for (int i = 0; i < N; i++) {
       float a1 = i       * 2.0f * PI / N;
       float a2 = (i + 1) * 2.0f * PI / N;
       push(0,            H,  0,            topCol[0],        topCol[1],        topCol[2]);
       push(R*cosf(a1),   H,  R*sinf(a1),   topCol[0]*0.90f,  topCol[1]*0.90f,  topCol[2]*0.90f);
       push(R*cosf(a2),   H,  R*sinf(a2),   topCol[0]*0.85f,  topCol[1]*0.85f,  topCol[2]*0.85f);

    }

    // ── Bottom face: fan of 9 triangles  ──

    for (int i = 0; i < N; i++) {
       float a1 = i       * 2.0f * PI / N;
       float a2 = (i + 1) * 2.0f * PI / N;
         push(0,       -H,  0,            botCol[0],        botCol[1],        botCol[2]);
         push(R*cosf(a2),   -H,  R*sinf(a2),   botCol[0]*0.90f,  botCol[1]*0.90f,  botCol[2]*0.90f);
         push(R*cosf(a1),   -H,  R*sinf(a1),   botCol[0]*0.85f,  botCol[1]*0.85f,  botCol[2]*0.85f);

    }

    // ── Side faces: 9 quads → 18 triangles ──

    for (int i = 0; i < N; i++) {
       float a1 = i       * 2.0f * PI / N;
       float a2 = (i + 1) * 2.0f * PI / N;
       float* c  = sideCol[i];
       float  dc = 0.75f;   // darken bottom edge for depth illusion

        // triangle 1
        push(R*cosf(a1),  H,  R*sinf(a1),  c[0],      c[1],      c[2]);
        push(R*cosf(a1), -H,  R*sinf(a1),  c[0]*dc,   c[1]*dc,   c[2]*dc);
        push(R*cosf(a2),  H,  R*sinf(a2),  c[0],      c[1],      c[2]);

        // triangle 2

        push(R*cosf(a2),  H,  R*sinf(a2),  c[0],      c[1],      c[2]);
        push(R*cosf(a1), -H,  R*sinf(a1),  c[0]*dc,   c[1]*dc,   c[2]*dc);
        push(R*cosf(a2), -H,  R*sinf(a2),  c[0]*dc,   c[1]*dc,   c[2]*dc);

    }

    // ── Upload to GPU 

    unsigned int VAO, VBO;
glGenVertexArrays(1, &VAO);
glGenBuffers(1, &VBO);
glBindVertexArray(VAO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER,
verts.size() * sizeof(float),
verts.data(),

GL_STATIC_DRAW);

    // Position  → layout location 0

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
6 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

    // Colour    → layout location 1

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
    6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    int vertexCount  = (int)(verts.size() / 6);

    // ── Render loop 

    while (!glfwWindowShouldClose(window))
   {
   processInput(window);

   // light purple background 
      glClearColor(0.82f, 0.75f, 0.95f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Build spinning transform:
        //   1. Constant tilt on X so the top face is visible
      //   2. Continuous spin on Y driven by elapsed time

        float time = (float)glfwGetTime();
       // tilt  0.70 for better top visibility

        Mat4 rx = rotateX(0.70f);          // ~40° tilt 
        Mat4 ry = rotateY(time * 1.2f);    // spin speed: 1.2 rad/s
        Mat4 transform = mul(rx, ry);      // rx applied after ry
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform.d);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    // ── Cleanup 

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();

    return 0;

}

// ─── Callbacks 

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
     glfwSetWindowShouldClose(window, true);
}
void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
{
glViewport(0, 0, width, height);
}
