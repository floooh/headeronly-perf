#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_ADDR_RANGE (1<<16)
#define MEM_ADDR_MASK (MEM_ADDR_RANGE-1)

#define MEM_PAGE_SHIFT (10)
#define MEM_PAGE_SIZE (1<<MEM_PAGE_SHIFT)
#define MEM_PAGE_MASK (MEM_PAGE_SIZE-1)

#define MEM_NUM_PAGES (MEM_ADDR_RANGE / MEM_PAGE_SIZE)
#define MEM_NUM_LAYERS (4)

typedef struct {
    const uint8_t* read_ptr;
    uint8_t* write_ptr;
} mem_page_t;

typedef struct {
    mem_page_t layers[MEM_NUM_LAYERS][MEM_NUM_PAGES];
    mem_page_t page_table[MEM_NUM_PAGES];
    uint8_t unmapped_page[MEM_PAGE_SIZE];
    uint8_t junk_page[MEM_PAGE_SIZE];
} mem_t;

extern void mem_init(mem_t* mem);
extern void mem_map_ram(mem_t* mem, int layer, uint16_t addr, uint32_t size, uint8_t* ptr);
extern void mem_map_rom(mem_t* mem, int layer, uint16_t addr, uint32_t size, const uint8_t* ptr);
extern void mem_map_rw(mem_t* mem, int layer, uint16_t addr, uint32_t size, const uint8_t* read_ptr, uint8_t* write_ptr);
extern void mem_unmap_layer(mem_t* mem, int layer);
extern void mem_unmap_all(mem_t* mem);
extern uint8_t* mem_readptr(mem_t* mem, uint16_t addr);
extern void mem_write_range(mem_t* mem, uint16_t addr, const uint8_t* src, int num_bytes);

extern uint8_t mem_rd(mem_t* mem, uint16_t addr);
extern void mem_wr(mem_t* mem, uint16_t addr, uint8_t data);
extern void mem_wr16(mem_t* mem, uint16_t addr, uint16_t data);
extern uint16_t mem_rd16(mem_t* mem, uint16_t addr);

#ifdef __cplusplus
} /* extern "C" */
#endif
