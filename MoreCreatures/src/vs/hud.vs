#version 330 core

// HUD는 화면 좌표(픽셀 단위)로 그리기 위해 ortho projection을 사용한다.
// gl_Position의 z는 0으로 두고, 깊이 테스트는 CPU 쪽에서 끄고 그린다.
//
// aPos: 단위 사각형(0~1) 좌표. CPU에서 iconSize/offset을 곱해서 위치 결정.
// aUV : 텍스처 좌표 (0~1).
//
// 픽셀 좌표 흐름:
//   localPos = aPos * iconSize        // 0~iconSize 픽셀 박스
//   screenPos = localPos + offset     // 화면상 픽셀 위치 (좌하단 기준)
//   gl_Position = projection * vec4(screenPos, 0.0, 1.0)
//
// projection = glm::ortho(0, SCR_WIDTH, 0, SCR_HEIGHT)
//   → 픽셀 좌표를 NDC([-1,+1])로 변환
//   → 이 ortho는 좌하단을 (0,0)으로 두기 때문에 OpenGL의 클립 공간과 자연스럽게 맞음

layout (location = 0) in vec2 aPos;  // 단위 사각형 좌표 (0~1)
layout (location = 1) in vec2 aUV;

out vec2 TexCoord;

uniform mat4 projection;   // ortho (픽셀 → NDC)
uniform vec2 offset;       // 아이콘의 화면상 좌하단 픽셀 위치
uniform float iconSize;    // 아이콘 한 변 픽셀 크기

void main()
{
    vec2 screenPos = aPos * iconSize + offset;
    gl_Position = projection * vec4(screenPos, 0.0, 1.0);
    TexCoord = aUV;
}
