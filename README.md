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
<img width="1199" height="831" alt="image" src="https://github.com/user-attachments/assets/fe6f10bb-b866-4a09-b876-1c9ffeac57dd" />

### 이동
![bandicam 2026-03-05 20-48-15-421](https://github.com/user-attachments/assets/a1c35149-55b7-439c-a5f7-c1cf037c5414)

### 1인칭 / 3인칭 전환
![bandicam 2026-03-05 20-48-15-421 (1)](https://github.com/user-attachments/assets/86e55647-b020-4b16-a728-cbfa34af6ad5)
<img width="1092" height="653" alt="image" src="https://github.com/user-attachments/assets/6516d90b-112c-45a9-b821-dd424f188889" />
trackingPos 벡터를 이용하여 플레이어의 앞 벡터를 구했습니다.

### 중력
![bandicam 2026-03-05 20-48-15-421 (2)](https://github.com/user-attachments/assets/c2074e8e-499e-4706-8b82-5fa7f808d810)

### AABB 알고리즘
물체 오브젝트 충돌 감지 알고리즘

## UI
<img width="1197" height="800" alt="image" src="https://github.com/user-attachments/assets/4efd8327-c1e4-4938-9282-3b412a59d7af" />

### FBM, Chunk System
<img width="1196" height="797" alt="image" src="https://github.com/user-attachments/assets/b54dafcf-5454-45ef-9573-8d7d3b053403" />


## 그래픽스
### 그림자 매핑
<img width="639" height="436" alt="image" src="https://github.com/user-attachments/assets/6da02df1-a017-4cbf-bf63-510ce9a8504c" />


### 노멀 맵
TBN 행렬을 이용해 경사진 지형에서도 자연스러운 텍스쳐 재질을 구현했다.
<img width="1043" height="658" alt="image" src="https://github.com/user-attachments/assets/795f984d-3bc0-4617-8a1e-8f9229e198b2" /><br>
before<br><br>
<img width="861" height="574" alt="image" src="https://github.com/user-attachments/assets/e8cf0df2-704c-4bfa-8d7f-585ccdd67de4" /><br>
after<br>

---
# 트러블슈팅 및 최적화 경험
- 울퉁불퉁한 청크 지형을 구현할때 Collider 구현이 힘들었습니다.
- 원활한 청크 생성을 위한 비동기 로딩

---
# 회고
상용엔진에서는 기본적인 기능이 여기선 너무 복잡하다.


