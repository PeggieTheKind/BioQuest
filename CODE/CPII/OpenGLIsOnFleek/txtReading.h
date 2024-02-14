#pragma once
#include <vector>
#include <fstream>
#include <pugixml/pugixml.hpp>
#include "cMesh.h"

class textReading
{
public:
	textReading()
	{
		pathOfTXT = "gameData.txt";
	}

	std::vector<cMesh*> readTxt();

	std::string pathOfTXT;
};

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
			temp->friendlyName = lineOf.substr(1, lineOf.find(':'));
		}

		if (stat == "placement")
		{
			//populate pos
			temp->drawPosition.x = stof(lineOf.substr(1, lineOf.find(':'))); // x: find split line at X pos, and turn to float

			temp->drawPosition.y = stof(lineOf.substr(2, lineOf.find(':')));// y: find split line at X pos, and turn to float

			temp->drawPosition.z = stof(lineOf.substr(3, lineOf.find(':')));// z: find split line at X pos, and turn to float

		}

		if (stat == "scale")
		{
			//get value, float it !
			temp->setUniformDrawScale(stof(lineOf.substr(1, lineOf.find(':'))));
		}

		if (stat == "health")
		{
			//get value, float it !
			temp->healthOf = stof(lineOf.substr(1, lineOf.find(':')));
		}


		std::cout << lineOf << std::endl;
	}

	/*
	name:player
	placement:0:0:0
	scale:1
	health:100
	*/

	return toReturn;
}

