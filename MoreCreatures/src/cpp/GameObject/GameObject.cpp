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

void GameObject::setMesh(Mesh* m) { mesh = m; }
Mesh* GameObject::getMesh() { return mesh; }


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

glm::vec3 GameObject::getPosition()
{
    return position;
}


bool GameObject::isCollisionEnter(GameObject* object)
{
    if (!(this->getIsActive()) || !(object->getIsActive()))
    {
        return false;
    }


    // ==이어도 0 ~ 10, 0 ~ 10, 0 ~ 10 즉, 3개가 같아야함 ==> 하나라도 다르면 물체가 만날 수 없음
    //x비교
    if (!isInBoundary(this->position.x, object->position.x, this->scale.x, object->scale.x))
    {
        return false;
    }

    if (!isInBoundary(this->position.y, object->position.y, this->scale.y, object->scale.y))
    {
        return false;
    }

    if (!isInBoundary(this->position.z, object->position.z, this->scale.z, object->scale.z))
    {
        return false;
    }
    //std::cout << object->position.y;
    return true;
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

bool GameObject::isInBoundary(float a, float b, float a_size, float b_size)
{
    float a_1 = a - a_size / 2;
    float a_2 = a + a_size / 2;
    float b_1 = b - b_size / 2;
    float b_2 = b + b_size / 2;

    if
        (
            (a_1 <= b_1 && b_1 <= a_2) ||
            (a_1 <= b_2 && b_2 <= a_2)
            )
    {
        return true;
    }
    else if
        (
            (b_1 <= a_1 && a_1 <= b_2) ||
            (b_1 <= a_2 && a_2 <= b_2)
            )
    {
        return true;
    }
    return false;
}
