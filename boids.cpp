#include <iostream>
#include <algorithm>

#define GLEW_NO_GLU
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "common/debug.h"
#include "common/programobject.h"
#include "common/objloader.h"
#include "common/meshbuffer.h"
#include "common/meshobject.h"

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
glm::mat4x4 Projection;
glm::mat4x4 ProjectionView;

MeshObject FlowVolume;
ProgramObject FlowVolumeShader;

void errorCallback(int error, const char* description)
{
    cerr << description << endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void mouseCallback(GLFWwindow* window, int btn, int action, int mods)
{
    if (btn == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
    }
    if (btn == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
    }
}

void initGLFW(){
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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

    int width,height;
    glfwGetFramebufferSize(glfwWindow, &width, &height);
    glViewport( 0, 0, (GLsizei)width, (GLsizei)height );

    glfwSetTime( 0.0 );
    glfwSetKeyCallback(glfwWindow, keyCallback);
    glfwSetErrorCallback(errorCallback);
    glfwSetMouseButtonCallback(glfwWindow, mouseCallback);
}

void initGLEW(){
    glewExperimental=GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        /// if it fails here, its becuase there is no current opengl version,
        /// don't call glewInit() until after a context has been made current.
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit( EXIT_FAILURE );
    }
    glGetError(); // GLEW has problems, clear the one that it creates.
}

void setDataDir(int argc, char *argv[]){
    // get base directory for reading in files
    DataDirectory = "../data/";
}

void initBoids(){
    std::map<unsigned int, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = DataDirectory + "boid.vert";
    shaders[GL_GEOMETRY_SHADER] = DataDirectory + "boid.geom";
    shaders[GL_FRAGMENT_SHADER] = DataDirectory + "boid.frag";
    BoidsShader.init(shaders);

    BoidsDrawable.init();
    
    Boids.init(10);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);
}

void initMesh(){
    // load up mesh
    ogle::ObjLoader loader;
    loader.load(DataDirectory + "Anatomy_A.obj");

    MeshBuffer buffer;
    buffer.setVerts(loader.getVertCount(), loader.getPositions());
    buffer.setNorms(loader.getVertCount(), loader.getNormals());
    buffer.setIndices(loader.getIndexCount(), loader.getIndices());
    FlowVolume.init(buffer); 
 
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    std::map<unsigned int, std::string> shaders;
    shaders[GL_VERTEX_SHADER] = DataDirectory + "diffuse.vert";
    shaders[GL_FRAGMENT_SHADER] = DataDirectory + "diffuse.frag";
    FlowVolumeShader.init(shaders);
}

void initView(){
    float fovy = glm::radians(30.f);
    Projection = glm::perspective<float>(fovy, WINDOW_WIDTH/WINDOW_HEIGHT, 0.1f, 10000.0f );

    glm::vec3 diagonal = FlowVolume.PivotPoint - FlowVolume.AABBMin;
    float opposite = glm::length(diagonal);
    float adjacent = atan(fovy*.5f) / opposite;
    adjacent = 1.f / adjacent;

    glm::vec3 direction(0,0,1);
    glm::vec3 eye = FlowVolume.PivotPoint + direction * adjacent;
    Camera = glm::lookAt(eye, glm::vec3(0), glm::vec3(0,1,0));
}

void init(int argc, char* argv[]){
    setDataDir(argc, argv);
    initGLFW();
    initGLEW();
    ogle::Debug::init();
    // initBoids();
    initMesh();

    initView();
}

void update(){
    float deltaTime = glfwGetTime(); // get's the amount of time since last setTime
    glfwSetTime(0);

    ProjectionView = Projection * Camera;

    // Boids.update(deltaTime);
    // BoidsDrawable.update(Boids);
}

void renderBoids(){
    BoidsShader.bind();
    BoidsShader.setFloat(0.1f, "Radius");
    BoidsDrawable.render();
}

void render(){
    glClearColor( 0,0,0,0 );
    glClearDepth( 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // renderBoids();

    FlowVolumeShader.bind();
    FlowVolumeShader.setMatrix44((const float*)&ProjectionView, "ProjectionView");
    FlowVolume.render();
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