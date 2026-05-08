#version 330 core

// HUD 아이콘 프래그먼트 셰이더.
// PNG 알파 채널을 그대로 사용해야 투명 배경이 살아남.
// 알파 블렌딩은 CPU 쪽에서 glEnable(GL_BLEND) + glBlendFunc로 켜둔다.

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D iconTex;

void main()
{
    FragColor = texture(iconTex, TexCoord);
}
