#pragma once
#include <vector>
#include <fstream>
#include <pugixml/pugixml.hpp>
#include "cMesh.h"

class textReading
{
protected:
	//base ctor
	textReading()
	{
		pathOfTXT = "gameData.txt";
	}

	//singleton
	static textReading* textReadingInstance; 

public:
	static textReading* GetInstance();

	//returns result from text
	std::vector<cMesh*> readTxt();
	std::string pathOfTXT;
};

textReading* textReading::textReadingInstance = nullptr;

inline textReading* textReading::GetInstance()
{
	if (textReadingInstance == nullptr)
	{
		textReadingInstance = new textReading();//if doesnt exist sent new object
	}

	//return pointer
	return textReadingInstance;
}

std::vector<cMesh*> textReading::readTxt()
{
	std::vector<cMesh*> toReturn;


	//read text
	std::ifstream file(pathOfTXT.c_str());
	std::string lineOf;

	cMesh* temp = new cMesh();

	//Loading format in txt
	//{stat to change}:{value}
	//value can be unique
	//example: 
		// POS: XYZ
	while (std::getline(file, lineOf)) {

		std::string stat = lineOf.substr(0, lineOf.find(':'));
		if (stat == "name")
		{
			temp->friendlyName = lineOf.substr(5, lineOf.find(':'));
		}

		if (stat == "pos")
		{
			//populate pos
			std::string tester = lineOf.substr(4, lineOf.find(':'));
			temp->drawPosition.x = stof(tester); // x: find split line at X pos, and turn to float

			tester = lineOf.substr(6, lineOf.find(':'));
			temp->drawPosition.y = stof(tester);// y: find split line at X pos, and turn to float

			tester = lineOf.substr(8, lineOf.find(':'));
			temp->drawPosition.z = stof(tester);// z: find split line at X pos, and turn to float

		}

		if (stat == "scale")
		{
			//get value, float it !
			std::string tester = lineOf.substr(6, lineOf.find(':'));
			temp->setUniformDrawScale(stof(tester));
		}

		if (stat == "health")
		{
			//get value, float it !
			std::string tester = lineOf.substr(7, lineOf.find(':'));
			temp->healthOf = stof(tester);
		}

		if (stat == "debug")
		{
			//get value, float it !
			std::string tester = lineOf.substr(6, lineOf.find(':'));
			int state = stoi(tester); //find int 
			if (state == 0)
				temp->bUseDebugColours = false;
			else
				temp->bUseDebugColours = true;

		}

		if (stat == "model")
		{
			//populate pos
			std::string tester = lineOf.erase(0, 6);//clear front
			temp->meshName = tester;
		

		}

	}

	toReturn.push_back(temp);
	/*
	name:player
	model:player7.ply
	pos:0:0:0
	scale:.10
	debug:true
	*/

	return toReturn;
}

