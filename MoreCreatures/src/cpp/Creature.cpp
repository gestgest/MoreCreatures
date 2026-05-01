#include <GameObject/Creature.h>


Creature::Creature()
{
    front = glm::vec3(0, 0, 0);
    //initObject();
}

void Creature::SetIsGround(bool isGround)
{
    //std::cout << "Cd" << '\n';
    this->isGround = isGround;
}

bool Creature::GetIsGround()
{
    return isGround;
}

void Creature::setFront(glm::vec3 front)
{
    this->front = front;
}
