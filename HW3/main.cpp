#include <stdlib.h>
#include <iostream>
// OSX systems need their own headers
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif
// Use of degrees is deprecated. Use radians for GLM functions
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Screenshot.h"
#include "Scene.h"

static const int width = 400;
static const int height = 300;
static const int swidth = 1024;
static const int sheight = 1024;
static const char* title = "Scene viewer";
static const glm::vec4 background(0.1f, 0.2f, 0.3f, 1.0f);
static Scene scene;
static DepthShader* depthShader;
static SurfaceShader* surfaceShader;

#include "hw3AutoScreenshots.h"

void printHelp(){
    std::cout << R"(
    Top-Left:     LiSPSM
    Top-Right:    LiSPSM + PCF
    Bottom-Left:  Base
    Bottom-Right: PCF
    Available commands:
      press 'H' to print this message again.
      press Esc to quit.
      press 'O' to save a screenshot.
      press the arrow keys to rotate camera.
      press 'A'/'Z' to zoom.
      press 'R' to reset camera.
      press 'B'/'b' to increase/decrease shadowBias.
      press 'N'/'n' to increase/decrease free parameter n.
      press 'S'/'s' to rotate the sun to different direction.
      press Spacebar to generate images based on hw3 submission images.
    
)";
}

void initialize(void){

    // Create Surface Shader
    surfaceShader = new SurfaceShader;
    surfaceShader->read_source("shaders/lightspace.vert", "shaders/lighting.frag");
    surfaceShader->compile();
    surfaceShader->initUniforms();

    //Create Depth Shader
    depthShader = new DepthShader;
    depthShader->read_source("shaders/projective.vert", "shaders/depth.frag");
    depthShader->compile();
    depthShader->initUniforms();

    printHelp();
    glClearColor(background[0], background[1], background[2], background[3]); // background color
    glViewport(0,0,width,height);
    
    // Initialize scene
    scene.init();
    scene.createTexture(swidth, sheight);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
}

void display(void){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    surfaceShader->enablePCF = 0;

    //// naive
    glViewport(width, 0, width, height);
    glUseProgram(depthShader->program);
    scene.drawShadowTexture(depthShader, 0, 0);

    glViewport(0, 0, swidth, sheight);
    glUseProgram(depthShader->program);
    scene.drawShadowTexture(depthShader, 1, 0);

    glViewport(0, 0, width, height);
    glUseProgram(surfaceShader->program);
    scene.draw(surfaceShader);


    // LiSPSM
    glViewport(width, height, width, height);
    glUseProgram(depthShader->program);
    scene.drawShadowTexture(depthShader, 0, 1);

    glViewport(0, 0, swidth, sheight);
    glUseProgram(depthShader->program);
    scene.drawShadowTexture(depthShader, 1, 1);

    glViewport(0, height, width, height);
    glUseProgram(surfaceShader->program);
    scene.draw(surfaceShader);

    // Enable PCF
    surfaceShader->enablePCF = 1;

    //// naive + PCF
    glViewport(2* width + width, 0, width, height);
    glUseProgram(depthShader->program);
    scene.drawShadowTexture(depthShader, 0, 0);

    glViewport(0, 0, swidth, sheight);
    glUseProgram(depthShader->program);
    scene.drawShadowTexture(depthShader, 1, 0);

    glViewport(2 * width, 0, width, height);
    glUseProgram(surfaceShader->program);
    scene.draw(surfaceShader);


    // LiSPSM + PCF
    glViewport(2 * width + width, height, width, height);
    glUseProgram(depthShader->program);
    scene.drawShadowTexture(depthShader, 0, 1);

    glViewport(0, 0, swidth, sheight);
    glUseProgram(depthShader->program);
    scene.drawShadowTexture(depthShader, 1, 1);

    glViewport(2 * width, height, width, height);
    glUseProgram(surfaceShader->program);
    scene.draw(surfaceShader);

    glutSwapBuffers();
    glFlush();
}

void saveScreenShot(const char* filename = "test.png"){
    int currentwidth = glutGet(GLUT_WINDOW_WIDTH);
    int currentheight = glutGet(GLUT_WINDOW_HEIGHT);
    Screenshot imag = Screenshot(currentwidth,currentheight);
    imag.save(filename);
}

void keyboard(unsigned char key, int x, int y){
    switch(key){
        case 27: // Escape to quit
            exit(0);
            break;
        case 'h': // print help
            printHelp();
            break;
        case 'o': // save screenshot
            saveScreenShot();
            break;
        case 'r':
            //scene.camera -> aspect_default = float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT));
            scene.camera -> reset();
            glutPostRedisplay();
            break;
        case 'a':
            scene.camera -> zoom(0.9f);
            glutPostRedisplay();
            break;
        case 'z':
            scene.camera -> zoom(1.1f);
            glutPostRedisplay();
            break;
        case ' ':
            hw3AutoScreenshots();
            glutPostRedisplay();
            break;
        // bias adjustment
        case 'B':
            surfaceShader->shadowBias += 0.001;
            std::cout << "shadowBias=" << surfaceShader->shadowBias << std::endl;
            glutPostRedisplay();
            break;
        case 'b':
            surfaceShader->shadowBias -= 0.001;
            std::cout << "shadowBias=" << surfaceShader->shadowBias << std::endl;
            glutPostRedisplay();
            break;
        case 'N':
            scene.free_param_n += 0.5;
            std::cout << "free_param_n=" << scene.free_param_n << std::endl;
            glutPostRedisplay();
            break;
        case 'n':
            scene.free_param_n -= 0.5;
            std::cout << "free_param_n=" << scene.free_param_n << std::endl;
            glutPostRedisplay();
            break;
        case 'S':
            scene.light["sun0"]->position = glm::vec4(scene.camera->rotation(15.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec3(scene.light["sun0"]->position), 0.0f);
            std::cout << "sun_position=" << "(" << scene.light["sun0"]->position.x << "," << scene.light["sun0"]->position.y << "," << scene.light["sun0"]->position.z << ")" << std::endl;
            glutPostRedisplay();
            break;
        case 's':
            scene.light["sun0"]->position = glm::vec4(scene.camera->rotation(-15.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec3(scene.light["sun0"]->position), 0.0f);
            std::cout << "sun_position=" << "(" << scene.light["sun0"]->position.x << "," << scene.light["sun0"]->position.y << "," << scene.light["sun0"]->position.z << ")" << std::endl;
            glutPostRedisplay();
            break;
        default:
            glutPostRedisplay();
            break;
    }
}

void specialKey(int key, int x, int y){
    switch (key) {
        case GLUT_KEY_UP: // up
            scene.camera -> rotateUp(-10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_DOWN: // down
            scene.camera -> rotateUp(10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_RIGHT: // right
            scene.camera -> rotateRight(-10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_LEFT: // left
            scene.camera -> rotateRight(10.0f);
            glutPostRedisplay();
            break;
    }
}

int main(int argc, char** argv)
{
    // BEGIN CREATE WINDOW
    glutInit(&argc, argv);
    
#ifdef __APPLE__
    glutInitDisplayMode( GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
#else
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
#endif
    glutInitWindowSize(width, height);
    glutCreateWindow(title);
#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    GLenum err = glewInit() ;
    if (GLEW_OK != err) {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    }
#endif
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    // END CREATE WINDOW
    
    initialize();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    
    glutMainLoop();
	return 0;   /* ANSI C requires main to return int. */
}
