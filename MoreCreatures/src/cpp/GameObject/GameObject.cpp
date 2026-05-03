#include <GameObject/GameObject.h>


void GameObject::move(glm::vec3 velocity, float deltaTime)
{
    position += velocity * deltaTime;
}


GameObject::GameObject()
{
    scale = glm::vec3(1.0f, 1.0f, 1.0f);
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    movement_speed = 0.0f;
    isActive = true;

    isStatic = true;
}

GameObject::~GameObject()
{
    delete collider;
}

void GameObject::setMesh(Mesh* m) { mesh = m; }
Mesh* GameObject::getMesh() { return mesh; }

void GameObject::setCollider(Collider* c) { collider = c; }
Collider* GameObject::getCollider() { return collider; }


void GameObject::drawGameObject(Camera& camera, glm::vec3 lightColor, glm::vec3 lightPos, glm::mat4 lightSpaceMatrix)
{
    if (!isActive || mesh == nullptr) return;
    mesh->updateUniforms(camera, lightColor, lightPos, glm::vec3(0.0f), position, scale);
    mesh->Bind();
    mesh->Draw();
}


void GameObject::playerMove(glm::vec3 vec, float deltaTime)
{
    vec = glm::normalize(vec);
    float velocity = movement_speed * deltaTime;

    move(vec, velocity);
}


// position은 왼쪽 아래 뒤? 기준이다.
glm::vec3 GameObject::getPosition()
{
    return position;
}


void GameObject::addRepulsion(float deltaTime)
{
    if (isStatic)
    {
        return;
    }
    //velocity *= COR; //반발계수
    if (velocity.y < 0)
    {
        velocity *= 0;
    }
}

void GameObject::applyPhysics(float deltaTime)
{
    //고정된 물체라면
    if (isStatic || !isActive)
    {
        return;
    }
    //a만큼 속력을 추가

    move(velocity, deltaTime);
    velocity += glm::vec3(0, GRAVITY_ACCELERATION, 0) * deltaTime;
}

glm::vec3 GameObject::getVelocity()
{
    return velocity;
}

void GameObject::setVelocity(glm::vec3 velocity)
{
    this->velocity = velocity;
}

bool GameObject::getIsStatic()
{
    return isStatic;
}

void GameObject::setIsActive(bool isActive)
{
    this->isActive = isActive;
}
bool GameObject::getIsActive()
{
    return isActive;
}
