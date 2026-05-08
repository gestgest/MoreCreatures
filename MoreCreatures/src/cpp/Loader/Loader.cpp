#include <Loader/Loader.h>

#include <glad/glad.h>
#include <header/std_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <header/tiny_obj_loader.h>

#include <iostream>
#include <cstring>


bool Loader::loadTexture(unsigned int& texture, const std::string& path)
{
    int width, height, nrChannels;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //트라이리니어 + (가능 시) anisotropic filtering — 멀리서의 자글거림 제거
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //anisotropic filtering — GL 확장이 있을 때만 사용
    #ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
    #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
    #endif
    {
        const char* exts = (const char*)glGetString(GL_EXTENSIONS);
        if (exts && strstr(exts, "GL_EXT_texture_filter_anisotropic"))
        {
            GLfloat maxAniso = 1.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
            GLfloat aniso = maxAniso < 16.0f ? maxAniso : 16.0f;
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
        }
    }

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (!data)
    {
        std::cout << "Failed to load texture: " << path << std::endl;
        return false;
    }

    // 채널 수에 따라 OpenGL 포맷 자동 결정.
    // PNG with alpha (RGBA) → GL_RGBA, 일반 RGB → GL_RGB, 그레이스케일 → GL_RED.
    // 채널 수 무시하고 GL_RGB로 강제하면 RGBA 이미지에서 사이즈 mismatch로 깨짐.
    GLenum format = GL_RGB;
    if (nrChannels == 1)      format = GL_RED;
    else if (nrChannels == 3) format = GL_RGB;
    else if (nrChannels == 4) format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
