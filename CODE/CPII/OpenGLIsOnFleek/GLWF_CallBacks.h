#pragma once
#include "OpenGLCommon.h"
#include <vector>
#include "cMesh.h"

extern int objectToMove;
extern std::vector< cMesh* > g_vec_pMeshesToDraw;
// Returns NULL if not found
cMesh* g_pFindMeshByFriendlyName(std::string friendlyNameToFind)
{
    for (unsigned int index = 0; index != ::g_vec_pMeshesToDraw.size(); index++)
    {
        if (::g_vec_pMeshesToDraw[index]->friendlyName == friendlyNameToFind)
        {
            // Found it
            return ::g_vec_pMeshesToDraw[index];
        }
    }
    // Didn't find it
    return NULL;
}

inline void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    const float CAMERA_MOVEMENT_SPEED = 1.0f;
    const float OBJECT_MOVEMENT_SPEED = 0.10f;
    const float LIGHT_MOVEMENT_SPEED = 1.0f;
    int maxCounter = g_vec_pMeshesToDraw.size() - 1;

    // Nothing down
    if (mods == 0)
    {

        //object selector
        if (key == GLFW_KEY_MINUS && action)
        {
            if (objectToMove - 1 >= 0)
            {
                objectToMove--;
                std::cout << "-" << objectToMove << std::endl;
            }
        }

        if (key == GLFW_KEY_EQUAL && action)
        {
            if (objectToMove < maxCounter)
            {
                objectToMove++;
                std::cout << "+" << objectToMove << std::endl;
            }
        }


        if (key == GLFW_KEY_A && action)
        {
            cMesh* temp = g_pFindMeshByFriendlyName("fred");//player
            temp->directionChange(1, temp);//left

            g_vec_pMeshesToDraw[objectToMove]->drawPosition.x -= CAMERA_MOVEMENT_SPEED;
            /*g_vec_pMeshesToDraw[1]->drawPosition.x += 0.05f;*/
            // std::cout << "x-: " << g_vec_pMeshesToDraw[objectToMove]->drawPosition.x << std::endl;

        }
        if (key == GLFW_KEY_D && action)
        {
            cMesh* temp = g_pFindMeshByFriendlyName("fred");//player
            temp->directionChange(2, temp);//left
            g_vec_pMeshesToDraw[objectToMove]->drawPosition.x += CAMERA_MOVEMENT_SPEED;
            //std::cout << "x+: " << g_vec_pMeshesToDraw[0]->drawPosition.x << std::endl;

        }

        if (key == GLFW_KEY_W && action)
        {
            cMesh* temp = g_pFindMeshByFriendlyName("fred");//player
            temp->directionChange(3, temp);//left
            g_vec_pMeshesToDraw[objectToMove]->drawPosition.y += CAMERA_MOVEMENT_SPEED;
            //std::cout << "z-: " << g_vec_pMeshesToDraw[objectToMove]->drawPosition.z << std::endl;
        }
        if (key == GLFW_KEY_S && action) //down
        {
            cMesh* temp = g_pFindMeshByFriendlyName("fred");//player
            temp->directionChange(0, temp);//down

            g_vec_pMeshesToDraw[objectToMove]->drawPosition.y -= CAMERA_MOVEMENT_SPEED;
            //std::cout << "z+: " << g_vec_pMeshesToDraw[0]->drawPosition.z << std::endl;

        }



    }

    return;
}

