#ifndef _XTOUCH_SD_MMC_PINS_S3_3248W535
#define _XTOUCH_SD_MMC_PINS_S3_3248W535

/*
 * JC3248W535 公式 Demo の pincfg.h（1-Demo/Demo_Arduino/DEMO_MP3/pincfg.h）より:
 *   SD_MMC_CLK 12, SD_MMC_CMD 11, SD_MMC_D0 13
 * 1bit SD_MMC（SD_MMC.begin(..., true, ...)）。
 */
#ifndef XTOUCH_SD_SCK
#define XTOUCH_SD_SCK 12
#endif
#ifndef XTOUCH_SD_CMD
#define XTOUCH_SD_CMD 11
#endif
#ifndef XTOUCH_SD_D0
#define XTOUCH_SD_D0 13
#endif
#ifndef XTOUCH_SD_D1
#define XTOUCH_SD_D1 (-1)
#endif
#ifndef XTOUCH_SD_D2
#define XTOUCH_SD_D2 (-1)
#endif
#ifndef XTOUCH_SD_D3
#define XTOUCH_SD_D3 (-1)
#endif

#endif
