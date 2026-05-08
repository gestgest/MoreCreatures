#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

// GLFW가 자체적으로 <GL/gl.h>를 포함하면 glad와 충돌함.
// GLFW_INCLUDE_NONE 정의하면 GLFW가 OpenGL 헤더를 안 끌어와서 충돌 회피.
// 이 헤더를 어떤 순서로 include해도 안전해짐.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// 모든 입력 처리를 한 곳에 모은 클래스.
// main.cpp에 흩어져 있던 콜백/폴링 코드를 추출해서 책임 분리.
//
// 특징:
//   - 모든 메소드가 static — GLFW가 C 함수 포인터를 요구하기 때문.
//   - 인스턴스 만들 필요 없음. 그냥 InputManager::xxx() 로 호출.
//   - 외부 의존(player, camera, hud, RestartGame)은 .cpp에서 extern으로 가져옴.
//
// 사용 흐름:
//   1. 윈도우 생성 후: InputManager::registerCallbacks(window)
//      → GLFW 콜백 다섯 개를 한꺼번에 등록
//   2. 매 프레임: InputManager::processInput(window, dt)
//      → WASD 같은 polled 입력 처리

class InputManager
{
public:
    // GLFW에 모든 콜백을 등록한다. 한 번만 호출하면 됨.
    static void registerCallbacks(GLFWwindow* window);

    // 매 프레임 호출 — 누르고 있는 키(WASD 등) 처리.
    // 이벤트성 입력(R, F5, - 등)은 keyCallback에서 처리.
    static void processInput(GLFWwindow* window, float deltaTime);

private:
    // GLFW 콜백 함수들 — 시그니처가 정해져 있음 (GLFW가 호출하는 형식)
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    // 마우스 첫 입력 처리 상태 — 첫 프레임의 큰 점프 방지용
    // (이전엔 main.cpp 전역이었음)
    static float lastX;
    static float lastY;
    static bool  firstMouse;
};

#endif
#pragma once
