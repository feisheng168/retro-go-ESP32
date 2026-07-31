/* gpSP memory arrays, split by access temperature.
 *
 *  - HOT structures live in fast internal SRAM (plain .bss). On ESP32-P4 the
 *    PSRAM goes through the L2 cache (a miss is expensive) while internal SRAM
 *    is ~single-cycle. The RISC-V load fast-path (riscv_emit.h) embeds each
 *    symbol's address at JIT-emit time, so relocating the array is transparent
 *    to the generated code -- no codegen change needed.
 *  - COLD / oversized structures stay in PSRAM via EXT_RAM_BSS_ATTR.
 *
 * Tier-1 hot set ~100 KB: iwram(64K) + memory_map_read(32K) +
 * palette_ram_converted + palette_ram + oam_ram + io_registers (~4K).
 */
#include "common.h"

#ifdef ESP_PLATFORM
#include "esp_attr.h"

/* Master switch: place the hot GBA structures in internal SRAM.
 * Set to 0 to fall back to PSRAM if internal RAM runs short -- check
 * heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL) at boot; iwram alone
 * needs a 64 KB contiguous block. */
#ifndef GPSP_HOT_IN_INTERNAL
#define GPSP_HOT_IN_INTERNAL 1
#endif

#if GPSP_HOT_IN_INTERNAL
#define HOT_BSS_ATTR              /* plain .bss -> internal SRAM */
#else
#define HOT_BSS_ATTR EXT_RAM_BSS_ATTR
#endif

/* reg[64], spsr[6], reg_mode[7][7] are defined in riscv_stub.S
   (or mips_stub.S) with guaranteed contiguous layout for asm offsets */
HOT_BSS_ATTR     u8 *memory_map_read[8 * 1024];   /* 32 KB - ROM page table, hit every ROM read */
HOT_BSS_ATTR     u16 oam_ram[512];                /*  1 KB */
HOT_BSS_ATTR     u16 palette_ram[512];            /*  1 KB */
HOT_BSS_ATTR     u16 palette_ram_converted[512];  /*  1 KB - color LUT, read per pixel */
EXT_RAM_BSS_ATTR u8 ewram[1024 * 256 * 2];        /* 512 KB - stays in PSRAM */
HOT_BSS_ATTR     u8 iwram[1024 * 32 * 2];         /* 64 KB - hottest RAM (code/stack), double for SMC sentinel */
EXT_RAM_BSS_ATTR u8 vram[1024 * 96];              /* 96 KB - PSRAM (won't fit internal: ~79 KB over) */
HOT_BSS_ATTR     u16 io_registers[512];           /*  1 KB - timers/DMA/video/IRQ */

/* From gba_memory.c */
EXT_RAM_BSS_ATTR u8 gamepak_backup[1024 * 128];

#ifdef HAVE_DYNAREC
/* Memory handler dispatch tables for dynarec (referenced by emit.h) */
/* tmemld[11][16]: load handlers, tmemst[4][16]: store handlers */
/* thnjal[15*16]: thumb handler jump table */
u32 tmemld[11][16];
u32 tmemst[4][16];
u32 thnjal[15 * 16];
#endif

#endif
