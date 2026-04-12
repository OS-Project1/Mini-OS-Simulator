#ifndef MM_CONFIG_H
#define MM_CONFIG_H

/*
 * Sanal bellek / çerçeve ayarları.
 * Derleme satırından geçersiz kılınabilir, örn.:
 *   make MM_FRAME_COUNT=8 MM_PAGE_SIZE=512
 * (Makefile bu makroları -D ile iletir.)
 */
#ifndef MM_PAGE_SIZE
#define MM_PAGE_SIZE 256
#endif

#ifndef MM_FRAME_COUNT
#define MM_FRAME_COUNT 4
#endif

#ifndef MM_MAX_PAGES
#define MM_MAX_PAGES 16
#endif

#ifndef MM_MAX_PID
#define MM_MAX_PID 32
#endif

#ifndef MM_LOAD_TICKS
#define MM_LOAD_TICKS 3
#endif

#endif
