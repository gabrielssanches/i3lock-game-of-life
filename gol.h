#ifndef GOL_H_
#define GOL_H_

#include <stdbool.h>
bool gol_cell_is_alive(const int col, const int line);
void gol_init(unsigned int width, unsigned int height, unsigned int *cols, unsigned int *rows, unsigned int *grid);
void gol_update(void);
#endif // GOL_H_
