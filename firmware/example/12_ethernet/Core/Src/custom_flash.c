#include "custom_flash.h"

#include "spi.h" // spi 사용을 위해 include

/**
 * @brief 예제에서 사용할 최소한의 커맨드를 정의해 줍시다.[SPI NOR Flash용 op-code]
 */
#define CMD_RDID      0x9F      // flash id를 얻기위한 커맨드
#define CMD_RDSR1     0x05      // Status Register 1(SR1)을 읽는 명령입니다.
#define CMD_WREN      0x06      // Write Enable 명령어, 해당 커맨드를 먼저 보내서 WEL 비트를 1로 SET해야 Erase/ Writing등이 가능
#define CMD_SE        0x20      // 4KB subsector erase (많이 씀) (4kb섹터를 통째로 지움)
#define CMD_PP        0x02      // page program(최대 256바이트를 한 "페이지"단위로 플래시에 쓰는 명령)
#define CMD_READ      0x03      // normal read (기본 SPI READ)
// #define FAST_READ_CMD 0x0B      // fast read command이지만, 해당 예제에서는 사용하지 않음.

// SR1 bit
#define SR1_WIP       0x01      // WIP(Write In Progress) 비트입니다. FLASH가 유휴중인지 작업중인지 상태 확인을 할때 사용
#define SR1_WEL       0x02      // 위의 "CMD_WREN" 에서 직접 건드릴 수 있었던 WEL 레지스터의 비트값을 확인합니다.

// SPI_CS핀의 매크로를 만들어 줍시다.
#define SPI_CS_SET_HIGH() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 1); 
#define SPI_CS_SET_LOW()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 0);

extern SPI_HandleTypeDef hspi1; // spi 객체 가져오기

/**
 * @details 해당 flash타입의 경우 (RDID: 0x9F를 쓰는 flash), 
 *          SPI 송신 후 dummy 송신을 3회 더 주어야 합니다.
 *          SPI는 송신 중, 수신이 이루어 지기 때문입니다. (정확한 유휴 횟수는 칩마다 다릅니다.)
 *          송신 1회 -> dummy(수신 1회) -> dummy(수신 2회) -> dummy (수신 3회) 로 이루어지며, 
 *          실제 확인해야할 spi data는 rx[1], rx[2], rx[3]일 것입니다.
 * 
 *          SPI를 사용하기 위해서는, SLAVE를 선택하기 위해 SLAVE와 연결된 CS선을 LOW로 내려 선택합니다.
 */
bool customFlashInit(void)
{
  // 먼저 spi메시지를 주고 받을 변수들을 생성해 줍니다.
  uint8_t spi_tx[4] = {CMD_RDID, 0, 0, 0};
  uint8_t spi_rx[4] = {0};

  // flash cs high
  SPI_CS_SET_HIGH();
  HAL_Delay(10); // 안정화

  // flash cs low
  SPI_CS_SET_LOW();
  // 이제 송신이 가능합니다.
  HAL_SPI_TransmitReceive(&hspi1, spi_tx, spi_rx, 4, 100); // slave의 id를 물어봅시다. 4번 송신하고 4번 받습니다. (블로킹)
  // flash cs high : slave 해제
  SPI_CS_SET_HIGH();

  // 이제 slave에게서 받은 flash id를 조회합시다.
  cliPrintf("FLASH ID: %02X %02X %02X\r\n", spi_rx[1], spi_rx[2], spi_rx[3]);

  // 데이터 시트에 정의된대로 정확히 아래와 같은 3바이트의 메시지가 왔다면
  if (spi_rx[1] == 0xEF && spi_rx[2] == 0x40 && spi_rx[3] == 0x19)
  {
    cliPrintf("FLASH INITED\n");
    return true;
  }
  else
  {
    cliPrintf("FLASH INIT FAILED\n");
    return false;
  }
}

/**
 * @brief Flash의 특정 영역을 읽는 함수
 * @param addr: 읽을 주소
 * @param read_data: 읽은 데이터를 저장할 버퍼
 * @param length: 읽을 길이
 * @return true: 성공, false: 실패
 */
bool customFlashRead(uint32_t addr, uint8_t *read_data, uint32_t length)
{
  // 시작 주소로부터 사용자가 지정한 길이만큼의 메모리 주소까지 읽기 위해 length를 더한다.
  uint8_t tx_buf[4 + length];
  uint8_t rx_buf[4 + length];

  // 읽기 명령 구성
  tx_buf[0] = CMD_READ;            // 읽기 커맨드
  tx_buf[1] = (addr >> 16) & 0xFF; // 주소 상위 바이트
  tx_buf[2] = (addr >> 8) & 0xFF;  // 주소 중간 바이트
  tx_buf[3] = addr & 0xFF;         // 주소 하위 바이트

  // SCLK핀에 클럭을 계속 흘리기 위한 Dummy데이터는 0으로 채운다. (MOSI 송신 -> SCLK 도 같이 흐르기 때문)
  for (uint32_t i = 0; i < length; i++)
  {
    tx_buf[4 + i] = 0;
  }
  
  // spi 송수신
  SPI_CS_SET_LOW();
  HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 4 + length, 100);
  SPI_CS_SET_HIGH();

  // 수신된 데이터 복사 (커맨드와 주소 3바이트 이후부터가 실제 데이터) 
  // tx_buf[0](cmd), tx_buf[1], tx_buf[2], tx_buf[3](addr) 가 보내지고 난 후 그 후부터 첫 데이터가 들어오기 때문임.
  // HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 4 + length, 100); 해당 함수 내부에서 tx버퍼와 rx버퍼의 인덱스는 같이 증가한다.
  for (uint32_t i = 0; i < length; i++)
  {
    read_data[i] = rx_buf[4 + i]; // 길이 만큼 읽어서 인자의 read_data 버퍼에 저장한다.
  }

  return true;
}

/**
 * @brief Flash의 특정 주소에 데이터를 쓰는 함수
 * @param addr: 쓸 주소
 * @param write_data: 쓸 데이터 버퍼
 * @param length: 쓸 길이 (최대 256 바이트, 페이지 크기)
 * @return true: 성공, false: 실패
 * @note spi nor flash는 한번에 한페이지 단위로 쓸 수 있다. 1 page = 256bytes 
 */
bool customFlashWrite(uint32_t addr, uint8_t *write_data, uint32_t length)
{
  uint8_t tx_buf[4 + length];

  // 페이지 크기 초과 체크
  if (length > 256 || length == 0)
  {
    cliPrintf("ERROR: Invalid page length (max 256)\n");
    return false;
  }
  
  uint8_t write_en_cmd = CMD_WREN;

  // CMD_WREN 를 송신하여 writing을 알린다.
  SPI_CS_SET_LOW();
  HAL_SPI_Transmit(&hspi1, &write_en_cmd, 1, 100);
  SPI_CS_SET_HIGH();

  // Page Write 명령 구성
  tx_buf[0] = CMD_PP;             // Page Program 커맨드
  tx_buf[1] = (addr >> 16) & 0xFF;// 이하는 주소 3바이트
  tx_buf[2] = (addr >> 8) & 0xFF;
  tx_buf[3] = addr & 0xFF;
   
  // 쓸 데이터 복사
  for (uint32_t i = 0; i < length; i++)
  {
    tx_buf[4 + i] = write_data[i];
  }
  
  // 송신
  SPI_CS_SET_LOW();
  HAL_SPI_Transmit(&hspi1, tx_buf, 4 + length, 100);
  SPI_CS_SET_HIGH();
  
  while(customFlashReadStatus() & SR1_WIP) // flash의 busy상태가 끝날때 까지 대기합니다.
  {
  }

  return true;
}

/**
 * @brief Flash의 특정 섹터(4KB)를 지우는 함수
 * @param addr: 지울 섹터의 주소 (4KB 정렬)
 * @return true: 성공, false: 실패
 * @note flash를 erase하는건 writing과는 행위가 다름
 *       erase 회로는 최소 섹터단위로 행해야 하는데, writing이 셀단위로 이루어지는것과는 상반됨
 *       이는 최소단위인 섹터별로 셀들이 erase회로에 묶여있기 때문.
 *       따라서 addr은 반드시 0x1000단위로 행해야 한다는걸 명심합시다.
 */
bool customFlashSectorErase(uint32_t addr)
{
  if (addr % 0x1000 != 0) // 섹터 단위의 인자가 아니면 false 반환
  {
    return false;
  }

  uint8_t tx_buf[4];

  uint8_t write_en_cmd = CMD_WREN;

  // CMD_WREN 를 송신하여 writing을 알린다.
  SPI_CS_SET_LOW();
  HAL_SPI_Transmit(&hspi1, &write_en_cmd, 1, 100);
  SPI_CS_SET_HIGH();

  // Sector Erase 명령 구성
  tx_buf[0] = CMD_SE;             // Sector Erase 커맨드
  tx_buf[1] = (addr >> 16) & 0xFF;
  tx_buf[2] = (addr >> 8) & 0xFF;
  tx_buf[3] = addr & 0xFF;

  SPI_CS_SET_LOW();
  HAL_SPI_Transmit(&hspi1, tx_buf, 4, 100);
  SPI_CS_SET_HIGH();
  
  while(customFlashReadStatus() & SR1_WIP) // flash의 busy상태가 끝날때 까지 대기합니다.
  {
    // 딜레이는 
  }

  return true;
}

/**
 * @brief Flash의 Status Register 1을 읽는 함수
 * @return SR1 값 (flash busy여부 체크 가능)
 */
uint8_t customFlashReadStatus(void)
{
  uint8_t tx_buf[3] = {CMD_RDSR1, 0};
  uint8_t rx_buf[3] = {0};

  SPI_CS_SET_LOW();
  HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 2, 100);
  SPI_CS_SET_HIGH();

  cliPrintf("rx: %d\n",rx_buf[1]);
  return rx_buf[1];
}

/**
 * @brief SPI Flash CLI 명령어 핸들러
 */
void cliFlash(cli_args_t *args)
{
  uint32_t addr;
  uint32_t length;
  uint8_t read_buf[256];
  uint8_t write_buf[256];
  bool ret;

  if (args->argc == 3 && args->isStr(0, "read"))
  {
    addr = args->getData(1);
    length = args->getData(2);

    if (length > 256)
    {
      cliPrintf("Error: 최대 read 길이는 256을 넘을 수 없습니다.\n");
      return;
    }

    ret = customFlashRead(addr, read_buf, length);
    if (ret)
    {
      cliPrintf("Read from 0x%06X (length: %d)\n", addr, length);
      for (uint32_t i = 0; i < length; i++)
      {
        if (i % 16 == 0) // 한 행당 16개를 출력
        {
          cliPrintf("0x%06X: ", addr + i);
        }
        cliPrintf("%02X ", read_buf[i]);
        if (i % 16 == 15)
        {
          cliPrintf("\n");
        }
      }
      if (length % 16 != 0) // 마지막 데이터가 출력되고 난 후 가독성을 위해 개행 출력
      {
        cliPrintf("\n");
      }
    }
    else
    {
      cliPrintf("읽기 실패\n");
    }
  }
  else if (args->argc >= 4 && args->isStr(0, "write"))
  {
    addr = args->getData(1);
    length = args->getData(2);

    if (length > 256 || length == 0)
    {
      cliPrintf("Error: 최대 write 길이는 256을 넘을 수 없습니다.\n");
      return;
    }

    // 나머지 인자들을 데이터로 받기
    if (args->argc < (3 + length))
    {
      cliPrintf("Error: 설정한 길이에 대한 데이터 수가 부족합니다.\n");
      cliPrintf("cli.h에서 인자 최대수를 확인 해주세요.\n");
      return;
    }
    
    for (uint32_t i = 0; i < length; i++)
    {
      write_buf[i] = args->getData(3 + i) & 0xFF;
    }

    ret = customFlashWrite(addr, write_buf, length);
    if (ret)
    {
      cliPrintf("라이팅 결과 (addr: 0x%06X, length: %d)\n", addr, length);
    }
    else
    {
      cliPrintf("라이팅 실패\n");
    }
  }
  else if (args->argc == 2 && args->isStr(0, "erase"))
  {
    addr = args->getData(1);

    ret = customFlashSectorErase(addr);
    if (ret)
    {
      cliPrintf("섹터 지우기 완료 (0x%06X)\n", addr);
    }
    else
    {
      cliPrintf("지우기 실패\n");
    }
  }
  else
  {
    cliPrintf("flash read [addr] [length]\n");
    cliPrintf("flash write [addr] [length] [data0] [data1] ...\n");
    cliPrintf("flash erase [addr]\n");
  }
}
