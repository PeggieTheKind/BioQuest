// Here's where I got this example: 
// https://assimp-docs.readthedocs.io/en/latest/usage/use_the_lib.html

// You need to point to the assimp-5.2.5\include folder.
// AND to the config.h file which is in the VS22XXX include folder

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

// You also need the zip header
#include <zip.h>

//
//  #include "../../include/assimp/Importer.hpp"
//  #include "../../include/assimp/scene.h">"           // Output data structure
//  #include "../../include/assimp/postprocess.h">"     // Post processing flags

#include <string>
#include <iostream>


void DoTheErrorLogging(const char * errorString)
{
    std::cout << "Error: " << errorString << std::endl;
    return;
}

void DoTheSceneProcessing(const aiScene* scene)
{

    return;
}



bool DoTheImportThing(const std::string& pFile) {
  // Create an instance of the Importer class
    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(pFile,
                                             aiProcess_CalcTangentSpace |
                                             aiProcess_Triangulate |
                                             aiProcess_JoinIdenticalVertices |
                                             aiProcess_SortByPType);

                                           // If the import failed, report it
    if (nullptr == scene) 
    {
        DoTheErrorLogging(importer.GetErrorString());
        return false;
    }

    // Now we can access the file's contents.
    DoTheSceneProcessing(scene);

    // We're done. Everything will be cleaned up by the importer destructor
    return true;
}


int main(int argc, char* argv[])
{
//    if ( DoTheImportThing("bathtub.ply") )
//    if ( DoTheImportThing("spongebob_model_v39v39.obj") )
    if ( DoTheImportThing("spongebob_model_v39v39.FBX") )
    {
        std::cout << "Loaded OK" << std::endl;
    }



	return 0;
}
