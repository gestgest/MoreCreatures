#include <Component/BoxCollider.h>
#include <GameObject/GameObject.h>


BoxCollider::BoxCollider(GameObject* owner, const glm::vec3& size)
    : Collider(owner), halfExtents(size * 0.5f) {}


glm::vec3 BoxCollider::worldMin() const
{
    glm::vec3 c = owner->getPosition() + center;
    return c - halfExtents;
}

glm::vec3 BoxCollider::worldMax() const
{
    glm::vec3 c = owner->getPosition() + center;
    return c + halfExtents;
}

//AABB vs AABB 물리 범주 검사.
bool BoxCollider::overlaps(Collider* other)
{
    if (!other) return false;

    glm::vec3 aMin = worldMin();
    glm::vec3 aMax = worldMax();

    glm::vec3 bMin = other->worldMin();
    glm::vec3 bMax = other->worldMax();

    if (aMax.x < bMin.x || aMin.x > bMax.x) return false;
    if (aMax.y < bMin.y || aMin.y > bMax.y) return false;
    if (aMax.z < bMin.z || aMin.z > bMax.z) return false;
    return true;
}
