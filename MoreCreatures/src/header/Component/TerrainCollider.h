#ifndef TERRAIN_COLLIDER_H
#define TERRAIN_COLLIDER_H

#include "Collider.h"

class Terrain;

//지형 heightfield 충돌체.
//상대(BoxCollider)의 xz 풋프린트를 샘플링해 최대 지형 높이를 구하고,
//상대 박스의 바닥(minY)이 그 높이보다 낮으면 충돌로 판정한다.
class TerrainCollider : public Collider {
    Terrain* terrain;
    glm::vec3 localMin;   //owner.position 기준 지형 코스 AABB 최소
    glm::vec3 localMax;   //owner.position 기준 지형 코스 AABB 최대
    int samplesPerSide;   //풋프린트 한 변당 샘플 수

public:
    TerrainCollider(GameObject* owner, Terrain* terrain,
                    const glm::vec3& localMin, const glm::vec3& localMax,
                    int samplesPerSide = 3);

    bool overlaps(Collider* other) override;

    glm::vec3 worldMin() const override;
    glm::vec3 worldMax() const override;

    bool isAABB() const override { return false; }
};

#endif
