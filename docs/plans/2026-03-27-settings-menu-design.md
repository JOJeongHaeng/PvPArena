# Settings Menu Design

## Goal

패키징 빌드에서도 `Esc`로 열 수 있는 인게임 설정 메뉴를 추가해, 사용자가 볼륨과 기본 디스플레이 옵션을 직접 조절하고 게임을 종료할 수 있게 한다.

## Scope

- `Esc`로 열고 닫는 설정 오버레이를 HUD에 추가한다.
- 메뉴 항목:
  - `Resume`
  - `Master Volume`
  - `BGM Volume`
  - `SFX Volume`
  - `Window Mode`
  - `Resolution`
  - `VSync`
  - `Quit to Desktop`
- 로비와 인게임 모두 같은 HUD 설정 패널을 재사용한다.
- 그래픽 설정은 `UGameUserSettings`를 통해 적용한다.

## Notes

- 이번 1차는 성능 체감에 직접 영향 있는 옵션만 넣고, 세부 품질 옵션은 나중으로 미룬다.
- 오디오와 디스플레이 설정은 패키징 빌드에서 바로 체감 가능해야 한다.
