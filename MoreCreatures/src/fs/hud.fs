#version 330 core

// HUD 아이콘 프래그먼트 셰이더.
// PNG 알파 채널을 그대로 사용해야 투명 배경이 살아남.
// 알파 블렌딩은 CPU 쪽에서 glEnable(GL_BLEND) + glBlendFunc로 켜둔다.
//
// tint: 텍스처 색에 곱하는 RGBA. 슬롯 상태별로 다르게 줌.
//   가득 찬 슬롯: (1,1,1,1) → 원본 그대로
//   빈 슬롯    : (0,0,0,1) → 검은 실루엣 (투명 배경은 알파로 그대로 보존)

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D iconTex;
uniform vec4 tint;

void main()
{
    FragColor = texture(iconTex, TexCoord) * tint;
}
