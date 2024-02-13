#include "GLWF_CallBacks.h"

#include <string>
#include <iostream>
#include <vector>
#include "cMesh.h"
#include "cLightManager.h"
#include "cGlobal.h"

#include "LuaBrain/cLuaBrain.h"

extern int g_selectedMesh;// = 0;
extern std::vector< cMesh* > g_vec_pMeshesToDraw;

extern glm::vec3 g_cameraEye;

extern cLightManager* g_pTheLights;
extern int g_selectedLight;// = 0;

bool SaveVectorSceneToFile(std::string saveFileName);
int objectToMove = 0;
// HACK:
extern float g_HeightAdjust; //= 10.0f;
extern glm::vec2 g_UVOffset;// = glm::vec2(0.0f, 0.0f);

// From main.cpp
extern cLuaBrain g_LuaBrain;
// Silly function binding example
//void ChangeTaylorSwiftTexture(std::string newTexture);




