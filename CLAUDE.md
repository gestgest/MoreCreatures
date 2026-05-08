# MoreCreatures — Claude 협업 규칙

C++/OpenGL로 만드는 오픈월드 서바이벌 게임 데모. 1인 학습 프로젝트라 코드와 주석은 **공부 기록**의 의미가 강하다는 점을 우선 인지할 것.

---

## 코드 편집 규칙 (중요)

### 1. 기존 주석은 절대 임의로 삭제하지 마라
- 한국어 설명 주석(`// ...`, `/* ... */`)은 학습/이해용으로 의도적으로 작성된 것이다.
- Edit/Write 시 변경 대상 라인 주변의 주석은 **원형 그대로** 유지할 것.
- 불필요해 보이거나 중복돼 보여도 마음대로 지우지 말 것.
- 주석 정리가 필요해 보이면 **먼저 사용자에게 물어볼 것.**
- 사용자가 명시적으로 "주석 정리해줘", "이 주석 지워줘"라고 요청한 경우에만 삭제 가능.

### 2. 주석 스타일
- 새 코드를 추가할 때 주석을 다는 건 환영이지만, **한국어로** 그리고 **WHY 위주로** 작성.
- 기존 주석의 톤(반말, 친근한 한국어 설명체)을 따를 것.
- 단순한 WHAT 설명이라도 학습 맥락에 도움된다면 남겨도 됨 (이 프로젝트의 기본 스타일).

### 3. 변경 범위 최소화
- 요청한 것만 수정. 주변 코드 리팩터링이나 정리는 자발적으로 하지 말 것.
- 버그 수정 요청에 부수적인 cleanup을 끼워 넣지 말 것.

---

## 프로젝트 구조

```
MoreCreatures/
├── MoreCreatures.sln              # Visual Studio 솔루션
├── MoreCreatures.vcxproj          # 프로젝트 파일
└── src/
    ├── main.cpp
    ├── glad.c
    ├── cpp/
    │   ├── RenderLoop.cpp         # 렌더링 메인 루프 (그림자 패스, 씬 패스)
    │   ├── Player.cpp
    │   ├── Component/             # Collider 등
    │   ├── GameObject/            # GameObject, Mouse, Ground, Terrain
    │   └── Loader/
    ├── header/
    │   ├── Config.h               # SCR_WIDTH, SCR_HEIGHT, SHADOW_MAP_SIZE 등
    │   ├── Component/, GameObject/, Loader/
    ├── vs/                        # 버텍스 셰이더 (.vs)
    │   ├── ground.vs, mouse.vs, DepthShader.vs, debugLine.vs
    └── fs/                        # 프래그먼트 셰이더 (.fs)
        ├── ground.fs, mouse.fs, DepthShader.fs, debugLine.fs
```

- 의존성: GLFW, GLAD, GLM (`include/`, `lib/`에 위치)
- 빌드: Visual Studio (Windows). PowerShell 환경.

---

## 렌더링 파이프라인 메모

- **2-pass shadow mapping**: `RenderShadowPass()` → depth FBO에 깊이만 그림 → `RenderScenePass()`에서 `shadowMap` 샘플러로 그림자 판정.
- 라이트는 **방향광(directional)** 으로 가정. `lightPos`는 위치가 아닌 "방향의 출발점"으로 사용.
- `ComputeLightSpaceMatrix()`는 **Stable Shadow Map** 기법: 박스 크기 고정 + 텍셀 단위 스냅으로 일렁임(swimming) 방지.
- `radius=30.0f` → ortho 박스 60×60 유닛. 플레이어 중심으로 따라다님.
- `RenderShadowFrustumDebug()`: 라이트 frustum을 노란 와이어박스로 시각화 (디버그용, 기본 비활성).

---

## 작업 스타일

- 답변은 **한국어**로.
- 사용자가 학습 중이므로 **왜 그런지** 설명 위주. 단순 정답 dump 지양.
- 짧고 명확하게. 불필요한 서론/요약 생략.
- 파일 위치 언급 시 `path/to/file.cpp:line` 형식 사용.
