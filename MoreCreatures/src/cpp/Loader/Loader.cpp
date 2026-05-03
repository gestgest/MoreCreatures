#include <Loader/Loader.h>

#include <glad/glad.h>
#include <header/std_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <header/tiny_obj_loader.h>

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


//obj 변환기
bool Loader::loadModel(const std::string& path, std::vector<float>& outVertices, int& outVertexCount)
{
    //설정
    tinyobj::ObjReaderConfig config;
    config.triangulate = true;

    //읽기
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(path, config))
    {
        if (!reader.Error().empty())
            std::cout << "Failed to load model: " << path << " — " << reader.Error() << std::endl;
        else
            std::cout << "Failed to load model: " << path << std::endl;
        return false;
    }
    if (!reader.Warning().empty())
        std::cout << "OBJ warning: " << reader.Warning() << std::endl;

    //속성
    const tinyobj::attrib_t& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes(); 
    const auto& materials = reader.GetMaterials();

    outVertices.clear();
    outVertexCount = 0;

    for (const auto& shape : shapes)
    {
        const auto& mesh = shape.mesh; //tinyobj::mesh_t
        size_t indexOffset = 0;

        // 면 단위로 순회 (각 면 = 3 vertex, triangulate=true)
        // 대체로 mesh.num_face_vertices.size() 갯수는 12988 정도 됨
        for (size_t f = 0; f < mesh.num_face_vertices.size(); ++f)
        {
            int matId = (f < mesh.material_ids.size()) ? mesh.material_ids[f] : -1;

            // 머티리얼의 diffuse 색상 (default : 흰색)
            float r = 1.0f, g = 1.0f, b = 1.0f;

            //rgb 가져오는 함수
            if (matId >= 0 && matId < (int)materials.size())
            {
                r = materials[matId].diffuse[0];
                g = materials[matId].diffuse[1];
                b = materials[matId].diffuse[2];
            }


            //vertex 3
            for (size_t v = 0; v < 3; ++v)
            {
                const tinyobj::index_t& idx = mesh.indices[indexOffset + v]; //인덱스 버퍼

                outVertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]); //x
                outVertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]); //y
                outVertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]); //z

                //normal 값이 있다면 normal
                if (idx.normal_index >= 0)
                {
                    outVertices.push_back(attrib.normals[3 * idx.normal_index + 0]);
                    outVertices.push_back(attrib.normals[3 * idx.normal_index + 1]);
                    outVertices.push_back(attrib.normals[3 * idx.normal_index + 2]);
                }
                else //default : normal 기본 위쪽 방향
                {
                    outVertices.push_back(0.0f);
                    outVertices.push_back(1.0f);
                    outVertices.push_back(0.0f);
                }

                //색 주입
                outVertices.push_back(r);
                outVertices.push_back(g);
                outVertices.push_back(b);

                outVertexCount++;
            }
            indexOffset += 3;
        }
    }

    return outVertexCount > 0;
}
