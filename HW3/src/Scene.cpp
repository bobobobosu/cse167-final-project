/**************************************************
Scene.cpp contains the implementation of the draw command
*****************************************************/
#include "Scene.h"
#include "Cube.h"
#include "Obj.h"

// The scene init definition 
#include "Scene.inl"


using namespace glm;
void Scene::draw(void) {
    // Pre-draw sequence: assign uniforms that are the same for all Geometry::draw call.  These uniforms include the camera view, proj, and the lights.  These uniform do not include modelview and material parameters.
    camera->computeMatrices();
    surfaceShader->view = camera->view;
    surfaceShader->projection = camera->proj;
    surfaceShader->nlights = light.size();
    surfaceShader->lightpositions.resize(surfaceShader->nlights);
    surfaceShader->lightcolors.resize(surfaceShader->nlights);
    int count = 0;
    for (std::pair<std::string, Light*> entry : light) {
        surfaceShader->lightpositions[count] = (entry.second)->position;
        surfaceShader->lightcolors[count] = (entry.second)->color;
        count++;
    }

    //depthShader->view = camera->view;
    //depthShader->projection = camera->proj;

    // Define stacks for depth-first search (DFS)
    std::stack < Node* > dfs_stack;
    std::stack < mat4 >  matrix_stack; // HW3: You will update this matrix_stack during the depth-first search while loop.

    // Initialize the current state variable for DFS
    Node* cur = node["world"]; // root of the tree
    mat4 cur_VM = camera->view; // HW3: You will update this current modelview during the depth first search.  Initially, we are at the "world" node, whose modelview matrix is just camera's view matrix.

    // HW3: The following is the beginning of the depth-first search algorithm.
    // HW3: The depth-first search for the node traversal has already been implemented (cur, dfs_stack).
    // HW3: All you have to do is to also update the states of (cur_VM, matrix_stack) alongside the traversal.  You will only need to modify starting from this line.
    dfs_stack.push(cur);
    matrix_stack.push(cur_VM);
    while (!dfs_stack.empty()) {

        // top-pop the stacks
        cur = dfs_stack.top();        dfs_stack.pop();
        // (HW3 hint: you should do something here)
        cur_VM = matrix_stack.top();  matrix_stack.pop();


        // draw all the models at the current node
        for (unsigned int i = 0; i < cur->models.size(); i++) {
            // Prepare to draw the geometry. Assign the modelview and the material.

            // (HW3 hint: you should do something here)
            surfaceShader->modelview = cur_VM * (cur->modeltransforms[i]); // HW3: Without updating cur_VM, modelview would just be camera's view matrix.
            surfaceShader->material = (cur->models[i])->material;
            //depthShader->modelview = cur_VM * (cur->modeltransforms[i]); // HW3: Without updating cur_VM, modelview would just be camera's view matrix.
            // The draw command
            surfaceShader->setUniforms();
            //depthShader->setUniforms();
            (cur->models[i])->geometry->draw();
        }

        // Continue the DFS: put all the child nodes of the current node in the stack
        for (unsigned int i = 0; i < cur->childnodes.size(); i++) {
            dfs_stack.push(cur->childnodes[i]);
            // (HW3 hint: you should do something here)
            matrix_stack.push(cur_VM * cur->childtransforms[i]);
        }

    } // End of DFS while loop.
    // HW3: Your code will only be above this line.

}


