/*-----------------------------------------------------------------------*/
/* Low level disk I/O module for FatFs (SDMMC1)                          */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "sd.h"  /* 사용자 SD API */

static uint8_t sd_rx_buf[512];
static uint8_t sd_tx_buf[512];

#define DEV_MMC  0  /* SD를 drive 0으로 사용 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status(BYTE pdrv) {
  if (pdrv != DEV_MMC) return STA_NOINIT;
  return sdIsInit() ? 0 : STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize(BYTE pdrv) {
  if (pdrv != DEV_MMC) return STA_NOINIT;
  return sdIsInit() ? 0 : STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
  if (pdrv != DEV_MMC || !sdIsInit())
    return RES_NOTRDY;

  for (UINT i = 0; i < count; i++)
  {
    if (!sdReadBlocks(sector + i, sd_rx_buf, 1, 5000))
      return RES_ERROR;

    memcpy(buff + i * 512, sd_rx_buf, 512);
  }

  return RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
  if (pdrv != DEV_MMC || !sdIsInit())
    return RES_NOTRDY;

  for (UINT i = 0; i < count; i++)
  {
    memcpy(sd_tx_buf, buff + i * 512, 512);

    if (!sdWriteBlocks(sector + i, sd_tx_buf, 1, 5000))
      return RES_ERROR;
  }

  return RES_OK;
}


#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
  if (pdrv != DEV_MMC) return RES_PARERR;

  sd_info_t info;
  switch (cmd) {
    case CTRL_SYNC: return RES_OK;
    case GET_SECTOR_COUNT:
      if (sdGetInfo(&info)) { *(DWORD*)buff = info.block_numbers; return RES_OK; }
      break;
    case GET_SECTOR_SIZE:
      if (sdGetInfo(&info)) { *(WORD*)buff = info.block_size; return RES_OK; }
      break;
    case GET_BLOCK_SIZE: *(DWORD*)buff = 1; return RES_OK;
  }
  return RES_ERROR;
}

DWORD get_fattime(void) {
  /* 2026-01-13 15:51:00 */
  return ((2026UL - 1980) << 25) | (1 << 21) | (13 << 16) | (15 << 11) | (51 << 5) | 0;
}
