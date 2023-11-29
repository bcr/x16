#ifndef CELL_MEM_H
#define CELL_MEM_H

struct cell_t
{
    uint8_t type;
    uint8_t format;
    uint8_t col;
    uint8_t row;
    uint8_t contents_len;
    uint8_t* contents;
    uint8_t value_len;
    uint8_t* value;
    struct number_t number;
    struct cell_t* next;
};

typedef void (*cell_func)(struct cell_t* cell);

void c_mem_init(void);
struct cell_t* c_mem_find_cell(uint8_t col, uint8_t row, uint8_t allocate_if_not_found);
void c_mem_iterate_cells(cell_func func);
void c_mem_iterate_cells_by_row(cell_func func);

#endif /* CELL_MEM_H */
