# Direct IP Cleanup Design

## Goal

포트포워딩과 직접 IP 접속만 사용하는 현재 방향에 맞춰, 남아 있는 EOS 전용 코드와 설정을 프로젝트에서 제거한다.

## Scope

- `UPvPArenaOnlineSubsystem`와 관련 자동화 테스트를 제거한다.
- `PvPArena.uproject`, `PvPArena.Build.cs`, `DefaultEngine.ini`, `.gitignore`에서 EOS/OnlineServices 전용 변경을 제거한다.
- 로비 HUD는 이미 구현된 `Host Match` / `Join By IP` 흐름만 유지한다.

## Notes

- 이번 정리 후 패키징 빌드와 에디터 빌드는 직접 IP 접속 흐름만 기준으로 동작한다.
- 로컬에 남겨둔 `WindowsEngine.ini` secret 구성도 더 이상 의미가 없어진다.
