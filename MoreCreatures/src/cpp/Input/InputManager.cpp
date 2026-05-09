// === 헤더 순서 주의 ===
// glad.h는 반드시 GLFW보다 먼저! (InputManager.h가 GLFW를 포함하니 그 전에 glad)
// 안 그러면 "OpenGL header already included" 에러 발생.
#include <glad/glad.h>

#include <Input/InputManager.h>

#include <header/camera.h>
#include <GameObject/Mouse.h>
#include <Manager/ChunkManager.h>
#include <UI/HUD.h>
#include <Config.h>

#include <iostream>


// === 외부 의존성 (main.cpp에 정의되어 있음) ===
extern Camera camera;
extern Mouse* player;
extern HUD*   hud;
extern ChunkManager* chunkManager;
extern float  deltaTime;

// 게임 재시작 함수 (main.cpp에 정의)
void RestartGame();


// === static 멤버 정의 ===
// 마우스 첫 입력 시 큰 점프 방지 — 첫 호출 때 화면 중심을 lastX/Y로 채워둠
float InputManager::lastX = SCR_WIDTH  / 2.0f;
float InputManager::lastY = SCR_HEIGHT / 2.0f;
bool  InputManager::firstMouse = true;


void InputManager::registerCallbacks(GLFWwindow* window)
{
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);   //리셋 버튼 클릭 처리용
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
}


// === 폴링: 매 프레임 키 상태 검사 ===
//
// 누르고 있는 동안 계속 동작해야 하는 입력은 여기서.
// (반면 R, F5, - 같은 "한 번 누름"은 keyCallback에서 처리)
void InputManager::processInput(GLFWwindow* window, float dt)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        player->playerMove(camera.getFrontPlayer(), deltaTime);
        camera.move(player->getPosition());
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        player->playerMove(-camera.getFrontPlayer(), deltaTime);
        camera.move(player->getPosition());
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        player->playerMove(-camera.getRightPlayer(), deltaTime);
        camera.move(player->getPosition());
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        player->playerMove(camera.getRightPlayer(), deltaTime);
        camera.move(player->getPosition());
    }

    //WASD 안 눌러도 (낙하 등) 카메라는 플레이어를 따라가야 함
    camera.move(player->getPosition());
}


// === 키 이벤트 콜백: "한 번 누름" 처리 ===
//
// GLFW_PRESS  — 처음 누른 순간
// GLFW_REPEAT — 누르고 있는 동안 반복
// GLFW_RELEASE — 뗀 순간
//
// 단발 동작(토글, 재시작 등)은 PRESS만 잡아야 폭주 방지.
void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //F5: 1인칭/3인칭 시점 전환
    if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
    {
        camera.isThirdView = !camera.isThirdView;
    }

    //F1: [디버그] 청크 상태 콘솔에 덤프 — 활성 청크 목록, 메모리 추정 등
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
    {
        if (chunkManager && player)
            chunkManager->printChunkMemory(player->getPosition());
    }

    //R: 게임 재시작 (HP/식량/아몬드 모두 복구)
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        RestartGame();
    }

    //'-': [디버그] 식량 우선 깎기, 식량 0이면 HP 깎기
    //굶주림 패널티 흐름(식량 → HP → 게임 오버)을 빠르게 테스트하는 용도.
    if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
    {
        if (hud)
        {
            if (hud->getFood() > 0)
            {
                hud->setFood(hud->getFood() - 1);
                std::cout << "[디버그] 식량 -1 → " << hud->getFood() << "/" << hud->getMaxFood() << std::endl;
            }
            else if (hud->getHp() > 0)
            {
                hud->setHp(hud->getHp() - 1);
                std::cout << "[디버그] 식량 0 → HP -1 → " << hud->getHp() << "/" << hud->getMaxHp() << std::endl;

                if (hud->getHp() == 0)
                    std::cout << "[게임 오버] 굶어 죽었습니다... (R로 리스폰)" << std::endl;
            }
            //else: 식량 0 + HP 0 — 더 누르는 건 무시
        }
    }
}


// === 마우스 이동 콜백: 시점 회전 ===
void InputManager::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    //첫 프레임 큰 점프 방지 — 화면 가운데에서 갑자기 마우스 좌표가 들어오면 시야가 휙 돌아감
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;   //y는 반전 — 화면 좌표는 위가 0이라

    lastX = (float)xpos;
    lastY = (float)ypos;

    camera.ProcessMouseMovement(xoffset, yoffset, player->getPosition());
    player->setFront(camera.getFrontPlayer());
}


// === 마우스 버튼 콜백: 클릭 처리 ===
//
// 현재는 좌클릭으로 게임 오버 화면의 "리셋 버튼"만 감지.
// HUD가 hit test 책임 지고, 여기는 클릭 이벤트 → HUD 질의 → 게임 로직 호출만.
//
// 향후: 메뉴 / 인벤토리 클릭도 같은 패턴으로 추가하면 됨.
void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        //현재 마우스 위치 가져오기 — 콜백 인자에 좌표 없으니까 직접 query
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (hud && hud->isResetButtonClicked((float)xpos, (float)ypos))
        {
            std::cout << "[리스폰] 게임 오버 버튼 클릭" << std::endl;
            RestartGame();
        }
    }
}


// === 휠 콜백: 줌 ===
void InputManager::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}


// === 윈도우 크기 변경 콜백 ===
void InputManager::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    //새 사이즈에 맞춰 OpenGL 뷰포트 갱신.
    //고DPI 디스플레이에선 width/height가 SCR_WIDTH/HEIGHT보다 클 수 있음 (이 콜백이 알려주는 값을 그대로 써야 정확)
    glViewport(0, 0, width, height);
}
