#ifndef LOADER_H
#define LOADER_H

#include <string>
#include <vector>

class Loader {
public:
    // 텍스처 로드 (RGB)
    static bool loadTexture(unsigned int& texture, const std::string& path);

    // 모델 로드 (OBJ / FBX / glTF 등 Assimp 지원 포맷)
    // 출력 포맷: pos3 + normal3 + color3 = 9 floats per vertex
    // 성공 시 outVertices에 9*N개의 float, outVertexCount에 N 저장
    //
    // targetObject: 비어있으면 OBJ 안의 모든 shape을 합쳐서 로드 (기존 동작).
    //               이름을 주면 해당 object만 골라 로드 (예: amond.obj 안의 "Almond"만).
    static bool loadModel(const std::string& path, std::vector<float>& outVertices, int& outVertexCount,
                          const std::string& targetObject = "");
};

#endif
#pragma once
