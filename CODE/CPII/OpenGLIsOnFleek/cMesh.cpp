#include "cMesh.h"
#include <iostream>			// For KillAllHumans() cout

#ifdef _DEBUG
#include "cGlobal.h"	//cDebugRenderer* 

#endif


// Constructor: Called on creation   c'tor
cMesh::cMesh()
{
	this->drawPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	//this->drawOrientation = glm::vec3(0.0f, 0.0f, 0.0f);
	this->setRotationFromEuler(glm::vec3(0.0f, 0.0f, 0.0f));
	this->drawScale = glm::vec3(1.0f);

	//physics temp
	this->acceleration = 0.0f;
	this->velocity = 0.0f;

	this->bIsWireframe = false;
	this->bDoNotLight = false;

	this->bIsVisible = true;
	this->alpha_trans = 1.0;//default show
	this->bUseDebugColours = false;
	this->wholeObjectDebugColourRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	// These all add up to 1.0f
	textureRatios[0] = 1.0f;
	textureRatios[1] = 0.0f;
	textureRatios[2] = 0.0f;
	textureRatios[3] = 0.0f;
	textureRatios[4] = 0.0f;
	textureRatios[5] = 0.0f;
	textureRatios[6] = 0.0f;
	textureRatios[7] = 0.0f;

	this->direction = 0;

	// Set uniqueID
	this->m_UniqueID = cMesh::m_nextUniqueID;
	cMesh::m_nextUniqueID++;

	playerMeshs.push_back("player7.ply"); //base
	playerMeshs.push_back("player5.ply"); //right
	playerMeshs.push_back("player2.ply"); //left
	playerMeshs.push_back("player11.ply"); // up
}

unsigned int cMesh::getUniqueID(void)
{
	return this->m_UniqueID;
}

// static
unsigned int cMesh::m_nextUniqueID = cMesh::FIRST_UNIQUE_ID;


// Destructor: Called on deletion   d'tor
cMesh::~cMesh()
{

}

void cMesh::setUniformDrawScale(float scale)
{
	this->drawScale.x = this->drawScale.y = this->drawScale.z = scale;
	return;
}

void cMesh::directionChange(int directionTo, cMesh* holderOf)
{
	if (directionTo == 0) //down
		holderOf->meshName = playerMeshs[0];

	if (directionTo == 1) //left
		holderOf->meshName = playerMeshs[1];

	if (directionTo == 2) //right
		holderOf->meshName = playerMeshs[2];

	if (directionTo == 3) //up
		holderOf->meshName = playerMeshs[3];
}

// STARTOF: From: iPhysicsMeshTransformAccess interface
glm::vec3 cMesh::getDrawPosition(void)
{
	return this->drawPosition;
}

void cMesh::setDrawPosition(const glm::vec3& newPosition)
{
	this->drawPosition = newPosition;
	return;
}

glm::vec3 cMesh::getDrawOrientation(void)
{
//	return this->drawOrientation;

	// Be a little careful because quaterion to Euler can return unexpected things
	return glm::eulerAngles(this->get_qOrientation());
}

void cMesh::setDrawOrientation(const glm::vec3& newOrientation)
{
//	this->drawOrientation = newOrientation;

	this->setRotationFromEuler(newOrientation);

	return;
}

void cMesh::setDrawOrientation(const glm::quat& newOrientation)
{
//	this->drawOrientation = newOrientation;

	this->m_qOrientation = newOrientation;

	return;
}// ENDOF: iPhysicsMeshTransformAccess interface


//void cMesh::Update(double deltaTime)
//{
//#ifdef _DEBUG
////	::g_pDebugRenderer->AddSphere();
//#endif
//	return;
//}

