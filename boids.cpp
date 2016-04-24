#include <iostream>
#include <algorithm>
#include <iomanip>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtx/string_cast.hpp"
#include "common/debug.h"
#include "common/programobject.h"
#include "common/objloader.h"
#include "common/meshbuffer.h"
#include "common/meshobject.h"
#include "common/torusgenerator.h"
#include "common/cubegenerator.h"
#include "common/curvegenerator.h"
#include "common/renderable.h"

#include "particles.h"
#include "particlesDrawable.h"

#include <pnglite.h>

using namespace std;
using namespace ogle;

std::string DataDirectory; // ends with a forward slash
int WINDOW_WIDTH = 1024;
int WINDOW_HEIGHT = 1024;
GLFWwindow* glfwWindow;

Particles Boids;
ParticlesDrawable BoidsDrawable;
ProgramObject BoidsShader;

glm::mat4x4 Camera;
glm::mat4x4 SceneBase;
glm::mat4x4 Projection;
glm::mat4x4 ProjectionView;

glm::vec3 ViewTarget;

MeshObject FlowVolume;
ProgramObject FlowVolumeShader;

MeshObject DebugTorus;
ProgramObject TorusShader;

Renderable DebugLines;
ProgramObject LinesShader;

bool MousePositionCapture = false;
glm::vec2 PrevMousePos;

Renderable Cube;
ProgramObject CubeShader;
const unsigned int CubeCount = 10;
glm::mat4x4 Models[CubeCount];
glm::vec4 Colors[CubeCount];
GLuint NormalMap = 0;

void errorCallback(int error, const char* description)
{
    cerr << description << endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (mods == GLFW_MOD_CONTROL && action == GLFW_RELEASE){
        MousePositionCapture = false;
        glfwSetCursorPosCallback(glfwWindow, nullptr);
    }

    if (key == GLFW_KEY_P && action == GLFW_RELEASE){
        png_t out_img = {0};

        int error = 0;
        error = png_open_file_write(&out_img, "screen_shot.png");

        int width = WINDOW_WIDTH;
        int height = WINDOW_HEIGHT;
        int comp_count = 4;
        int bytes_per = 1;
        unsigned char* data = new unsigned char[width * height * comp_count * bytes_per];
        glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, data);

        int bits_per_pixel = 8;
        error = png_set_data(&out_img, width, height, bits_per_pixel, PNG_TRUECOLOR_ALPHA, data);
        error = png_close_file(&out_img);
        delete [] data; data = 0;
    }
}

void cursorCallback(GLFWwindow* window, double x, double y)
{
    if (MousePositionCapture){
        glm::vec2 curr = glm::vec2(float(x), float(y));
        glm::vec2 diff = curr - PrevMousePos;
        PrevMousePos = curr;

        glm::vec3 rotation_axis = glm::vec3(diff.y, diff.x, 0.f);
        float mag = glm::length(rotation_axis);
        if (mag == 0) return;

        rotation_axis = rotation_axis / mag;
        glm::mat4x4 cam = glm::mat4x4(
            Camera[0],
            Camera[1],
            Camera[2],
            glm::vec4(ViewTarget,1)
            );

        float dist = glm::length(ViewTarget - glm::vec3(Camera[3]));
        glm::mat4x4 rot;
        rot = glm::rotate(rot, mag * 0.001f, rotation_axis);
        Camera = rot * cam;

        glm::vec3 position = dist * glm::vec3(Camera[2]);
        position = position * glm::mat3x3(Camera);
        Camera[3] = glm::vec4(position, 1.f);
        Camera[3][2] *= -1.0f;
    }
}

void mouseCallback(GLFWwindow* window, int btn, int action, int mods)
{
    if (mods == GLFW_MOD_ALT){
        if (btn == GLFW_MOUSE_BUTTON_1) {
            if(action == GLFW_PRESS) {
                double x,y;
                glfwGetCursorPos(glfwWindow, &x, &y);
                PrevMousePos = glm::vec2(float(x), float(y));
                MousePositionCapture = true;

                glfwSetCursorPosCallback(glfwWindow, cursorCallback);
            }
            if(action == GLFW_RELEASE) {
                MousePositionCapture = false;
                glfwSetCursorPosCallback(glfwWindow, nullptr);
            }
        }
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    float dist = glm::length(ViewTarget - glm::vec3(Camera[3]));
    dist -= 10.f * float(yoffset);
    glm::vec3 position = dist * -glm::vec3(Camera[2]);
    Camera[3][0] = -glm::dot( glm::vec3(Camera[0]), position);
    Camera[3][1] = -glm::dot( glm::vec3(Camera[1]), position);
    Camera[3][2] =  glm::dot( glm::vec3(Camera[2]), position);
    Camera[3][3] = 1.f;
}

void initGLFW(){
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    glfwWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Boids", NULL, NULL);
    if (!glfwWindow)
    {
        fprintf(stderr, "Failed to create GLFW glfwWindow\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    } 

    glfwMakeContextCurrent(glfwWindow);
    glfwSwapInterval( 0 ); // Turn off vsync for benchmarking.

    glfwSetTime( 0.0 );
    glfwSetKeyCallback(glfwWindow, keyCallback);
    glfwSetErrorCallback(errorCallback);
    glfwSetMouseButtonCallback(glfwWindow, mouseCallback);
    glfwSetScrollCallback(glfwWindow, scrollCallback);
}

void initGLAD(){
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "initGLAD: Failed to initialize OpenGL context" << std::endl;
        exit(EXIT_FAILURE);
    }

    int width,height;
    glfwGetFramebufferSize(glfwWindow, &width, &height);
    glViewport( 0, 0, (GLsizei)width, (GLsizei)height );    
}

void setDataDir(int argc, char *argv[]){
    // get base directory for reading in files
    DataDirectory = "./data/";
}

void initBoids(){
    std::map<unsigned int, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = DataDirectory + "boid.vert";
    shaders[GL_GEOMETRY_SHADER] = DataDirectory + "boid.geom";
    shaders[GL_FRAGMENT_SHADER] = DataDirectory + "boid.frag";
    BoidsShader.init(shaders);

    Boids.init(20);
    BoidsDrawable.init(Boids);
}

void initMesh(){
    // load up mesh
    ogle::ObjLoader loader;
    loader.load(DataDirectory + "Anatomy_A.obj");

    MeshBuffer buffer;
    buffer.setVerts(loader.getVertCount(), loader.getPositions());
    buffer.setNorms(loader.getVertCount(), loader.getNormals());
    buffer.setIndices(loader.getIndexCount(), loader.getIndices());
    buffer.generateFaceNormals();
    FlowVolume.init(buffer);

    std::map<unsigned int, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = DataDirectory + "diffuse.vert";
    shaders[GL_FRAGMENT_SHADER] = DataDirectory + "diffuse.frag";
    FlowVolumeShader.init(shaders);
}

void initLines(){

    ogle::CurveGenerator lines;
    lines.addLineSegments(Boids.Constraint);
    lines.generate();

    MeshBuffer buffer;
    buffer.setVerts(lines.Positions.size(), (const float*)lines.Positions.data());
    buffer.setGenerics(0, lines.PrevPoint);
    buffer.setGenerics(1, lines.NextPoint);

    buffer.setIndices(lines.Indices.size(), lines.Indices.data());
    DebugLines.init(buffer);

    std::map<unsigned int, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = DataDirectory + "lines.vert";
    shaders[GL_FRAGMENT_SHADER] = DataDirectory + "diffuse.frag";
    LinesShader.init(shaders);
}

void initTorus(){

    ogle::TorusGenerator torus;
    torus.centerlinePath(Boids.Constraint);
    torus.generate();

    MeshBuffer buffer;
    buffer.setVerts(torus.Positions.size(), (const float*)torus.Positions.data());
    buffer.setNorms(torus.Normals.size(), (const float*)torus.Normals.data());
    buffer.setIndices(torus.Indices.size(), torus.Indices.data());
    DebugTorus.init(buffer);

    std::map<unsigned int, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = DataDirectory + "diffuse.vert";
    shaders[GL_FRAGMENT_SHADER] = DataDirectory + "diffuse.frag";
    TorusShader.init(shaders);
}

void initView(){
    float fovy = glm::radians(30.f);
    Projection = glm::perspective<float>(fovy, WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1f, 1000.0f );

    // glm::vec3 diagonal = FlowVolume.PivotPoint - FlowVolume.AABBMin;
    // float opposite = glm::length(diagonal);
    float opposite = 3.f;
    float adjacent = atan(fovy*.5f) / opposite;
    adjacent = 1.f / adjacent;

    ViewTarget = glm::vec3(0);
    glm::vec3 direction(0,0,1);
    glm::vec3 eye = FlowVolume.PivotPoint + direction * adjacent;
    Camera = glm::lookAt(eye, ViewTarget, glm::vec3(0,1,0));
}

void initCubes(){
    ogle::CubeGenerator shape;
    shape.scale(1.f);
    shape.generate();

    MeshBuffer buffer;
    buffer.setVerts(shape.Positions.size(), (const float*)shape.Positions.data());
    buffer.setNorms(shape.Positions.size(), (const float*)shape.Normals.data());
    buffer.setTexCoords(0, shape.Positions.size(), (const float*)shape.TexCoords.data());

    buffer.setIndices(shape.Indices.size(), shape.Indices.data());
    Cube.init(buffer);

    std::map<unsigned int, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = DataDirectory + "cube.vert";
    shaders[GL_FRAGMENT_SHADER] = DataDirectory + "cube.frag";
    CubeShader.init(shaders);

    for (int i=0; i<CubeCount; ++i)
    {   
        float randval = 2;
        glm::vec4 color(glm::linearRand(0.f,1.f),glm::linearRand(0.f,1.f),glm::linearRand(0.f,1.f), 1.f); 
        glm::vec4 position(glm::linearRand(-randval,randval),glm::linearRand(-randval,randval),glm::linearRand(-randval,randval), 1);
        Models[i][3] = position;
        Colors[i] = color;
    }

    png_t img = {0};
    png_open_file_read(&img, (DataDirectory + "NM13.png").c_str());
    unsigned char* img_data;
    img_data = new unsigned char[img.width * img.height * img.bpp];
    png_get_data(&img, img_data);
    png_close_file(&img);
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &NormalMap);
    glBindTexture(GL_TEXTURE_2D, NormalMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLvoid*)img_data);
    glFlush();
    
    glBindTexture(GL_TEXTURE_2D, 0);
    delete [] img_data;
}

void init(int argc, char* argv[]){
    setDataDir(argc, argv);
    initGLFW();
    initGLAD();
    ogle::Debug::init();
    png_init(0,0);

    // initBoids();
    // initMesh();
    // initTorus();
    // initLines();
    initCubes();
    initView();
}

void update(){
    float deltaTime = (float)glfwGetTime(); // get's the amount of time since last setTime
    glfwSetTime(0);

    ProjectionView = Projection * Camera;

    Boids.update(deltaTime);
    BoidsDrawable.update(Boids);
}

void renderBoids(){
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    BoidsShader.bind();
    BoidsShader.setFloat(10.1f, "Radius");
    BoidsShader.setMatrix44((const float*)&ProjectionView, "ProjectionView");
    BoidsDrawable.render(Boids);
}

void renderFlowVolume(){
    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    FlowVolumeShader.bind();
    FlowVolumeShader.setMatrix44((const float*)&ProjectionView, "ProjectionView");
    FlowVolume.render();
}

void renderTorus(){
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // glEnable(GL_CULL_FACE);

    TorusShader.bind();
    TorusShader.setMatrix44((const float*)&ProjectionView, "ProjectionView");
    DebugTorus.render();
    // glDisable(GL_CULL_FACE);
}

void renderLines(){
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    LinesShader.bind();
    LinesShader.setMatrix44((const float*)&ProjectionView, "ProjectionView");

    DebugLines.render();
    glDisable(GL_CULL_FACE);
}

void renderCubes(){
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, NormalMap);

    CubeShader.bind();
    CubeShader.setMatrix44((const float*)&Camera, "View");
    CubeShader.setMatrix44((const float*)&Projection, "Projection");
    CubeShader.setFloat(10, "Shininess");

    for (int i=0; i<CubeCount; ++i)
    {
        CubeShader.setMatrix44((const float*)&Models[i], "Model");
        CubeShader.setVec4((const float*)&Colors[i], "Diffuse");
        Cube.render();
    }
}

void render(){
    glClearColor( 0,0,0,0 );
    glClearDepth( 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // renderFlowVolume();
    // renderTorus();
    // renderLines();
    // renderBoids();
    renderCubes();
}

void runloop(){
    glfwSetTime(0); // init timer
    while (!glfwWindowShouldClose(glfwWindow)){
        glfwPollEvents();
        update();
        render();
        glfwSwapBuffers(glfwWindow);
    }
}

void shutdown(){

    Boids.shutdown();
    BoidsDrawable.shutdown();
    BoidsShader.shutdown();

    FlowVolumeShader.shutdown();
    TorusShader.shutdown();
    LinesShader.shutdown();

    FlowVolume.shutdown();
    DebugTorus.shutdown();
    DebugLines.shutdown();

    glDeleteTextures(1, &NormalMap);
    CubeShader.shutdown();
    Cube.shutdown();

    ogle::Debug::shutdown();
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();    
}

int main(int argc, char* argv[]){
    init(argc, argv);
    runloop();
    shutdown();
    exit(EXIT_SUCCESS);
}
