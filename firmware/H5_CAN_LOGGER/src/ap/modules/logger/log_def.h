#ifndef LOG_DEF_H_
#define LOG_DEF_H_

/**
 * @brief 로그 파일 변경 주기 (단위 : 분)
 * @note  60의 약수만 기입
 *        예 ) 1, 2, 5, 10, 15, 20, 30, 60
 */
#define LOG_ROTATE_INTERVAL_MIN 1

/**
 * @brief log file명 최대 길이
 */
#define LOG_FILE_MAX_PATH_LEN   64

// 파일명 형식
// CAN1_YYYYMMDD_HHMM_SECOND

#endif