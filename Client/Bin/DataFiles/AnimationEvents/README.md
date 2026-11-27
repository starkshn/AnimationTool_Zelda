# 애니메이션 이벤트 데이터

LinkAnimationEvents.xml은 링크 모델의 애니메이션 이벤트 데이터다.
Nml_Move_Dash 클립에 Explosion 이벤트 7개와 Spark 이벤트 2개가 들어 있다.
SparkLeft와 Explosion의 Left는 13.23429tick, SparkRight와 Right는 17.404846tick에서 동시에 실행된다.

- Animation의 Name은 FBX 클립 이름과 정확하게 일치해야 한다. Armature| 접두사도 유지한다.
- Layer는 이벤트 트랙, Event의 Name은 이벤트 이름이다.
- TrackPos는 초가 아니라 애니메이션 타임라인의 tick 위치다.
- 현재 툴에서 Spark 레이어는 노란 타격 스파크 효과, 나머지 레이어는 폭발 효과로 미리 본다. 같은 시각이라도 각 레이어의 콜백은 별도로 실행된다.

툴 시작 시 이 파일을 자동으로 읽는다. 타임라인에서 다른 이름으로 저장하거나 이벤트 XML을 불러올 수 있다.
한 파일에 여러 클립을 담을 수 있으므로 파일명은 모델명 + AnimationEvents.xml 형식으로 정한다.
기존 AnimData 및 모델 리소스 폴더의 XML은 원본 자료이며 이 기본 파일과 별개다.
