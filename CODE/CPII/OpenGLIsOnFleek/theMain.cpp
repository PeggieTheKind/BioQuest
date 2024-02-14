//https://www.glfw.org/docs/latest/quick.html
//http://graphics.stanford.edu/data/3Dscanrep/
#include "OpenGLCommon.h"
#include "cGlobal.h"
#include <iostream>
#include <fstream>      // C++ file IO (secret: it's a wraper for the c IO)
#include <sstream>      // like a string builder
#include <vector>       // Aka a "smart array"
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "Basic Shader Manager/cShaderManager.h"
#include "cVAOManager/cVAOManager.h"
#include "GLWF_CallBacks.h" // keyboard and mouse input
#include "cMesh.h"
#include "cPhysics.h"
#include "cLightManager.h"
#include "cLightHelper.h"
#include "TextureManager/cBasicTextureManager.h"
#include "cHiResTimer.h"
#include "cCommand_MoveTo.h"
#include <algorithm>
#include "txtReading.h"

//camera stuff
glm::vec3 g_cameraEye = glm::vec3(0.0, 20.0, 100.0f);
glm::vec3 g_cameraTarget = glm::vec3(0.0f, 5.0f, 0.0f);
glm::vec3 g_upVector = glm::vec3(0.0f, 1.0f, 0.0f);
int objectToMove = 0;

//managers
cVAOManager* g_pMeshManager = NULL;
cBasicTextureManager* g_pTextureManager = NULL;

//debugmesh stuffs
cMesh* g_pDebugSphereMesh = NULL;
// Used by g_DrawDebugSphere()
GLuint g_DebugSphereMesh_shaderProgramID = 0;

//vectors for frame times
std::vector<double> g_vecLastFrameTimes;

// This is the list of meshes                                                           
std::vector< cMesh* > g_vec_pMeshesToDraw;

// This is the list of physical properties 
cPhysics* g_pPhysics = NULL;

// Returns NULL if not found
cMesh* g_pFindMeshByFriendlyName(std::string friendlyNameToFind);

//lights
cLightManager* g_pTheLights = NULL;

//animations
std::vector<cCommand_MoveTo> g_vecAnimationCommands;

//selectors
int g_selectedMesh = 0;
int g_selectedLight = 0;

// Function signature
bool SaveVectorSceneToFile(std::string saveFileName);
void DrawObject(cMesh* pCurrentMesh, glm::mat4 matModel, GLuint shaderProgramID);
cMesh* g_pFindMeshByFriendlyName(std::string friendlyNameToFind);
void DrawLightDebugSpheres(glm::mat4 matProjection, glm::mat4 matView,GLuint shaderProgramID);
float getRandomFloat(float a, float b);
glm::vec3 LoadAllTheModels(std::string sceneFileName,
    cVAOManager* pVAOManager,
    unsigned int shaderProgramID,
    std::string& error);
bool consoleOutputOfScene(void);
bool LoadModels();


// HACK:
float g_HeightAdjust = 10.0f;
glm::vec2 g_UVOffset = glm::vec2(0.0f, 0.0f);

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}


int main(void)
{
    //attempt load from json
    textReading* temp = new textReading();
    temp->readTxt();
    
    cMesh bob;

    std::cout << "About to blow you mind with OpenGL!" << std::endl;

    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    cHiResTimer* p_HRTimer = new cHiResTimer(60);

    cShaderManager* pShaderThing = new cShaderManager();
    pShaderThing->setBasePath("assets/shaders");

    cShaderManager::cShader vertexShader;
    vertexShader.fileName = "vertexShader01.glsl";

    // Add a geometry shader
    cShaderManager::cShader geometryShader;
    geometryShader.fileName = "geometryPassThrough.glsl";

    cShaderManager::cShader fragmentShader;
    fragmentShader.fileName = "fragmentShader01.glsl";

    if ( ! pShaderThing->createProgramFromFile("shader01", vertexShader, geometryShader, fragmentShader ) )
    {
        std::cout << "Error: Couldn't compile or link:" << std::endl;
        std::cout << pShaderThing->getLastError();
        return -1;
    }


//
    GLuint shaderProgramID = pShaderThing->getIDFromFriendlyName("shader01");

    // Set the debug shader ID we're going to use
    ::g_DebugSphereMesh_shaderProgramID = shaderProgramID;

    //VAO
    ::g_pMeshManager = new cVAOManager();
  
    ::g_pMeshManager->setBasePath("assets/models");

    sModelDrawInfo sphereDrawingInfo;
    //    ::g_pMeshManager->LoadModelIntoVAO("Sphere_1_unit_Radius.ply",
    ::g_pMeshManager->LoadModelIntoVAO("Flat_Grid_100x100.ply",
        sphereDrawingInfo, shaderProgramID);
    std::cout << "Loaded: " << sphereDrawingInfo.numberOfVertices << " vertices" << std::endl;

    sModelDrawInfo danbo;
    //    ::g_pMeshManager->LoadModelIntoVAO("Sphere_1_unit_Radius.ply",
    ::g_pMeshManager->LoadModelIntoVAO("danbo.ply",
        danbo, shaderProgramID);
    std::cout << "Loaded: " << danbo.numberOfVertices << " vertices" << std::endl;

    //Terrain_xyz_n_rgba.ply
    sModelDrawInfo map;
    //    ::g_pMeshManager->LoadModelIntoVAO("Sphere_1_unit_Radius.ply",
    ::g_pMeshManager->LoadModelIntoVAO("map.ply",
        map, shaderProgramID);
    std::cout << "Loaded: " << map.numberOfVertices << " vertices" << std::endl;

    //Flat_1x1_plane.ply
    sModelDrawInfo floorModel;
    //    ::g_pMeshManager->LoadModelIntoVAO("Sphere_1_unit_Radius.ply",
    ::g_pMeshManager->LoadModelIntoVAO("Sphere_1_unit_Radius_xyz_n_rgba_uv.ply",
        floorModel, shaderProgramID);
    std::cout << "Loaded: " << floorModel.numberOfVertices << " vertices" << std::endl;
    // ... and so on
    ::g_pTextureManager = new cBasicTextureManager();

    ::g_pTextureManager->SetBasePath("assets/textures"); 
    //2d texture from BMP
    ::g_pTextureManager->Create2DTextureFromBMPFile("Blank_UV_Text_Texture.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("SpidermanUV_square.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("Water_Texture_01.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("taylor-swift-jimmy-fallon.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("table.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("bear.bmp", true);
    ::g_pTextureManager->Create2DTextureFromBMPFile("floorTexture.bmp", true);

    // Load the HUGE height map
    ::g_pTextureManager->Create2DTextureFromBMPFile("NvF5e_height_map.bmp", true);
    
    // Using this for discard transparency mask
    ::g_pTextureManager->Create2DTextureFromBMPFile("FAKE_Stencil_Texture_612x612.bmp", true);

    // Load a cube map
    ::g_pTextureManager->SetBasePath("assets/textures/CubeMaps");
    std::string errors;

    //sunny day view
    ::g_pTextureManager->CreateCubeTextureFromBMPFiles("SunnyDay",
                                                       "TropicalSunnyDayLeft2048.bmp",
                                                       "TropicalSunnyDayRight2048.bmp",
                                                       "TropicalSunnyDayUp2048.bmp",
                                                       "TropicalSunnyDayDown2048.bmp",
                                                       "TropicalSunnyDayFront2048.bmp",
                                                       "TropicalSunnyDayBack2048.bmp",
                                                       true,
                                                       errors);

    //models
    //map for discard
    cMesh* mapOf = new cMesh();
    mapOf->meshName = "map.ply";
    mapOf->drawPosition.y = 0.0f;
    mapOf->drawPosition.z = 20.0f;
    mapOf->setUniformDrawScale(.2);
    mapOf->friendlyName = "map";
    //mapOf->bIsWireframe = true;
    mapOf->alpha_trans = 0.1;
    mapOf->textureName[1] = "Water_Texture_01.bmp";

    //mapOf->bUseDebugColours;
    //mapOf->wholeObjectDebugColourRGBA.g = 120.0;
    ::g_vec_pMeshesToDraw.push_back(mapOf);

    //make many danbos
    for (int i = 1; i < 20; i++)
    {
       
        //models
        cMesh* pDanbo = new cMesh();
        pDanbo->meshName = "danbo.ply";
        pDanbo->drawPosition.y = 0.0f;
        pDanbo->drawPosition.z = 0.0f;
        pDanbo->setUniformDrawScale(2.0f);
        pDanbo->friendlyName = "danbo";
        pDanbo->textureName[0] = "taylor-swift-jimmy-fallon.bmp";
        pDanbo->textureRatios[0] = 0.6f;
        pDanbo->textureName[1] = "Water_Texture_01.bmp";
        pDanbo->textureRatios[1] = 0.4f;
        pDanbo->drawPosition.x = i * 5;
        ::g_vec_pMeshesToDraw.push_back(pDanbo);

    }

    for (int i = 1; i < 20; i++)
    {

        //models
        cMesh* pDanbo = new cMesh();
        pDanbo->meshName = "danbo.ply";
        pDanbo->drawPosition.y = 0.0f;
        pDanbo->drawPosition.z = 0.0f;
        pDanbo->setUniformDrawScale(2.0f);
        pDanbo->friendlyName = "danbo";
        pDanbo->textureName[0] = "taylor-swift-jimmy-fallon.bmp";
        pDanbo->textureRatios[0] = 0.6f;
        pDanbo->textureName[1] = "Water_Texture_01.bmp";
        pDanbo->textureRatios[1] = 0.4f;
        pDanbo->drawPosition.x = i * - 5;
        ::g_vec_pMeshesToDraw.push_back(pDanbo);

    }


    //models
    /*cMesh* pDanbo1 = new cMesh();
    pDanbo1->meshName = "danbo.ply";
    pDanbo1->drawPosition.x = 10.0f;
    pDanbo1->drawPosition.y = 0.0f;
    pDanbo1->drawPosition.z = 0.0f;
    pDanbo1->setUniformDrawScale(2.0f);
    pDanbo1->friendlyName = "danbo";
    pDanbo1->textureName[0] = "taylor-swift-jimmy-fallon.bmp";
    pDanbo1->textureRatios[0] = 0.6f;
    pDanbo1->textureName[1] = "Water_Texture_01.bmp";
    pDanbo1->textureRatios[1] = 0.4f;*/


   // pDanbo1->bUseDebugColours = true;
   /* cMesh* pDanbo = new cMesh();
    pDanbo->meshName = "danbo.ply";
    pDanbo->drawPosition.x = 0.0f;
    pDanbo->drawPosition.y = 0.0f;
    pDanbo->drawPosition.z = 0.0f;
    pDanbo->setUniformDrawScale(2.0f);
    pDanbo->friendlyName = "danbo1";
    pDanbo->textureName[0] = "taylor-swift-jimmy-fallon.bmp";
    pDanbo->textureRatios[0] = 0.6f;
    pDanbo->textureName[1] = "Water_Texture_01.bmp";
    pDanbo->textureRatios[1] = 0.4f;*/
    //::g_vec_pMeshesToDraw.push_back(pDanbo1);

    //pDanbo1->alpha_trans = 0.5;
    //trans add 
    /*::g_vec_pMeshesToDraw.push_back(pDanbo1);

    ::g_vec_pMeshesToDraw.push_back(pDanbo);*/

    //floors
   
    cMesh* floor = new cMesh();
    floor->meshName = "Flat_Grid_100x100.ply";
    floor->friendlyName = "floor";
    floor->setUniformDrawScale(0.3);
    floor->textureName[0] = "floorTexture.bmp";
    floor->textureRatios[0] = 1.0f;
    floor->bIsVisible = true;
    floor->drawPosition = glm::vec3(0, -2, 0);
     g_vec_pMeshesToDraw.push_back(floor);


    //::g_vec_pMeshesToDraw.push_back(pDanbo1);

    //LIGHTS
    ::g_pTheLights = new cLightManager();
    
    ::g_pTheLights->SetUniformLocations(shaderProgramID);
    ::g_pTheLights->theLights[0].param2.x = 1.0f;   // Turn on
    ::g_pTheLights->theLights[0].param1.x = 0.0f;   // 1 = spot light
    ::g_pTheLights->theLights[0].atten.x = 0.0f;        // Constant attenuation
    ::g_pTheLights->theLights[0].atten.y = 0.001f;        // Linear attenuation
    ::g_pTheLights->theLights[0].atten.z = 0.001f;        // Quadratic attenuation
    ::g_pTheLights->theLights[0].position.x = 0.0f;
    ::g_pTheLights->theLights[0].position.y = 40.0f;
    ::g_pTheLights->theLights[0].position.z = -40.0f;


    float yaxisRotation = 0.0f;

    double lastTime = glfwGetTime();

    //SORT BEFORE based on XYZ
    std::sort(g_vec_pMeshesToDraw.begin(), g_vec_pMeshesToDraw.end(), [](cMesh* &pos1, cMesh* &pos2) {return pos1->drawPosition.x < pos1->drawPosition.x; });
    std::sort(g_vec_pMeshesToDraw.begin(), g_vec_pMeshesToDraw.end(), [](cMesh*& pos1, cMesh*& pos2) {return pos1->drawPosition.y < pos1->drawPosition.y; });
    std::sort(g_vec_pMeshesToDraw.begin(), g_vec_pMeshesToDraw.end(), [](cMesh*& pos1, cMesh*& pos2) {return pos1->drawPosition.z < pos1->drawPosition.z; });




    while (!glfwWindowShouldClose(window))
    {

        // Switch the "main" shader
        shaderProgramID = pShaderThing->getIDFromFriendlyName("shader01");
        glUseProgram(shaderProgramID);

        float ratio;
        int width, height;

        glUseProgram(shaderProgramID);

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // While drawing a pixel, see if the pixel that's already there is closer or not?
        glEnable(GL_DEPTH_TEST);

        // (Usually) the default - does NOT draw "back facing" triangles
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
// *****************************************************************

        ::g_pTheLights->UpdateUniformValues(shaderProgramID);

// *****************************************************************
        //uniform vec4 eyeLocation;

        //double check camera
        // Point the spotlight at the bathtub
       /* cMesh* pTemp = g_pFindMeshByFriendlyName("danbo");
        if (pTemp)
        {
            glm::vec3 bathTubToLightRay = pTemp->drawPosition - glm::vec3(::g_pTheLights->theLights[0].position);

            bathTubToLightRay = glm::normalize(bathTubToLightRay);

            ::g_pTheLights->theLights[0].direction = glm::vec4(bathTubToLightRay, 1.0f);
        }*/

        GLint eyeLocation_UL = glGetUniformLocation(shaderProgramID, "eyeLocation");
        glUniform4f(eyeLocation_UL,
                    ::g_cameraEye.x, ::g_cameraEye.y, ::g_cameraEye.z, 1.0f);



//       //mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        glm::mat4 matProjection = glm::perspective(0.6f,
                                                   ratio,
                                                   0.1f,        // Near (as big)
                                                   10'000.0f);    // Far (as small)

        glm::mat4 matView = glm::lookAt(::g_cameraEye, ::g_cameraTarget,::g_upVector);
       
        GLint matProjection_UL = glGetUniformLocation(shaderProgramID, "matProjection");
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, glm::value_ptr(matProjection));

        GLint matView_UL = glGetUniformLocation(shaderProgramID, "matView");
        glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(matView));

        //move based on acc
        const float CAMERA_MOVEMENT_SPEED = 1.0f;

        
        
        // *********************************************************************
        // Draw all the objects
        for ( unsigned int index = 0; index != ::g_vec_pMeshesToDraw.size(); index++ )
        {
            cMesh* pCurrentMesh = ::g_vec_pMeshesToDraw[index];

            if (pCurrentMesh->bIsVisible)
            {
                
                glm::mat4 matModel = glm::mat4(1.0f);   // Identity matrix
                if (pCurrentMesh->friendlyName == "danbo")
                {
                
                glm::vec3 m_rayFromStartToEnd = g_cameraEye - g_vec_pMeshesToDraw[index]->drawPosition;
                glm::vec3 m_direction = glm::normalize(m_rayFromStartToEnd);
                pCurrentMesh->setDrawOrientation(glm::quatLookAt(m_direction, g_upVector));
                }
                   // std::cout << pCurrentMesh->drawPosition.x << std::endl;
                
                DrawObject(pCurrentMesh, matModel, shaderProgramID);
            }

        }

        // Draw the skybox
        {
            // HACK: I'm making this here, but hey...
            cMesh theSkyBox;
            theSkyBox.meshName = "Sphere_1_unit_Radius_xyz_n_rgba_uv.ply";
            theSkyBox.setUniformDrawScale(10.0f);

            theSkyBox.setUniformDrawScale(5'000.0f);
            theSkyBox.setDrawPosition(::g_cameraEye);
//            theSkyBox.bIsWireframe = true;

            // Depth test
//            glDisable(GL_DEPTH_TEST);       // Writes no matter what
            // Write to depth buffer (depth mask)
//            glDepthMask(GL_FALSE);          // Won't write to the depth buffer
            
            // uniform bool bIsSkyBox;
            GLint bIsSkyBox_UL = glGetUniformLocation(shaderProgramID, "bIsSkyBox");
            glUniform1f(bIsSkyBox_UL, (GLfloat) GL_TRUE);

            // The normals for this sphere are facing "out" but we are inside the sphere
            glCullFace(GL_FRONT);

            DrawObject(&theSkyBox, glm::mat4(1.0f), shaderProgramID);

            glUniform1f(bIsSkyBox_UL, (GLfloat)GL_FALSE);

            // Put the culling back to "normal" (back facing are not drawn)
            glCullFace(GL_BACK);
        }

        //delta time
        double deltaTime = p_HRTimer->getFrameTime();

     
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Update the title screen
        std::stringstream ssTitle;
        ssTitle << "Camera (x,y,z): "
            << ::g_cameraEye.x << ", "
            << ::g_cameraEye.y << ", "
            << ::g_cameraEye.z << ") Pos #" << objectToMove << ":("
            << ::g_vec_pMeshesToDraw[objectToMove]->drawPosition.x << ", "
            << ::g_vec_pMeshesToDraw[objectToMove]->drawPosition.y << ", "
            << ::g_vec_pMeshesToDraw[objectToMove]->drawPosition.z << ") ";

        std::string theTitle = ssTitle.str();

        glfwSetWindowTitle(window, theTitle.c_str() );


    }

    // Delete everything
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

// Returns NULL if not found
cMesh* g_pFindMeshByFriendlyName(std::string friendlyNameToFind)
{
    for ( unsigned int index = 0; index != ::g_vec_pMeshesToDraw.size(); index++ )
    {
        if ( ::g_vec_pMeshesToDraw[index]->friendlyName == friendlyNameToFind )
        {
            // Found it
            return ::g_vec_pMeshesToDraw[index];
        }
    }
    // Didn't find it
    return NULL;
}


void DrawLightDebugSpheres(glm::mat4 matProjection, glm::mat4 matView,
                           GLuint shaderProgramID)
{
    if ( ! ::g_drawDebugLightSpheres )
    {
        return;
    }

    // Draw concentric spheres to indicate light position and attenuation

    // Small white sphere where the light is
    ::g_DrawDebugSphere(::g_pTheLights->theLights[g_selectedLight].position,
                        0.5f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    cLightHelper lightHelper;

    // vec4 atten;		// x = constant, y = linear, z = quadratic, w = DistanceCutOff
    float constantAtten = ::g_pTheLights->theLights[g_selectedLight].atten.x;
    float linearAtten = ::g_pTheLights->theLights[g_selectedLight].atten.y;
    float quadAtten = ::g_pTheLights->theLights[g_selectedLight].atten.z;


    // Draw a red sphere at 75% brightness
    {
        float distAt75Percent = lightHelper.calcApproxDistFromAtten(0.75f, 0.01f, 100000.0f,
                                                                    constantAtten, linearAtten, quadAtten, 50);

        ::g_DrawDebugSphere(::g_pTheLights->theLights[g_selectedLight].position,
                            distAt75Percent, 
                            glm::vec4(0.5f, 0.0f, 0.0f, 1.0f));
    }


    // Draw a green sphere at 50% brightness
    {
        float distAt50Percent = lightHelper.calcApproxDistFromAtten(0.50f, 0.01f, 100000.0f,
                                                                    constantAtten, linearAtten, quadAtten, 50);

        ::g_DrawDebugSphere(::g_pTheLights->theLights[g_selectedLight].position,
                            distAt50Percent,
                            glm::vec4(0.0f, 0.5f, 0.0f, 1.0f));
    }

    // Draw a yellow? sphere at 25% brightness
    {
        float distAt25Percent = lightHelper.calcApproxDistFromAtten(0.25f, 0.01f, 100000.0f,
                                                                    constantAtten, linearAtten, quadAtten, 50);

        ::g_DrawDebugSphere(::g_pTheLights->theLights[g_selectedLight].position,
                            distAt25Percent,
                            glm::vec4(0.50f, 0.5f, 0.0f, 1.0f));
    }

    // Draw a blue sphere at 1% brightness
    {
        float distAt_5Percent = lightHelper.calcApproxDistFromAtten(0.01f, 0.01f, 100000.0f,
                                                                    constantAtten, linearAtten, quadAtten, 50);

        ::g_DrawDebugSphere(::g_pTheLights->theLights[g_selectedLight].position,
                            distAt_5Percent,
                            glm::vec4(0.0f, 0.0f, 0.5f, 1.0f));
    }


    return;
}

void SetUpTextures(cMesh* pCurrentMesh, GLuint shaderProgramID)
{
//    GLuint Texture00 = ::g_pTextureManager->getTextureIDFromName(pCurrentMesh->textureName[0]);
//    if (Texture00 == 0)
//    {
//        Texture00 = ::g_pTextureManager->getTextureIDFromName("Blank_UV_Text_Texture.bmp");
//    }
//
//    // We are just going to pick texture unit 5 (for no reason, just as an example)
//    //    glActiveTexture(GL_TEXTURE5);       // #5 TEXTURE UNIT
//    glActiveTexture(GL_TEXTURE0 + 5);       // #5 TEXTURE UNIT
//    glBindTexture(GL_TEXTURE_2D, Texture00);
//
//    //uniform sampler2D texture_00;
//    GLint texture_00_UL = glGetUniformLocation(shaderProgramID, "texture_00");
//    glUniform1i(texture_00_UL, 5);     // <- 5, an integer, because it's "Texture Unit #5"
//    // ***************************************************************

//    uniform sampler2D texture_00;			// 2D meaning x,y or s,t or u,v
//    uniform sampler2D texture_01;
//    uniform sampler2D texture_02;
//    uniform sampler2D texture_03;
//    uniform sampler2D texture_04;			// 2D meaning x,y or s,t or u,v
//    uniform sampler2D texture_05;
//    uniform sampler2D texture_06;
//    uniform sampler2D texture_07;
//    //... and so on
//    //uniform float textureMixRatio[8];
//    uniform vec4 textureMixRatio_0_3;
//    uniform vec4 textureMixRatio_4_7;


    {
        GLint textureUnitNumber = 0;
        GLuint Texture00 = ::g_pTextureManager->getTextureIDFromName(pCurrentMesh->textureName[textureUnitNumber]);
        glActiveTexture(GL_TEXTURE0 + textureUnitNumber);
        glBindTexture(GL_TEXTURE_2D, Texture00);
        GLint texture_00_UL = glGetUniformLocation(shaderProgramID, "texture_00");
        glUniform1i(texture_00_UL, textureUnitNumber);
    }

    {
        GLint textureUnitNumber = 1;
        GLuint Texture01 = ::g_pTextureManager->getTextureIDFromName(pCurrentMesh->textureName[textureUnitNumber]);
        glActiveTexture(GL_TEXTURE0 + textureUnitNumber);
        glBindTexture(GL_TEXTURE_2D, Texture01);
        GLint texture_01_UL = glGetUniformLocation(shaderProgramID, "texture_01");
        glUniform1i(texture_01_UL, textureUnitNumber);
    }

    {
        GLint textureUnitNumber = 2;
        GLuint Texture02 = ::g_pTextureManager->getTextureIDFromName(pCurrentMesh->textureName[textureUnitNumber]);
        glActiveTexture(GL_TEXTURE0 + textureUnitNumber);
        glBindTexture(GL_TEXTURE_2D, Texture02);
        GLint texture_02_UL = glGetUniformLocation(shaderProgramID, "texture_02");
        glUniform1i(texture_02_UL, textureUnitNumber);
    }

    {
        GLint textureUnitNumber = 3;
        GLuint Texture03 = ::g_pTextureManager->getTextureIDFromName(pCurrentMesh->textureName[textureUnitNumber]);
        glActiveTexture(GL_TEXTURE0 + textureUnitNumber);
        glBindTexture(GL_TEXTURE_2D, Texture03);
        GLint texture_03_UL = glGetUniformLocation(shaderProgramID, "texture_03");
        glUniform1i(texture_03_UL, textureUnitNumber);
    }    
    // and so on to however many texture you are using

//    uniform vec4 textureMixRatio_0_3;
//    uniform vec4 textureMixRatio_4_7;

    GLint textureMixRatio_0_3_UL = glGetUniformLocation(shaderProgramID, "textureMixRatio_0_3");
//    GLint textureMixRatio_4_7_UL = glGetUniformLocation(shaderProgramID, "textureMixRatio_4_7");

    glUniform4f(textureMixRatio_0_3_UL,
                pCurrentMesh->textureRatios[0],
                pCurrentMesh->textureRatios[1],
                pCurrentMesh->textureRatios[2],
                pCurrentMesh->textureRatios[3]);
//    glUniform4f(textureMixRatio_4_7_UL,
//                pCurrentMesh->textureRatios[4],
//                pCurrentMesh->textureRatios[5],
//                pCurrentMesh->textureRatios[6],
//                pCurrentMesh->textureRatios[7]);


    // Also set up the height map and discard texture

    {
        // uniform sampler2D heightMapSampler;		// Texture unit 20
        GLint textureUnitNumber = 20;
        GLuint Texture20 = ::g_pTextureManager->getTextureIDFromName("NvF5e_height_map.bmp");
        glActiveTexture(GL_TEXTURE0 + textureUnitNumber);
        glBindTexture(GL_TEXTURE_2D, Texture20);
        GLint texture_20_UL = glGetUniformLocation(shaderProgramID, "heightMapSampler");
        glUniform1i(texture_20_UL, textureUnitNumber);
    }    



    // Set up a skybox
    {
        // uniform samplerCube skyBoxTexture;		// Texture unit 30
        GLint textureUnit30 = 30;
        GLuint skyBoxID = ::g_pTextureManager->getTextureIDFromName("SunnyDay");
        glActiveTexture(GL_TEXTURE0 + textureUnit30);
        // NOTE: Binding is NOT to GL_TEXTURE_2D
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxID);
        GLint skyBoxSampler_UL = glGetUniformLocation(shaderProgramID, "skyBoxTexture");
        glUniform1i(skyBoxSampler_UL, textureUnit30);
    }

    
    return;
}

void DrawObject(cMesh* pCurrentMesh, glm::mat4 matModelParent, GLuint shaderProgramID)
{

    //         mat4x4_identity(m);
    glm::mat4 matModel = matModelParent;




    // Translation
    glm::mat4 matTranslate = glm::translate(glm::mat4(1.0f),
        glm::vec3(pCurrentMesh->drawPosition.x,
            pCurrentMesh->drawPosition.y,
            pCurrentMesh->drawPosition.z));


    // Rotation matrix generation
//    glm::mat4 matRotateX = glm::rotate(glm::mat4(1.0f),
//                                       pCurrentMesh->drawOrientation.x, // (float)glfwGetTime(),
//                                       glm::vec3(1.0f, 0.0, 0.0f));
//
//
//    glm::mat4 matRotateY = glm::rotate(glm::mat4(1.0f),
//                                       pCurrentMesh->drawOrientation.y, // (float)glfwGetTime(),
//                                       glm::vec3(0.0f, 1.0, 0.0f));
//
//    glm::mat4 matRotateZ = glm::rotate(glm::mat4(1.0f),
//                                       pCurrentMesh->drawOrientation.z, // (float)glfwGetTime(),
//                                       glm::vec3(0.0f, 0.0, 1.0f));

    // Now we are all bougie, using quaternions
    glm::mat4 matRotation = glm::mat4(pCurrentMesh->get_qOrientation());


    // Scaling matrix
    glm::mat4 matScale = glm::scale(glm::mat4(1.0f),
        glm::vec3(pCurrentMesh->drawScale.x,
            pCurrentMesh->drawScale.y,
            pCurrentMesh->drawScale.z));
    //--------------------------------------------------------------

    // Combine all these transformation
    matModel = matModel * matTranslate;         // Done last

    //    matModel = matModel * matRotateX;
    //    matModel = matModel * matRotateY;
    //    matModel = matModel * matRotateZ;
        //
    matModel = matModel * matRotation;


    matModel = matModel * matScale;             // Mathematically done 1st

    //        m = m * rotateZ;
    //        m = m * rotateY;
    //        m = m * rotateZ;



       //mat4x4_mul(mvp, p, m);
    //    glm::mat4 mvp = matProjection * matView * matModel;

    //    GLint mvp_location = glGetUniformLocation(shaderProgramID, "MVP");
    //    //glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
    //    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));

    GLint matModel_UL = glGetUniformLocation(shaderProgramID, "matModel");
    glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(matModel));


    // Also calculate and pass the "inverse transpose" for the model matrix
    glm::mat4 matModel_InverseTranspose = glm::inverse(glm::transpose(matModel));

    // uniform mat4 matModel_IT;
    GLint matModel_IT_UL = glGetUniformLocation(shaderProgramID, "matModel_IT");
    glUniformMatrix4fv(matModel_IT_UL, 1, GL_FALSE, glm::value_ptr(matModel_InverseTranspose));


    if (pCurrentMesh->bIsWireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    //        glPointSize(10.0f);


            // uniform bool bDoNotLight;
    GLint bDoNotLight_UL = glGetUniformLocation(shaderProgramID, "bDoNotLight");

    if (pCurrentMesh->bDoNotLight)
    {
        // Set uniform to true
        glUniform1f(bDoNotLight_UL, (GLfloat)GL_TRUE);
    }
    else
    {
        // Set uniform to false;
        glUniform1f(bDoNotLight_UL, (GLfloat)GL_FALSE);
    }

    //uniform bool bUseDebugColour;	
    GLint bUseDebugColour_UL = glGetUniformLocation(shaderProgramID, "bUseDebugColour");
    if (pCurrentMesh->bUseDebugColours)
    {
        glUniform1f(bUseDebugColour_UL, (GLfloat)GL_TRUE);
        //uniform vec4 debugColourRGBA;
        GLint debugColourRGBA_UL = glGetUniformLocation(shaderProgramID, "debugColourRGBA");
        glUniform4f(debugColourRGBA_UL,
            pCurrentMesh->wholeObjectDebugColourRGBA.r,
            pCurrentMesh->wholeObjectDebugColourRGBA.g,
            pCurrentMesh->wholeObjectDebugColourRGBA.b,
            pCurrentMesh->wholeObjectDebugColourRGBA.a);
    }
    else
    {
        glUniform1f(bUseDebugColour_UL, (GLfloat)GL_FALSE);
    }



    // FOR NOW, hardcode the texture settings

    // uniform bool bUseVertexColours;
    GLint bUseVertexColours_UL = glGetUniformLocation(shaderProgramID, "bUseVertexColours");
    glUniform1f(bUseVertexColours_UL, (GLfloat)GL_FALSE);



    SetUpTextures(pCurrentMesh, shaderProgramID);

    // *********************************************************************
        // Is this using the heigth map?
        // HACK:
    GLint bUseHeightMap_UL = glGetUniformLocation(shaderProgramID, "bUseHeightMap");
    // uniform bool bUseHeightMap;
    if (pCurrentMesh->friendlyName == "map")
    {
        glUniform1f(bUseHeightMap_UL, (GLfloat)GL_TRUE);

        //uniform float heightScale;
        GLint heightScale_UL = glGetUniformLocation(shaderProgramID, "heightScale");
        glUniform1f(heightScale_UL, ::g_HeightAdjust);

        //uniform vec2 UVOffset;
        GLint UVOffset_UL = glGetUniformLocation(shaderProgramID, "UVOffset");
        glUniform2f(UVOffset_UL, ::g_UVOffset.x, ::g_UVOffset.y);


    }
    else
    {
        glUniform1f(bUseHeightMap_UL, (GLfloat)GL_FALSE);
    }
    // *********************************************************************


    // *********************************************************************
        //  Discard texture example
        //    uniform bool bUseDiscardMaskTexture;
        //    uniform sampler2D maskSamplerTexture01;
    {
        GLint bUseDiscardMaskTexture_UL = glGetUniformLocation(shaderProgramID, "bUseDiscardMaskTexture");

        // uniform bool bUseHeightMap;
        if (pCurrentMesh->friendlyName == "Ground")
        {
            glUniform1f(bUseDiscardMaskTexture_UL, (GLfloat)GL_TRUE);

            //uniform sampler2D maskSamplerTexture01; 	// Texture unit 25
            GLint textureUnitNumber = 25;
            GLuint stencilMaskID = ::g_pTextureManager->getTextureIDFromName("FAKE_Stencil_Texture_612x612.bmp");
            glActiveTexture(GL_TEXTURE0 + textureUnitNumber);
            glBindTexture(GL_TEXTURE_2D, stencilMaskID);

            GLint bUseDiscardMaskTexture_UL = glGetUniformLocation(shaderProgramID, "maskSamplerTexture01");
            glUniform1i(bUseDiscardMaskTexture_UL, textureUnitNumber);

        }
        else
        {
            glUniform1f(bUseDiscardMaskTexture_UL, (GLfloat)GL_FALSE);
        }
    }
    // *********************************************************************

    //for trans: ALPHA
    if (pCurrentMesh->alpha_trans < 1.0f)
    {
        //trigger
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    }
    else
    {
        //turn off
        glDisable(GL_BLEND);

    }

    GLint transparencyAlpha = glGetUniformLocation(shaderProgramID, "transparencyAlpha");
    glUniform1f(transparencyAlpha, pCurrentMesh->alpha_trans);

    sModelDrawInfo modelInfo;
    if (::g_pMeshManager->FindDrawInfoByModelName(pCurrentMesh->meshName, modelInfo))
    {
        // Found it!!!

        glBindVertexArray(modelInfo.VAO_ID); 		//  enable VAO (and everything else)
        glDrawElements(GL_TRIANGLES,
            modelInfo.numberOfIndices,
            GL_UNSIGNED_INT,
            0);
        glBindVertexArray(0); 			            // disable VAO (and everything else)

    }

    // Are there any child meshes on this mesh
    // std::vector<cMesh*> vec_pChildMeshes;

    glm::mat4 matRemoveScaling = glm::scale(glm::mat4(1.0f),
        glm::vec3(
            1.0f / pCurrentMesh->drawScale.x,
            1.0f / pCurrentMesh->drawScale.y,
            1.0f / pCurrentMesh->drawScale.z));

    matModel = matModel * matRemoveScaling;

    for (cMesh* pChild : pCurrentMesh->vec_pChildMeshes)
    {

        // Notice we are passing the "parent" (already transformed) matrix
        // NOT an identiy matrix

        // if you are using scaling, you can "undo" the scaling
        // i.e. add the opposite of the scale the parent had



        DrawObject(pChild, matModel, shaderProgramID);

    }//for ( cMesh* pChild 



    return;
}

void g_DrawDebugSphere(glm::vec3 position, float scale, glm::vec4 colourRGBA)
{
    // Save the debug sphere state
    bool OLD_isVisible = ::g_pDebugSphereMesh->bIsVisible;
    glm::vec3 OLD_position = ::g_pDebugSphereMesh->drawPosition;
    glm::vec3 OLD_scale = ::g_pDebugSphereMesh->drawScale;
    glm::vec4 OLD_colours = ::g_pDebugSphereMesh->wholeObjectDebugColourRGBA;

    ::g_pDebugSphereMesh->bIsVisible = true;
    ::g_pDebugSphereMesh->drawPosition = position;
    ::g_pDebugSphereMesh->setUniformDrawScale(scale);
    ::g_pDebugSphereMesh->bDoNotLight = true;
    ::g_pDebugSphereMesh->bUseDebugColours = true;
    ::g_pDebugSphereMesh->wholeObjectDebugColourRGBA = colourRGBA;

   
    DrawObject(::g_pDebugSphereMesh, glm::mat4(1.0f), ::g_DebugSphereMesh_shaderProgramID);

    ::g_pDebugSphereMesh->bIsVisible = OLD_isVisible;
    ::g_pDebugSphereMesh->drawPosition = OLD_position;
    ::g_pDebugSphereMesh->drawScale = OLD_scale;
    ::g_pDebugSphereMesh->wholeObjectDebugColourRGBA = OLD_colours;

    return;
}

// https://stackoverflow.com/questions/5289613/generate-random-float-between-two-floats
float getRandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}
std::string modelName = "";

glm::vec3 LoadAllTheModels(std::string sceneFileName,
    cVAOManager* pVAOManager,
    unsigned int shaderProgramID,
    std::string& error)
{
    // Open the file that lists the models I'd like to load into the VAO
    std::ifstream sceneFile(sceneFileName.c_str());

    if (!sceneFile.is_open())
    {
        error = "Can't load file " + sceneFileName;
        exit(0);//quit 
    }

    int modelsLoaded = 0;
    int meshsCreated = 0;

    // File is open at this point
    std::string dumpster = "";

    //remove comment 
    sceneFile >> dumpster;

    //get camera pos
    float xCamera = 0;
    float yCamera = 0;
    float zCamera = 0;

    //get inputs
    sceneFile >> xCamera;
    sceneFile >> yCamera;
    sceneFile >> zCamera;

    //add to be returned
    glm::vec3 toBeReturned = glm::vec3(xCamera, yCamera, zCamera);

    //remove comment 
    sceneFile >> dumpster;
    //model types loaded
    do
    {
        //get file name
        std::string modelFileName;
        sceneFile >> modelFileName;

        if (modelFileName == "END") // LOOP TILL END
            break;

        //add info
        sModelDrawInfo drawingInfo;
        pVAOManager->LoadModelIntoVAO("assets/models/Core/" + modelFileName, drawingInfo, shaderProgramID);
      /*  if (!pVAOManager->LoadModelIntoVAO("assets/models/Core/" + modelFileName, drawingInfo, shaderProgramID))
        {
            std::cout << "assets/models/Core/" << modelFileName << "Is not valid" << std::endl;
        }*/
        if(modelName != "")
            modelName = modelName + "\n";

        std::cout << "Loaded: " << modelFileName << " with " << drawingInfo.numberOfVertices << " vertices" << std::endl;
        modelName = modelName + modelFileName;
        modelsLoaded++;

    } while (true);

    //remove meshDesc
    sceneFile >> dumpster;

    do
    {
        cMesh* meshDrawInfo = new cMesh;

        //model path
        std::string modelType = "";
        sceneFile >> modelType;

        if (modelType == "END")// LOOP TILL END
            break;

        meshDrawInfo->meshName = modelType;

        //xyz
        std::string xCord = "";
        std::string yCord = "";
        std::string zCord = "";

        //get them
        sceneFile >> xCord;
        sceneFile >> yCord;
        sceneFile >> zCord;

        //xyz
        meshDrawInfo->drawPosition.x = std::stof(xCord);
        meshDrawInfo->drawPosition.y = std::stof(yCord);
        meshDrawInfo->drawPosition.z = std::stof(zCord);

        //xyz Rot
        std::string xRot = "";
        std::string yRot = "";
        std::string zRot = "";

        sceneFile >> xRot;
        sceneFile >> yRot;
        sceneFile >> zRot;


        meshDrawInfo->setRotationFromEuler(glm::vec3(std::stof(xRot), std::stof(yRot), std::stof(zRot)));

        //scale
        std::string scale = "";
        sceneFile >> scale;

        meshDrawInfo->setUniformDrawScale(std::stof(scale));//scales

        //RGBA
        std::string rCol = "";
        std::string gCol = "";
        std::string bCol = "";
        std::string aCol = "";

        //get color
        sceneFile >> rCol;
        sceneFile >> gCol;
        sceneFile >> bCol;
        sceneFile >> aCol;

        //RGBA inputed as 255 vals: needs floats
        meshDrawInfo->wholeObjectDebugColourRGBA = glm::vec4(std::stof(rCol) / 255.0, std::stof(gCol) / 255.0, std::stof(bCol) / 255.0, std::stof(aCol));

        //frendily name
        std::string fName = "";
        sceneFile >> fName;

        meshDrawInfo->friendlyName = fName;

        //texture name
        std::string textName = "";
        sceneFile >> textName;

        meshDrawInfo->textureName[0] = textName;

        //texture number
        float textNumber = 0.0;
        sceneFile >> textNumber;

        meshDrawInfo->textureRatios[0] = textNumber;

        //add to
        meshsCreated++;
        g_vec_pMeshesToDraw.push_back(meshDrawInfo);

    } while (true);

    //output result
    std::cout << "Loaded from file: Models = " << std::to_string(modelsLoaded) << " and Meshs = " + std::to_string(meshsCreated) << std::endl;
    return toBeReturned;
}

bool consoleOutputOfScene(void)
{
    system("cls");// clear

    //camera block
    std::cout << "CAMERA" << std::endl;
    std::cout << g_cameraEye.x << " " << g_cameraEye.y << " " << g_cameraEye.z << std::endl;

    //MODEL block
    std::cout << "MODELS" << std::endl;

    //list model names
    std::cout << modelName << std::endl;

    std::cout << "END" << std::endl;

    //SET BLOCK
    std::cout << "SETS" << std::endl;

    //load all models
    //cMesh* pTable = new cMesh();
    //pTable->meshName = "table.ply";
    //pTable->drawPosition.y = -10.0f;
    //pTable->drawPosition.z = 0.0f;
    //pTable->setUniformDrawScale(.03);
    //pTable->friendlyName = "Table";
    //pTable->textureName[0] = "table.bmp";
    //pTable->textureRatios[0] = 1.0f;
    //::g_vec_pMeshesToDraw.push_back(pTable);

    //list model names
    for (int i = 0; i < g_vec_pMeshesToDraw.size(); i++)
    {
        std::cout << g_vec_pMeshesToDraw[i]->meshName << std::endl;
        std::cout << g_vec_pMeshesToDraw[i]->drawPosition.x << " " << g_vec_pMeshesToDraw[i]->drawPosition.y << " " << g_vec_pMeshesToDraw[i]->drawPosition.z << std::endl;
        std::cout << g_vec_pMeshesToDraw[i]->getDrawOrientation().x << " " << g_vec_pMeshesToDraw[i]->getDrawOrientation().y << " " << g_vec_pMeshesToDraw[i]->getDrawOrientation().z << std::endl;
        std::cout << g_vec_pMeshesToDraw[i]->drawScale.x << std::endl; // all are uniform
        std::cout << g_vec_pMeshesToDraw[i]->wholeObjectDebugColourRGBA.x << " " << g_vec_pMeshesToDraw[i]->wholeObjectDebugColourRGBA.y << " " << g_vec_pMeshesToDraw[i]->wholeObjectDebugColourRGBA.z << " " << g_vec_pMeshesToDraw[i]->wholeObjectDebugColourRGBA.a << std::endl; // all are uniform
        std::cout << g_vec_pMeshesToDraw[i]->friendlyName << std::endl; // all are uniform
        std::cout << g_vec_pMeshesToDraw[i]->textureName[0] << std::endl;
        std::cout << g_vec_pMeshesToDraw[i]->textureRatios[0] << std::endl;

    }

    std::cout << "END" << std::endl;

    //could add a write to file


    return true;
}

bool LoadModels()
{


    cMesh* pTable = new cMesh();
    pTable->meshName = "table.ply";
   // pTable->drawPosition.y = 0.0f;
    //pTable->drawPosition.z = 10.0f;
    pTable->setUniformDrawScale(0.04);
    pTable->friendlyName = "Table";
    pTable->textureName[0] = "table.bmp";
    pTable->textureRatios[0] = 1.0f;
  // ::g_vec_pMeshesToDraw.push_back(pTable);

    //model needs fixin
    cMesh* pBear = new cMesh();
    pBear->meshName = "bear.ply";
    // pTable->drawPosition.y = 0.0f;
     //pTable->drawPosition.z = 10.0f;
    pBear->setUniformDrawScale(0.01f);
    pBear->friendlyName = "bear";
    pBear->textureName[0] = "bear.bmp";
    pBear->textureRatios[0] = 1.0f;
   // ::g_vec_pMeshesToDraw.push_back(pBear);

    //model needs fixin
    //cMesh* pDilly = new cMesh();
    //pDilly->meshName = "dilly.ply";
    //// pTable->drawPosition.y = 0.0f;
    // //pTable->drawPosition.z = 10.0f;
    //pDilly->setUniformDrawScale(0.05f);
    //pDilly->friendlyName = "dilly";
    ////pDilly->textureName[0] = "bear.bmp";
    ////pDilly->textureRatios[0] = 1.0f;
    //::g_vec_pMeshesToDraw.push_back(pDilly);

        //model needs fixin
    cMesh* pDoom = new cMesh();
    pDoom->meshName = "doom.ply";
    // pTable->drawPosition.y = 0.0f;
     //pTable->drawPosition.z = 10.0f;
    pDoom->setUniformDrawScale(10.05f);
    pDoom->friendlyName = "doom";
    //pDilly->textureName[0] = "bear.bmp";
    //pDilly->textureRatios[0] = 1.0f;
   // ::g_vec_pMeshesToDraw.push_back(pDoom);



    cMesh* pGroundMesh = new cMesh();
    //    pGroundMesh->meshName = "Terrain_xyz_n_rgba_uv.ply";
    //    pGroundMesh->meshName = "Big_Flat_Mesh_256x256_00_132K_xyz_n_rgba_uv.ply";    
    //    pGroundMesh->meshName = "Big_Flat_Mesh_256x256_07_1K_xyz_n_rgba_uv.ply";    
    pGroundMesh->meshName = "Big_Flat_Mesh_256x256_12_5_xyz_n_rgba_uv.ply";
    pGroundMesh->drawPosition.y = -50.0f;
    pGroundMesh->drawPosition.z = -50.0f;
    pGroundMesh->friendlyName = "Ground";

    //pGroundMesh->bIsWireframe = true;
    //pGroundMesh->bDoNotLight = true;
    //pGroundMesh->wholeObjectDebugColourRGBA = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    //pGroundMesh->bUseDebugColours = true;
    //
//    pGroundMesh->textureName[0] = "NvF5e_height_map.bmp";
//    pGroundMesh->textureName[0] = "Blank_UV_Text_Texture.bmp";
    pGroundMesh->textureName[0] = "TaylorSwift_Eras_Poster.bmp";
    pGroundMesh->textureRatios[0] = 1.0f;



    //::g_vec_pMeshesToDraw.push_back(pGroundMesh);




    const float MAX_SPHERE_LOCATION = 30.0f;
    const float MAX_VELOCITY = 1.0f;

    // Make a bunch of spheres...

    return true;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    const float CAMERA_MOVEMENT_SPEED = 1.0f;
    const float OBJECT_MOVEMENT_SPEED = 0.01f;
    const float LIGHT_MOVEMENT_SPEED = 1.0f;
    int maxCounter = g_vec_pMeshesToDraw.size() - 1;

    // Nothing down
    if (mods == 0)
    {
        if (key == GLFW_KEY_C && action)
        {
            std::cout <<" -/+ switches selected object" << std::endl;
            std::cout << " n/m changes scale (m is minus)" << std::endl;
            std::cout << " wasd moves objects" << std::endl;
            std::cout << " qe moves objects y state" << std::endl;
            std::cout << " p prints out current states" << std::endl;
            std::cout << " t test key" << std::endl;

        }

        if (key == GLFW_KEY_T && action)
        {
             g_HeightAdjust += 10.0f;

        }

        if (key == GLFW_KEY_P && action)
        {
            std::cout << "drawPosition " << g_vec_pMeshesToDraw[objectToMove]->drawPosition.x << " " << g_vec_pMeshesToDraw[objectToMove]->drawPosition.y << " " << g_vec_pMeshesToDraw[objectToMove]->drawPosition.z << std::endl;
            std::cout << "drawScale " << g_vec_pMeshesToDraw[objectToMove]->drawScale.x << " " << g_vec_pMeshesToDraw[objectToMove]->drawScale.y << " " << g_vec_pMeshesToDraw[objectToMove]->drawScale.z << std::endl;
        }

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


        if (key == GLFW_KEY_N && action)
        {
            g_vec_pMeshesToDraw[objectToMove]->drawScale.x += 0.05;
            g_vec_pMeshesToDraw[objectToMove]->drawScale.y += 0.05;
            g_vec_pMeshesToDraw[objectToMove]->drawScale.z += 0.05;


        }


        if (key == GLFW_KEY_N && action)
        {
            g_vec_pMeshesToDraw[objectToMove]->drawScale.x -= 0.05;
            g_vec_pMeshesToDraw[objectToMove]->drawScale.y -= 0.05;
            g_vec_pMeshesToDraw[objectToMove]->drawScale.z -= 0.05;
        }



        if (key == GLFW_KEY_A && action)
        {

            g_vec_pMeshesToDraw[objectToMove]->drawPosition.x -= CAMERA_MOVEMENT_SPEED;
            /*g_vec_pMeshesToDraw[1]->drawPosition.x += 0.05f;*/
           // std::cout << "x-: " << g_vec_pMeshesToDraw[objectToMove]->drawPosition.x << std::endl;

        }
        if (key == GLFW_KEY_D && action)
        {
            g_vec_pMeshesToDraw[objectToMove]->drawPosition.x += CAMERA_MOVEMENT_SPEED;
            //std::cout << "x+: " << g_vec_pMeshesToDraw[0]->drawPosition.x << std::endl;

        }

        if (key == GLFW_KEY_W && action)
        {
            g_vec_pMeshesToDraw[objectToMove]->drawPosition.z -= CAMERA_MOVEMENT_SPEED;
            //std::cout << "z-: " << g_vec_pMeshesToDraw[objectToMove]->drawPosition.z << std::endl;
        }
        if (key == GLFW_KEY_S && action)
        {
            g_vec_pMeshesToDraw[objectToMove]->drawPosition.z += CAMERA_MOVEMENT_SPEED;
            //std::cout << "z+: " << g_vec_pMeshesToDraw[0]->drawPosition.z << std::endl;

        }

        if (key == GLFW_KEY_Q && action)
        {
            g_vec_pMeshesToDraw[objectToMove]->drawPosition.y += CAMERA_MOVEMENT_SPEED;
            //std::cout << "y+: " << g_vec_pMeshesToDraw[0]->drawPosition.y << std::endl;
            
        }
        if (key == GLFW_KEY_E && action)
        {
            g_vec_pMeshesToDraw[objectToMove]->drawPosition.y -= CAMERA_MOVEMENT_SPEED;
            //std::cout << "y-: " << g_vec_pMeshesToDraw[0]->drawPosition.y << std::endl;

        }

    }

    return;
}