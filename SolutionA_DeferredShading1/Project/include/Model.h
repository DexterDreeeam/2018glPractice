#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <../include/Mesh.h>
#include <Shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

unsigned int TextureFromFile(const char *, const string &, bool gamma = false);

class Model{
public:
    ///model data
    vector<Texture> textures_loaded;
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;
    ///constructor
    Model(const string & path, bool gamma = false) : gammaCorrection(gamma){
        loadModel(path);
    }
    ///draw the model
    void Draw(Shader shader){
        for(unsigned int i = 0; i != meshes.size(); ++i){
            meshes[i].Draw(shader);
        }
    }
private:
    ///load a model with supported ASSIMP
    void loadModel(const string & path){
        Assimp::Importer importer;
        const aiScene * scene = importer.ReadFile(
            path, aiProcess_Triangulate | 
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace
            );
        ///check error
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
            cout << "error, Assimp " << importer.GetErrorString() << endl;
            return;
        }
        ///retrieve the directory path
        directory = path.substr(0, path.find_last_of('/'));
        ///process ASSIMP root node recursively
        processNode(scene->mRootNode, scene);
    }
    ///process a node in a recursive fashion
    void processNode(aiNode * node, const aiScene * scene){
        for(unsigned int i = 0; i != node->mNumMeshes; ++i){
            aiMesh * mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for(unsigned int i = 0; i != node->mNumChildren; ++i){
            processNode(node->mChildren[i], scene);
        }
    }
    ///process mesh
    Mesh processMesh(aiMesh * mesh, const aiScene * scene){
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;
        ///walk through each of the mesh's vertices
        for(unsigned int i = 0; i != mesh->mNumVertices; ++i){
            Vertex vertex;
            glm::vec3 vector;
            ///position
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            ///normal
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            ///texture coordinates
            if(mesh->mTextureCoords[0]){
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            ///tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            ///bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
            vertices.push_back(vertex);
        }
        ///walk through each of the mesh's faces
        for(unsigned int i = 0; i != mesh->mNumFaces; ++i){
            aiFace face = mesh->mFaces[i];
            ///retrieve all indices of the face and store
            for(unsigned int j = 0; j != face.mNumIndices; ++j){
                indices.push_back(face.mIndices[j]);
            }
        }
        ///process materials
        aiMaterial * material = scene->mMaterials[mesh->mMaterialIndex];
        ///1. diffuse maps
        vector<Texture> diffuseMaps = 
            loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        ///2. specular maps
        vector<Texture> specularMaps =
            loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        ///3. normal maps
        vector<Texture> normalMaps = 
            loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        ///4. height maps
        vector<Texture> heightMaps =
            loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        ///return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }
    ///check and et all material texutres of a given type
    vector<Texture> loadMaterialTextures(aiMaterial * mat, aiTextureType type, string typeName){
        vector<Texture> textures;
        for(unsigned int i = 0; i != mat->GetTextureCount(type); ++i){
            aiString str;
            mat->GetTexture(type, i, &str);
            ///check if texture was loaded before
            bool skip = false;
            for(unsigned int j = 0; j != textures_loaded.size(); ++j){
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0){
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            ///texture hasn't been loaded
            if(!skip){
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }
};

unsigned int TextureFromFile(const char * path, const string & directory, bool gamma){
    string filename = string(path);
    filename = directory + '/' + filename;
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char * data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if(data){
        GLenum format;
        if(nrComponents == 1)
            format = GL_RED;
        else if(nrComponents == 3)
            format = GL_RGB;
        else if(nrComponents == 4)
            format = GL_RGBA;
        ///bind img
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0 , format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        ///set attrib
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        ///free data
        stbi_image_free(data);
    }
    else{
        std::cout << "Texture failed to load at path : " << path << endl;
        stbi_image_free(data);
    }
    return textureID;
}