#ifndef CONFIG_H
#define CONFIG_H

constexpr unsigned int SCR_WIDTH  = 1200;
constexpr unsigned int SCR_HEIGHT = 800;

// shadow map은 윈도우 크기와 무관 — 정사각형 고해상도가 자연스러움
constexpr unsigned int SHADOW_MAP_SIZE = 2048;

constexpr float COR = -0.33f;
constexpr float GRAVITY_ACCELERATION = -9.81f;

#endif
