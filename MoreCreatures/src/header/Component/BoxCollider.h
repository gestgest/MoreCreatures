#ifndef BOX_COLLIDER_H
#define BOX_COLLIDER_H

#include "Collider.h"

class BoxCollider : public Collider {
    glm::vec3 halfExtents;   //박스 반쪽 크기 (월드 단위)

public:
    BoxCollider(GameObject* owner, const glm::vec3& size);

    bool overlaps(Collider* other) override;

    glm::vec3 worldMin() const override;
    glm::vec3 worldMax() const override;

    void setSize(const glm::vec3& size) { halfExtents = size * 0.5f; }
    glm::vec3 getSize() const { return halfExtents * 2.0f; }
};

#endif
#pragma once
