# STM32H5-KIT LED Blink Example

이 예제는 500ms마다 LED를 깜빡이도록(Blink) 동작하는 가장 기본적인 예제입니다.

---

## 개발 환경

- **MCU**: STM32H563-kit(Higenis)
- **IDE**: STM32CubeIDE
- **언어**: C
- **드라이버**: HAL Driver 기반

---

## 동작 원리

`main()` 함수의 `while` 루프에서 현재 시간과 이전 시간을 비교하여 500ms가 지나면 LED 상태를 반전시킵니다.

