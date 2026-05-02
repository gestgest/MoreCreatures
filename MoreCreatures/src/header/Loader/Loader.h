#ifndef LOADER_H
#define LOADER_H

#include <string>
#include <vector>

class Loader {
public:
    // 텍스처 로드 (RGB)
    static bool loadTexture(unsigned int& texture, const std::string& path);

    // 모델 로드 (OBJ / FBX / glTF 등 Assimp 지원 포맷)
    // 출력 포맷: pos3 + normal3 + tex2 = 8 floats per vertex
    // 성공 시 outVertices에 8*N개의 float, outVertexCount에 N 저장
    static bool loadModel(const std::string& path, std::vector<float>& outVertices, int& outVertexCount);
};

#endif
#pragma once
