#include <iostream>
#include <algorithm>
#include <iomanip>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "common/debug.h"
#include "common/programobject.h"
#include "common/objloader.h"
#include "common/meshbuffer.h"
#include "common/meshobject.h"
#include "common/torusgenerator.h"
#include "common/curvegenerator.h"
#include "common/renderable.h"

#include "particles.h"
#include "particlesDrawable.h"

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
    dist -= 50.f * float(yoffset);
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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
    cout << "[!] Warning, be sure that vsync is disabled in NVidia controller panel." << endl;

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

    glm::vec3 diagonal = FlowVolume.PivotPoint - FlowVolume.AABBMin;
    // float opposite = glm::length(diagonal);
    float opposite = 101.f;
    float adjacent = atan(fovy*.5f) / opposite;
    adjacent = 1.f / adjacent;

    ViewTarget = glm::vec3(0);
    glm::vec3 direction(0,0,1);
    glm::vec3 eye = FlowVolume.PivotPoint + direction * adjacent;
    Camera = glm::lookAt(eye, ViewTarget, glm::vec3(0,1,0));
}

void init(int argc, char* argv[]){
    setDataDir(argc, argv);
    initGLFW();
    initGLAD();
    ogle::Debug::init();
    initBoids();
//    initMesh();

    //initTorus();
    initLines();

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

void render(){
    glClearColor( 0,0,0,0 );
    glClearDepth( 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // renderFlowVolume();
    renderTorus();
    renderLines();
    renderBoids();
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
