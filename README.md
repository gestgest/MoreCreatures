# MoreCreatures
open world survival game with OpenGL
- 1인칭 / 3인칭 시점 캐릭터 조작과 카메라 추적, 기본 렌더링 파이프라인을 구현한 C++/OpenGL 데모 프로젝트입니다.

---
# 프로젝트 정보
### 기간
2026년 1월 14일 ~ 1월 16일 (3일)
2026년 4월 27일 ~ 5월 11일 (14일)

### 인원
1인 개발

### 개발로그
[velog에서 개발로그 보기](https://velog.io/@gestgest/series/OpenGL)

---
# 구현한 기술
### 게임 플레이
- 이동
- 1인칭/3인칭 전환
- 마우스 화면 회전
  - 화면 스크롤 줌
- 중력
- 물체 오브젝트 충돌 감지 알고리즘 (AABB 알고리즘)
- obj 로더
- 지형
  - **FBM**
  - **청크 시스템**
- UI

### 그래픽스
- **그림자, 그림자 최적화**
- TBN 행렬을 이용한 normal mapping


---
# 인 게임 플레이

### 1인칭 / 3인칭 전환
![bandicam 2026-03-05 20-48-15-421 (1)](https://github.com/user-attachments/assets/86e55647-b020-4b16-a728-cbfa34af6ad5)
<img width="1092" height="653" alt="image" src="https://github.com/user-attachments/assets/6516d90b-112c-45a9-b821-dd424f188889" />
trackingPos 벡터를 이용하여 플레이어의 앞 벡터를 구했습니다.

### 중력
![bandicam 2026-03-05 20-48-15-421 (2)](https://github.com/user-attachments/assets/c2074e8e-499e-4706-8b82-5fa7f808d810)

### AABB 알고리즘
물체 오브젝트 충돌 감지 알고리즘을 구현했습니다.

## 지형 생성
오픈월드를 구현하기 위해 FBM과 청크 시스템 기법을 사용했습니다.
### FBM, Chunk System
<img width="1196" height="797" alt="image" src="https://github.com/user-attachments/assets/b54dafcf-5454-45ef-9573-8d7d3b053403" />
- FBM : 여러 주파수의 노이즈를 겹쳐 자연스럽고 불규칙한 산맥과 평야 지형을 절차적으로 생성했습니다.
- 청크 시스템 : 맵 전체를 랜더링 하는 게 아닌 플레이어 기준으로 청크를 생성한 다음, 이동할 때 마다 지나간 청크는 언로드하고 보여질 청크는 로드했습니다.


## 그래픽스
### 그림자 매핑
빛의 시점에서 씬을 렌더링하여 깊이 맵(Depth Map)을 생성한 뒤, 이를 바탕으로 그림자를 구현했습니다.
오픈월드에 대비하기 위해 FSM(Frustum-fitted Shadow Mapping)기술을 써서 카메라 범위 내에서만 그림자를 생성하도록 구현했습니다.
<img width="639" height="436" alt="image" src="https://github.com/user-attachments/assets/6da02df1-a017-4cbf-bf63-510ce9a8504c" />


### 노멀 맵
Tangent, Bitangent, Normal 벡터를 활용해 TBN 행렬을 구성하여, 경사진 지형이나 회전된 오브젝트에서도 빛의 반사가 자연스럽게 연산되도록 구현했습니다.
<img width="1043" height="658" alt="image" src="https://github.com/user-attachments/assets/795f984d-3bc0-4617-8a1e-8f9229e198b2" /><br>
before<br><br>
<img width="861" height="574" alt="image" src="https://github.com/user-attachments/assets/e8cf0df2-704c-4bfa-8d7f-585ccdd67de4" /><br>
after<br><br>

---
# 트러블슈팅 및 최적화 경험
### 지형 Collider 구현 이슈
불규칙한 지형 특성상 BoxCollider로는 한계가 있었습니다. 어떻게 구현할까하다가 박스 풋프린트 알고리즘으로 해결했습니다.
플레이어의 서있는 위치에서 3x3의 높이중에서 가장 높은 maxHeight를 찾고 플레이어의 y 포지션을 maxHeight()로 설정했습니다.
그러니까 자연스럽게 언덕에 올라가고 내려가는 모습이 되었습니다.

### 초반 프레임 드랍 해결
원활한 청크 생성을 위해 청크 생성하는 과정을 비동기 로딩으로 해결했습니다. 단, 셰이더를 활용한 경우에는 메인으로 돌려야 하기 때문에 나머지 즉, 정점 위치를 노이즈로 생성하는 함수인 buildMeshData를 비동기로 호출했다.
- 노이즈
- 정점 노멀 계산 + tangent
- 정점 데이터 패킹
- 인덱스 버퍼
 
### 그림자 끊기는 버그
FSM 구현할때 그림자가 끊기는 현상이 있었지만 단순히 bias 값이 커서 생긴 문제였다

---
# 회고
Unity나 Unreal 같은 상용 엔진에서 마우스 클릭 몇 번으로 손쉽게 추가하던 기능들이, 내부적으로는 얼마나 복잡한 수학적 연산과 최적화 알고리즘을 거치는지 체감할 수 있었습니다.



