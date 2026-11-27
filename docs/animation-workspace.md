# 애니메이션 툴 화면 정리

## 목적과 변경 범위
기존 애니메이션을 확인하고 포트폴리오 영상을 찍기 쉬운 화면으로 정리했다. 재생, 보간, 블렌딩, IK 계산은 기존 구현을 사용한다.

- 왼쪽: 재생 방식, 클립 검색과 선택, 속도, 이전 / 재생·정지 / 다음.
- 가운데: 기본 링크, 회색 배경, 격자 바닥. IK 계단은 View > IK stairs에서 별도로 표시한다.
- 오른쪽: 모델 Transform, Root motion, 전환 / 키프레임, Skeleton, Meshes, IK.
- 아래: Timeline을 항상 표시한다. 기존 TrackBar 드래그와 이벤트 추가 / XML 저장·불러오기 함수를 사용한다.
- 로그와 기즈모 창은 View에서 연다. 카메라는 WASD 이동, Q/E 상하 이동, 마우스 우클릭을 누른 채 회전한다. Tab 잠금은 제거했다. 입력 필드를 편집할 때는 카메라 키 입력을 받지 않는다.

## 기본 링크
기존 기본 FBX에는 머리카락 메시가 없었다. 리소스에 있던 HeadA는 후드와 앞머리가 함께 있는 헤드다. 이 헤드를 기본 링크에만 연결하고 몸의 계산된 뼈 행렬을 따라 그리도록 했다. 일반적인 긴 머리 전체가 있는 모델을 찾은 것은 아니다.

기본 몸 모델은 82개 클립이 있는 LinkVer 2.2로 변경하고 Nml_Wait를 이름으로 선택한다. 다른 FBX에는 첫 클립을 사용한다. 이전의 고정 인덱스 56은 클립 수가 적은 모델에 맞지 않았다.

## UI 연결 보완
- 재생 버튼의 중복 정지/재생 호출을 한 번으로 정리했다.
- Space는 Timeline에서 한 번만 처리하며 텍스트 입력 중에는 실행하지 않는다.
- 길이 0인 얼굴 포즈는 Static pose로 표시해 Timeline의 나눗셈을 피한다.
- 눈금은 유지하면서 숫자 간격만 벌렸다. 시간 계산과 TrackBar 드래그 함수는 유지했다.
- 기존 ImGui DX11 바이너리가 고정 백버퍼와 창 크기의 차이를 처리하지 않아 UI 그리기 좌표만 보정했다.

## 빌드 복구
기존 저장소에 빠진 TinyXml2 소스를 추가하고 Engine 프로젝트 참조를 연결했다. 기존 Model.cpp가 쓰던 누락 멤버 선언 한 개를 복구했다. Animation.cpp, Channel.cpp, Bone.cpp, Model.cpp의 계산 코드는 수정하지 않았다.

## 확인
Debug x64 빌드와 실행, 기본 Idle와 헤드 표시, 창 크기를 줄였을 때 패널과 Timeline 배치를 확인했다. 기존 Release 구성은 ImGui가 Debug에만 연결되어 있어 이번 검증 대상에 포함하지 않았다.


## 기존 기능을 다시 사용하는 순서

| 기능 | 화면에서 조작하는 순서 |
| --- | --- |
| 한 클립 재생 | 왼쪽 Mode > Single clip, Search로 클립 검색 후 선택. Play / Pause 또는 Space. |
| 순차 재생 | Mode > Sequence (no blend), Two clips 또는 Three clips 선택. Start / Middle / End 목록을 고르고 Play sequence. 패널 아래쪽은 스크롤한다. |
| 두 클립 보간 | Mode > Crossfade > Two clips. 시작과 끝 클립, 전환 설정을 선택한 뒤 기존 재생 버튼을 누른다. 전환 XML은 상단 Load 메뉴에서 불러온다. |
| 부위별 블렌딩 | Mode > Body blending, Load blend bones XML로 해당 모델의 뼈 그룹 데이터를 불러온다. 그룹별 Source에서 클립을 선택하고 Add selected to blend로 옮긴다. 기존 그룹 설정과 재생 로직을 사용한다. |
| 루트 모션 | 오른쪽 Root motion에서 원본 재생 / Root Motion / Identity / Follow Root Bone을 선택한다. 모델에 맞는 루트 뼈 XML은 Load 메뉴에서 불러온다. |
| 뼈와 메시 | 오른쪽 Skeleton / Meshes를 펼친다. 뼈 계층 데이터가 필요한 기능은 Load의 Bone XML을 먼저 불러온다. |
| IK | 오른쪽 Inverse kinematics에서 기존 뼈 제어 또는 다리 IK 옵션을 선택한다. 계단이 필요하면 View > IK stairs를 켠다. 기존 모델별 뼈 설정은 그대로 필요하다. |
| TrackBar와 이벤트 | 아래 Timeline의 청록색 막대를 잡고 드래그한다. 기존 이벤트 추가 메뉴와 Save / Load Events XML을 그대로 사용한다. |

## 이번 표시 수정과 확인 범위

- 고정 400px 가로 목록을 패널 폭에 맞춘 세로 목록으로 바꿨다. 긴 클립 이름은 Armature 접두사를 화면에서만 생략하고 마우스를 올리면 원래 이름을 보여준다.
- Root motion 선택지는 세로로 배치했다. 콤보 이름은 입력칸 위에 표시하고, 패널 내용은 스크롤할 수 있다.
- 창 크기에 맞춰 패널 폭과 Timeline 높이를 조절한다. ImGui의 별도 OS 창 기능을 끄고 한 창에서 좌표를 관리한다.
- 기존 최종 합성은 백버퍼에 직접 출력하지만 배경은 사용하지 않는 별도 타깃에 그려지고 있었다. Priority 배경도 백버퍼에 그리도록 연결했다.
- 격자 바닥은 배경 셰이더에서 그린 시각적 기준면이다. 지형 충돌이나 IK 판정용 바닥을 새로 구현한 것은 아니다. 기존 IK 계단과 계산은 유지한다.
- 기존 Three-clip transitions 함수는 비어 있다. 동작하는 기능처럼 보이지 않도록 해당 선택에 미구현 안내를 표시했다. 2개 클립 전환과 2/3개 순차 재생 연결은 유지했다.
- Debug 빌드, 배경 렌더링, 1240×840 및 960×720 창 배치, 재생 모드 선택, Root motion 목록 표시, WASD 입력과 우클릭 회전을 확인했다. 모델별 XML을 필요로 하는 모든 IK/블렌딩 조합을 재검증한 것은 아니다.

## 순차 재생과 블렌딩 구분

Sequence (no blend)는 기존 MULTI_DEFAULT 경로다. 클립이 끝나면 Reset_CurAnim과 SetUp_Animation으로 다음 클립을 바로 시작한다. 보간을 하지 않는 것이 원래 동작이다.

Crossfade는 MULTI_INTERPOLATION 경로다. 클립이 끝나면 Change_Anim으로 다음 클립에 보간한다. Two clips에서 서로 다른 시작과 끝 클립을 선택하고, Transition settings의 Manual settings로 값을 정한 뒤 Play crossfade를 누른다. XML 설정을 쓰려면 해당 전환 데이터를 먼저 불러온다.

Body blending은 뼈 그룹별 애니메이션을 섞는 별도 기능이다. 화면의 Root Motion 선택은 루트 뼈 처리 방식이며 클립 전환 보간을 켜는 옵션이 아니다.

주의: 기존 Model의 Set_InterpolationData는 XML 로드 여부를 먼저 검사한다. 따라서 Manual settings의 수동 값도 전환 XML을 먼저 불러온 경우에만 적용되며, 미로드 시 기본 보간 값을 사용한다. 이번에는 이 계산 경로를 변경하지 않았다.
