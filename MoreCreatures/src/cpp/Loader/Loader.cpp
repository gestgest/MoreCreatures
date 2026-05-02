#include <Loader/Loader.h>

#include <glad/glad.h>
#include <header/std_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>


bool Loader::loadTexture(unsigned int& texture, const std::string& path)
{
    int width, height, nrChannels;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (!data)
    {
        std::cout << "Failed to load texture: " << path << std::endl;
        return false;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return true;
}


bool Loader::loadModel(const std::string& path, std::vector<float>& outVertices, int& outVertexCount)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "Failed to load model: " << path << " — " << importer.GetErrorString() << std::endl;
        return false;
    }

    outVertices.clear();
    outVertexCount = 0;

    // 모든 메시의 삼각형을 펼쳐서 8 floats/vertex 포맷으로 출력
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        const aiMesh* mesh = scene->mMeshes[m];

        for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
        {
            const aiFace& face = mesh->mFaces[f];
            if (face.mNumIndices != 3) continue; // Triangulate 후이므로 항상 3

            for (unsigned int i = 0; i < 3; ++i)
            {
                unsigned int idx = face.mIndices[i];

                // position
                outVertices.push_back(mesh->mVertices[idx].x);
                outVertices.push_back(mesh->mVertices[idx].y);
                outVertices.push_back(mesh->mVertices[idx].z);

                // normal
                if (mesh->HasNormals())
                {
                    outVertices.push_back(mesh->mNormals[idx].x);
                    outVertices.push_back(mesh->mNormals[idx].y);
                    outVertices.push_back(mesh->mNormals[idx].z);
                }
                else
                {
                    outVertices.push_back(0.0f);
                    outVertices.push_back(1.0f);
                    outVertices.push_back(0.0f);
                }

                // tex coords (channel 0)
                if (mesh->HasTextureCoords(0))
                {
                    outVertices.push_back(mesh->mTextureCoords[0][idx].x);
                    outVertices.push_back(mesh->mTextureCoords[0][idx].y);
                }
                else
                {
                    outVertices.push_back(0.0f);
                    outVertices.push_back(0.0f);
                }

                outVertexCount++;
            }
        }
    }

    return outVertexCount > 0;
}
