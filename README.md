# MoreCreatures
open world survival game with OpenGL
- 1인칭 / 3인칭 시점 캐릭터 조작과 카메라 추적, 기본 렌더링 파이프라인을 구현한 C++/OpenGL 데모 프로젝트입니다.

---
## 프로젝트 정보
### 기간
2026년 1월 14일 ~ 1월 16일 (3일)

### 인원
1인 개발

### 구현한 기술
- 이동
- 1인칭/3인칭 전환
- 마우스 화면 회전
  - 화면 스크롤 줌
- 중력
- 물체 오브젝트 충돌 감지 알고리즘 (AABB 알고리즘)

---
# 인 게임 플레이
<img width="1199" height="831" alt="image" src="https://github.com/user-attachments/assets/fe6f10bb-b866-4a09-b876-1c9ffeac57dd" />

### 이동
![bandicam 2026-03-05 20-48-15-421](https://github.com/user-attachments/assets/a1c35149-55b7-439c-a5f7-c1cf037c5414)

### 1인칭 / 3인칭 전환
![bandicam 2026-03-05 20-48-15-421 (1)](https://github.com/user-attachments/assets/86e55647-b020-4b16-a728-cbfa34af6ad5)
<img width="1092" height="653" alt="image" src="https://github.com/user-attachments/assets/6516d90b-112c-45a9-b821-dd424f188889" />
trackingPos 벡터를 이용하여 플레이어의 앞 벡터를 구했습니다.
```
void updateCameraPosition(glm::vec3 player_pos)
{
    trackingPos.x = cos(glm::radians(Yaw)) * -cos(glm::radians(Pitch));
    trackingPos.y = -sin(glm::radians(Pitch));
    trackingPos.z = sin(glm::radians(Yaw)) * -cos(glm::radians(Pitch));

    //3인칭 뷰
    if (isThirdView)
    {
        trackingPos *= Zoom;

        //trackingPos는 Back;
        Position = player_pos + trackingPos;

        //카메라가 플레이어를 쳐다보는
        glm::vec3 frontCameraVector = player_pos - Position;
        updateCameraVectors(frontCameraVector);
    }
    //1인칭 뷰
    else
    {
        updateCameraVectors(-trackingPos);
        float m = 5;
        //Position = player_pos + Front;
        Position = player_pos + glm::vec3(Front.x * m, Front.y * m + 5.0, Front.z * m);
    }
}

```


### 중력
![bandicam 2026-03-05 20-48-15-421 (2)](https://github.com/user-attachments/assets/c2074e8e-499e-4706-8b82-5fa7f808d810)
```

// render loop
// -----------
while (!glfwWindowShouldClose(window))
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    for (int i = 0; i < objects.size(); i++)
    {
        objects[i]->applyPhysics(deltaTime);
        for (int j = i + 1; j < objects.size(); j++)
        {
            //움직이는 물체면
            if (!objects[j]->getIsActive())
            {
                continue;
            }

            //물체가 닿았는지
            if (objects[i]->isCollisionEnter(objects[j]))
            {
                objects[i]->addRepulsion(deltaTime);
                objects[j]->addRepulsion(deltaTime);

                //땅에 닿았는지
                if (objects[i] == player || objects[j] == player)
                    player->SetIsGround(true);
            }
        }
    }

    // render
}

```

### AABB 알고리즘
물체 오브젝트 충돌 감지 알고리즘
```
bool isCollisionEnter(GameObject* object)
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

```

