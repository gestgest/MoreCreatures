#ifndef HUD_H
#define HUD_H

#include <header/shader.h>

// 화면 우하단에 식량 아이콘을 N개 그리는 HUD.
// 식량 게이지 = 아이콘 갯수로 표현 (예: maxFood=5, food=3 → 아이콘 3개만 그림).
//
// 사용 흐름:
//   HUD* hud = new HUD();
//   hud->init();           // 셰이더/VAO/텍스처 로드
//   hud->setFood(5);
//   ...
//   hud->draw();           // 매 프레임 swapBuffers 직전에 호출
//
// 구현 메모:
// - 픽셀 좌표 기반 ortho projection 사용 → 해상도 변경에 자연스럽게 대응 가능 (현재는 SCR_WIDTH/HEIGHT 고정)
// - 단위 사각형(0~1) VBO 1개 + 5번 draw call → 각 아이콘 위치는 uniform으로 전달
//   (draw call 5번이 아깝다면 인스턴싱으로 1번에 묶을 수도 있지만, HUD라 무시 가능 수준)

class HUD {
public:
    HUD();
    ~HUD();

    // 셰이더 컴파일, VAO/VBO 생성, 아이콘 텍스처 로드
    void init();

    // 현재 식량 게이지 (0 ~ maxFood). 범위 밖은 자동으로 클램프
    void setFood(int value);
    int  getFood() const { return food; }
    int  getMaxFood() const { return maxFood; }

    // HP 게이지 (0 ~ maxHp). 식량 0 도달 시 패널티로 깎임.
    void setHp(int value);
    int  getHp() const { return hp; }
    int  getMaxHp() const { return maxHp; }

    // HUD 그리기. 깊이 테스트 끄고 알파 블렌딩 켰다가 원래대로 복구함
    void draw();

private:
    Shader* shader = nullptr;
    unsigned int vao = 0;
    unsigned int vbo = 0;

    unsigned int foodIcon = 0;   // 식량 아이콘 텍스처 ID
    unsigned int hpIcon   = 0;   // HP 아이콘(하트) 텍스처 ID

    int food = 5;        // 현재 식량
    int maxFood = 5;     // 슬롯 갯수
    int hp   = 5;        // 현재 HP
    int maxHp = 5;       // HP 슬롯 갯수

    // 레이아웃 (픽셀 단위)
    float iconSize  = 48.0f;
    float spacing   =  8.0f;
    float marginX   = 16.0f;   // 화면 우측에서 떨어진 거리
    float marginY   = 16.0f;   // 화면 하단에서 떨어진 거리
};

#endif
