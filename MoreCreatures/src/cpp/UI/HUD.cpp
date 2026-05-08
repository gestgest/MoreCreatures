#include <UI/HUD.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Loader/Loader.h>
#include <Config.h>

#include <algorithm>


HUD::HUD() {}

HUD::~HUD()
{
    if (shader) delete shader;
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    // foodIcon 텍스처는 OpenGL이 컨텍스트 종료 시 정리 (간단히 처리)
}


void HUD::init()
{
    // 1) 셰이더 컴파일
    shader = new Shader("src/vs/hud.vs", "src/fs/hud.fs");

    // 2) 단위 사각형(0~1) VBO 생성
    //   삼각형 두 개로 사각형 만듦.
    //   pos는 (0,0)~(1,1) 단위 박스 → vs에서 iconSize/offset으로 스케일+이동.
    //   uv는 (0,0)~(1,1).
    //
    //   ※ stbi_set_flip_vertically_on_load(true) 가 켜져 있어서
    //     이미지 첫 줄(원본 위쪽)이 v=1에 매핑됨 → 화면에서 똑바로 보임.
    float verts[] = {
        // pos      // uv
        0.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 0.0f,  1.0f, 0.0f,
        1.0f, 1.0f,  1.0f, 1.0f,

        0.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 1.0f,  1.0f, 1.0f,
        0.0f, 1.0f,  0.0f, 1.0f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); //vbo

    // location 0: pos (vec2)
    glEnableVertexAttribArray(0); //vao 0번 온.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); //vbo

    // location 1: uv (vec2)
    glEnableVertexAttribArray(1); //vao 1번 온
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); //vbo

    glBindVertexArray(0);

    // 3) 아이콘 텍스처 로드 (PNG 알파 채널 포함 → Loader에서 RGBA 자동 감지)
    Loader::loadTexture(foodIcon, "textures/food_icon.png");
    Loader::loadTexture(hpIcon,   "textures/heart.png");
}


void HUD::setHp(int value)
{
    //0 ~ maxHp 범위로 클램프 — food와 동일한 패턴
    hp = std::max(0, std::min(value, maxHp));
}


void HUD::setFood(int value)
{
    // 0 ~ maxFood 범위로 클램프 — 음수/초과 들어와도 안전
    food = std::max(0, std::min(value, maxFood));
}


//UI 총괄
void HUD::draw()
{
    if (!shader || vao == 0 || foodIcon == 0 || hpIcon == 0) return;

    // === GL 상태 변경 ===
    // HUD는 항상 화면 위에 보여야 하므로 깊이 테스트 끔.
    // 알파 블렌딩 켜야 PNG 투명 배경이 살아남.
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader->use();

    // ortho projection: 픽셀 좌표 → NDC
    //   left=0, right=SCR_WIDTH, bottom=0, top=SCR_HEIGHT
    //   → 좌하단이 (0,0). HUD 좌표 계산이 직관적.
    glm::mat4 projection = glm::ortho(
        0.0f, (float)SCR_WIDTH,
        0.0f, (float)SCR_HEIGHT
    );
    shader->setMat4("projection", projection);
    shader->setFloat("iconSize", iconSize);

    // 텍스처는 항상 0번 유닛에서 샘플 (셰이더 sampler 바인딩)
    glActiveTexture(GL_TEXTURE0);
    shader->setInt("iconTex", 0);

    glBindVertexArray(vao); //여기서도 그릴 준비 => 마치 vao는 설명서같다.

    // ===========================
    // 식량 슬롯 (우하단)
    // ===========================
    //
    // 우측 끝에서부터 거꾸로 배치:
    //   가장 오른쪽 아이콘의 우측 x = SCR_WIDTH - marginX
    //   i번째(0이 가장 오른쪽) 아이콘의 좌측 x = (SCR_WIDTH - marginX) - iconSize - i*(iconSize + spacing)
    glBindTexture(GL_TEXTURE_2D, foodIcon);
    for (int i = 0; i < maxFood; ++i)
    {
        float x = (float)SCR_WIDTH - marginX - iconSize - i * (iconSize + spacing);
        float y = marginY;
        shader->setVec2("offset", x, y);

        if (i < food)
            shader->setVec4("tint", 1.0f, 1.0f, 1.0f, 1.0f);  //가득 찬 슬롯 — 원본 색
        else
            shader->setVec4("tint", 0.0f, 0.0f, 0.0f, 1.0f);  //빈 슬롯 — 검은 실루엣

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // HP 슬롯 (좌하단)
    // 왼쪽 끝에서부터 오른쪽으로 배치:
    //   i=0이 가장 왼쪽
    //   x = marginX + i * (iconSize + spacing)
    //
    // heart.png는 흰색 단색 이미지 → tint를 빨강(1,0,0,1)으로 곱해서 빨간 하트로 칠함.
    // 빈 슬롯은 어두운 빨강(0.2,0,0,1) — "사라진 자리"를 살짝 보여줌.
    glBindTexture(GL_TEXTURE_2D, hpIcon);
    for (int i = 0; i < maxHp; ++i)
    {
        float x = marginX + i * (iconSize + spacing);
        float y = marginY;
        shader->setVec2("offset", x, y);

        if (i < hp)
            shader->setVec4("tint", 1.0f, 0.0f, 0.0f, 1.0f);  //가득 찬 HP — 빨간색
        else
            shader->setVec4("tint", 0.2f, 0.0f, 0.0f, 1.0f);  //빈 HP — 어두운 빨강

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // ===========================
    // 게임 오버 시 — 화면 중앙에 큰 빨간 하트 버튼
    // ===========================
    //
    // hp == 0 일 때만 그림. 클릭하면 RestartGame()이 호출되도록 InputManager가 hit test를 함.
    // 시각적으로 "HP 다 닳았다 + 다시 살아나려면 여기 누르세요" 메세지.
    if (hp == 0)
    {
        glBindTexture(GL_TEXTURE_2D, hpIcon);

        //iconSize 임시로 키움 — 다음 프레임 시작 시 어차피 재설정됨
        shader->setFloat("iconSize", resetBtnSize);

        //중앙 정렬: 좌하단 좌표 = (화면 중앙 - 버튼 절반)
        float bx = (float)SCR_WIDTH  * 0.5f - resetBtnSize * 0.5f;
        float by = (float)SCR_HEIGHT * 0.5f - resetBtnSize * 0.5f;
        shader->setVec2("offset", bx, by);

        //순수 빨강 — "지금 누를 수 있다"는 강한 신호
        shader->setVec4("tint", 1.0f, 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);

    // === GL 상태 복원 ===
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}


// === 마우스 클릭이 리셋 버튼 영역 안인지 검사 ===
//
// GLFW 마우스 좌표는 좌상단 (0, 0) 기준 — 아래로 갈수록 Y 증가.
// HUD ortho는 좌하단 (0, 0) 기준 — 위로 갈수록 Y 증가.
// → 비교 전에 Y를 뒤집어야 함: hudY = SCR_HEIGHT - screenY
bool HUD::isResetButtonClicked(float screenX, float screenY) const
{
    if (hp != 0) return false;   //게임 오버 상태가 아니면 버튼 자체가 없음

    //HUD 좌표로 변환
    float hudX = screenX;
    float hudY = (float)SCR_HEIGHT - screenY;

    //버튼 사각형 (draw에서 쓴 것과 동일한 계산식)
    float bx = (float)SCR_WIDTH  * 0.5f - resetBtnSize * 0.5f;
    float by = (float)SCR_HEIGHT * 0.5f - resetBtnSize * 0.5f;

    //AABB hit test
    return hudX >= bx && hudX <= bx + resetBtnSize
        && hudY >= by && hudY <= by + resetBtnSize;
}
