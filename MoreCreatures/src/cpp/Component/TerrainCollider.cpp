#include <Component/TerrainCollider.h>
#include <GameObject/Terrain.h>
#include <GameObject/GameObject.h>

#include <algorithm>
#include <limits>


TerrainCollider::TerrainCollider(GameObject* owner, Terrain* terrain,
                                 const glm::vec3& localMin, const glm::vec3& localMax,
                                 int samplesPerSide)
    : Collider(owner), terrain(terrain),
      localMin(localMin), localMax(localMax),
      samplesPerSide(samplesPerSide < 1 ? 1 : samplesPerSide) {}


//박스의 최소 값
glm::vec3 TerrainCollider::worldMin() const
{
    return owner->getPosition() + center + localMin;
}

//박스의 최대 값
glm::vec3 TerrainCollider::worldMax() const
{
    return owner->getPosition() + center + localMax;
}


//핵심
bool TerrainCollider::overlaps(Collider* other)
{
    if (!other || !terrain) return false;

    glm::vec3 oMin = other->worldMin();
    glm::vec3 oMax = other->worldMax();

    glm::vec3 tMin = worldMin();
    glm::vec3 tMax = worldMax();

    //1) xz 평면에서 지형 영역과 안 겹치면 false. 마치 동 떨어진 지구와 명왕성
    if (oMax.x < tMin.x || oMin.x > tMax.x) return false;
    if (oMax.z < tMin.z || oMin.z > tMax.z) return false;

    //반례) 만약 봉우리가 0 ~ 100인 지형에서 y가 50으로 비둘기가 날라다닌다면? 기존의 AABB에서는 충돌됨
    //2) 박스 바닥이 지형의 최대 가능 높이보다 위면 컷
    if (oMin.y > tMax.y) return false;

    //3) 박스 풋프린트를 지형 영역으로 클램프 후 NxN 그리드 샘플
    float x0 = std::max(oMin.x, tMin.x);
    float x1 = std::min(oMax.x, tMax.x);
    float z0 = std::max(oMin.z, tMin.z);
    float z1 = std::min(oMax.z, tMax.z);

    int n = samplesPerSide;
    float maxHeight = -std::numeric_limits<float>::infinity();
    for (int iz = 0; iz < n; ++iz)
    {
        float v = (n == 1) ? 0.5f : float(iz) / float(n - 1);
        float wz = z0 + (z1 - z0) * v;
        for (int ix = 0; ix < n; ++ix)
        {
            float u = (n == 1) ? 0.5f : float(ix) / float(n - 1);
            float wx = x0 + (x1 - x0) * u;

            float h = terrain->getHeightAt(wx, wz);
            if (h > maxHeight) maxHeight = h;
        }
    }

    //4) 박스 바닥이 샘플 최대 높이보다 낮으면 충돌
    if (oMin.y >= maxHeight) return false;

    //5) 위치 보정: 상대(움직이는) 오브젝트를 지형 표면 위로 끌어올린다.
    //   addRepulsion은 속도만 0으로 만들고 위치는 안 고치므로, heightfield는
    
    
    //   **여기서 직접 박스를 표면에 붙여줘야 언덕을 자연스럽게 오를 수 있다.**
    GameObject* obj = other->getOwner();
    if (obj && !obj->getIsStatic())
    {
        float penetration = maxHeight - oMin.y;
        glm::vec3 p = obj->getPosition();
        p.y += penetration;            // 박스 바닥이 maxHeight에 닿도록
        obj->setPosition(p);

        glm::vec3 v = obj->getVelocity();
        if (v.y < 0.0f) v.y = 0.0f;    // 아래로 향한 속도만 제거 (점프는 보존)
        obj->setVelocity(v);
    }
    return true;
}
