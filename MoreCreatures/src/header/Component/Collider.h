#ifndef COLLIDER_H
#define COLLIDER_H

#include "Component.h"
#include <glm/glm.hpp>

class GameObject;

class Collider : public Component {
protected:
    GameObject* owner;
    glm::vec3 center;   //owner.position 기준 로컬 오프셋

public:
    Collider(GameObject* owner) : owner(owner), center(0.0f) {}
    virtual ~Collider() = default;

    virtual bool overlaps(Collider* other) = 0;

    void setCenter(const glm::vec3& c) { center = c; }
    glm::vec3 getCenter() const { return center; }
    GameObject* getOwner() const { return owner; }

    //월드공간 AABB (overlaps 디스패치용). 비-AABB 모양도 보수적 AABB 반환.
    virtual glm::vec3 worldMin() const = 0;
    virtual glm::vec3 worldMax() const = 0;
};

#endif
#pragma once
