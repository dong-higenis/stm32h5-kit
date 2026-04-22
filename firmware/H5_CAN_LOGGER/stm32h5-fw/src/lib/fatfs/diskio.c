#include "diskio.h"
#include "sd.h"

#define DEV_SD 0

#define SD_TIMEOUT 1000

static volatile DSTATUS disk_status_flag = STA_NOINIT;


DSTATUS disk_status(BYTE physical_driver_number)
{
  if (physical_driver_number != DEV_SD)
  {
    return STA_NOINIT;
  }

  return disk_status_flag;
}


DSTATUS disk_initialize(BYTE physical_driver_number)
{
  if (physical_driver_number != DEV_SD)
  {
    return STA_NOINIT;
  }

  if (sdIsInit() == true)
  {
    disk_status_flag &= ~STA_NOINIT;
  }
  else
  {
    disk_status_flag = STA_NOINIT;
  }

  return disk_status_flag;
}


DRESULT disk_read(BYTE physical_driver_number, BYTE *read_buffer, LBA_t sector_start_num, UINT sector_count)
{
  if (physical_driver_number != DEV_SD)
  {
    return RES_PARERR;
  }

  if (disk_status_flag & STA_NOINIT)
  {
    return RES_NOTRDY;
  }

  if (read_buffer == NULL)
  {
    return RES_PARERR;
  }

  if (sdReadBlocks(sector_start_num, read_buffer, sector_count, SD_TIMEOUT) == false)
  {
    return RES_ERROR;
  }

  return RES_OK;
}


DRESULT disk_write(BYTE physical_driver_number, const BYTE *write_buffer, LBA_t sector_start_num, UINT sector_count)
{
  if (physical_driver_number != DEV_SD)
  {
    return RES_PARERR;
  }

  if (disk_status_flag & STA_NOINIT)
  {
    return RES_NOTRDY;
  }

  if (disk_status_flag & STA_PROTECT)
  {
    return RES_WRPRT;
  }

  if (write_buffer == NULL)
  {
    return RES_PARERR;
  }

  if (sdWriteBlocks(sector_start_num, (uint8_t *)write_buffer, sector_count, SD_TIMEOUT) == false)
  {
    return RES_ERROR;
  }

  return RES_OK;
}


DRESULT disk_ioctl(BYTE physical_driver_number, BYTE cmd, void *io_buff)
{
  DRESULT res = RES_ERROR;
  sd_info_t sd_info;

  if (physical_driver_number != DEV_SD)
  {
    return RES_PARERR;
  }

  if (disk_status_flag & STA_NOINIT)
  {
    return RES_NOTRDY;
  }

  switch (cmd)
  {
    case CTRL_SYNC:
      if (sdIsReady(SD_TIMEOUT) == true)
      {
        res = RES_OK;
      }
      break;

    case GET_SECTOR_COUNT:
      if (sdGetInfo(&sd_info) == true)
      {
        *(LBA_t *)io_buff = sd_info.block_numbers;
        res = RES_OK;
      }
      break;

    case GET_SECTOR_SIZE:
      if (sdGetInfo(&sd_info) == true)
      {
        *(WORD *)io_buff = sd_info.block_size;
        res = RES_OK;
      }
      break;

    case GET_BLOCK_SIZE:
      *(DWORD *)io_buff = 1;
      res = RES_OK;
      break;

    case CTRL_TRIM:
      res = RES_OK;
      break;

    default:
      res = RES_PARERR;
      break;
  }

  return res;
}


DWORD get_fattime(void)
{
  return ((DWORD)(2024 - 1980) << 25)
       | ((DWORD)1 << 21)
       | ((DWORD)1 << 16)
       | ((DWORD)0 << 11)
       | ((DWORD)0 << 5)
       | ((DWORD)0 >> 1);
}