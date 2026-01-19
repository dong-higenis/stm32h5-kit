#include "can_game.h"

static void canMainDraw(void);
static void protocolSaveDraw(void);;
static void canMonitorDraw(void);
static void canGameDraw(void);
static void canGameOverDraw(void);
static void canTimeDraw(void);
static void gameInit(void);
static void drawByteBox(int x, int y, uint8_t value);

typedef enum
{
  MENU_MONITOR,
  MENU_GAME,
  MENU_TIME,
  MENU_SAVE,
  MENU_COUNT
} menu_definer_t;

typedef enum
{
  STATE_MENU,
  STATE_MONITOR,
  STATE_GAME,
  STATE_TIME,
  STATE_SAVE,
  STATE_GAME_OVER
} screen_state_t;

typedef struct
{
  int16_t x;
  int16_t y;
  bool    active;
} bullet_t;

#define MAX_BULLETS 10

static screen_state_t   cur_screen = STATE_MENU;
static uint8_t          menu_index = 0;
static int16_t          scroll_y = 0;

/* 게임 변수 */
static int16_t  player_x = 64;
static int16_t  player_y = 20;
static bullet_t bullets[MAX_BULLETS];
static uint32_t game_start_time = 0;
static uint32_t last_bullet_spawn = 0;

/* 세이브 파일 */
char save_lines[20][64];  // 한 화면에 20줄, 각 64바이트

int scroll_offset = 0;
static bool can_button_selected = false;

static int log_index = 0;

static uint32_t logtime     = 0;
static bool     is_captured = false;
static uint32_t captured_num = 0;


void canToyInit(void)
{
  canAppInit();
  shutterInit();
}

void canToyUpdate(uint8_t ch)
{
    can_msg_t msg;

    if (canMsgAvailable(ch))
    {
        canMsgRead(ch, &msg);
//        canAppOnRx(&msg);
        screenOnButton(msg.id);   // UI에게 버튼 이벤트 전달
    }

    screenDraw();   // 항상 현재 화면만 그림
}

//void canToyUpdate(uint8_t ch)
//{
//  can_msg_t msg;
//
//  /* ===== CAN 메시지 없을 때: 화면 갱신 ===== */
//  if (!canMsgAvailable(ch))
//  {
//    static uint32_t draw_time = 0;
//    if (millis() - draw_time > 50)
//    {
//      draw_time = millis();
//
//      screenDraw();
//    }
//    return;
//  }
//
//  canMsgRead(ch, &msg); // can 읽기
//
//  canAppOnRx(&msg); // can_app 에 갱신
//
//  /* 이제 UI 이벤트 처리 */
//  if (msg.id == 0x121)
//  {
//      if (cur_screen == STATE_MONITOR)
//      {
//          const can_frame_t* latest_can = canAppGetLatest();
//          takePicture(latest_can);
//
//          captured_num++;
//          is_captured = true;
//          logtime = millis();
//      }
//    else if (cur_screen == STATE_MENU)
//    {
//      // 메뉴 선택 → 해당 화면으로 전환
//      switch (menu_index)
//      {
//        case MENU_MONITOR: cur_screen = STATE_MONITOR; break;
//        case MENU_GAME:    cur_screen = STATE_GAME; gameInit(); break;
//        case MENU_TIME:    cur_screen = STATE_TIME; break;
//        case MENU_SAVE:
//          cur_screen = STATE_SAVE;
//          loadCanTxt();  // 파일 로드
//          break;
//      }
//    }
//    else if (cur_screen == STATE_SAVE)
//    {
//      protocolSaveDraw();
//    }
//    else if (cur_screen == STATE_GAME)
//    {
//      // GAME: 플레이어 오른쪽 이동
//      player_x += 8;
//      if (player_x > 124) player_x = 4;
//      gameUpdate();
//      canGameDraw();
//    }
//
//    return;
//  }



// }
/* ===================================================== */
/* ==================== GAME =========================== */
/* ===================================================== */

void gameInit(void)
{
  player_x = 64;
  player_y = 50;  //
  game_start_time = millis();
  last_bullet_spawn = millis();

  /* 총알 초기화 */
  for (int i = 0; i < MAX_BULLETS; i++)
  {
    bullets[i].active = false;
  }
}

void gameUpdate(void)
{
  /* 총알 생성 (200ms마다) */
  if (millis() - last_bullet_spawn > 200)
  {
    last_bullet_spawn = millis();

    /* 비활성 총알 찾아서 생성 */
    for (int i = 0; i < MAX_BULLETS; i++)
    {
      if (!bullets[i].active)
      {
        bullets[i].x = (millis() * 13) % 124 + 2;  // 랜덤 x 위치
        bullets[i].y = 0;
        bullets[i].active = true;
        break;
      }
    }
  }

  /* 총알 이동 */
  for (int i = 0; i < MAX_BULLETS; i++)
  {
    if (bullets[i].active)
    {
      bullets[i].y += 2;

      /* 화면 밖으로 나가면 비활성화 */
      if (bullets[i].y > 64)
      {
        bullets[i].active = false;
      }

      /* 충돌 체크 (플레이어: 반경 3, 총알: 반경 2) */
      int dx = bullets[i].x - player_x;
      int dy = bullets[i].y - player_y;
      if (dx*dx + dy*dy < 25)  // (3+2)^2 = 25
      {
        cur_screen = STATE_GAME_OVER;
        return;
      }
    }
  }
}

void canGameDraw(void)
{
  if (!lcdDrawAvailable()) return;

  gameUpdate();

  lcdClearBuffer(black);
  lcdSetFont(LCD_FONT_05x08);

  lcdDrawFillCircle(player_x, player_y, 3, white);

  /* Bullets */
  for (int i = 0; i < MAX_BULLETS; i++)
  {
    if (bullets[i].active)
    {
      lcdDrawFillCircle(bullets[i].x, bullets[i].y, 2, white);
    }
  }

  /* UI - 위치 조정 */
  lcdPrintf(0, 0, white, "TIME:%lds", (millis() - game_start_time) / 1000);
  lcdPrintf(60, 0, white, "X:%d", player_x);  // 디버그용

  lcdRequestDraw();
}

void canGameOverDraw(void)
{
  if (!lcdDrawAvailable()) return;

  lcdClearBuffer(black);
  lcdSetFont(LCD_FONT_05x08);

  /* 게임오버 메시지 */
  lcdPrintf(30, 20, white, "CAN LOSE");

  uint32_t game_time = (millis() - game_start_time) / 1000;
  lcdPrintf(20, 35, white, "Time: %lds", game_time);

  lcdPrintf(10, 50, white, "Press 0x120: Menu");

  lcdRequestDraw();
}

/* ===================================================== */
/* ==================== TIME =========================== */
/* ===================================================== */

void canTimeDraw(void)
{
  if (!lcdDrawAvailable()) return;

  lcdClearBuffer(black);
  lcdSetFont(LCD_FONT_05x08);

  uint32_t sec = millis() / 1000;
  uint32_t min = sec / 60;
  uint32_t hour = min / 60;

  sec %= 60;
  min %= 60;

  lcdPrintf(20, 0, white, "SYSTEM TIME");
  lcdPrintf(10, 20, white, "%02ld:%02ld:%02ld", hour, min, sec);

  /* 하단 안내 */
  lcdPrintf(0, 56, white, "Press 0x122: Menu");

  lcdRequestDraw();
}


