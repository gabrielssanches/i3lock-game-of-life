#include "gol.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

struct gol {
    int cell_nh;
    int cell_nv;
    unsigned int* cell_array;
};

struct gamectx {
    struct gol gol;
    struct {
        int width;
        int height;
    } display;
    struct {
        int size;
        int nh;
        int nv;
    } grid;
};
static struct gamectx _g;

#define CELL_ALIVE  (1 << 0)
#define CELL_BORN   (1 << 1)
#define CELL_KILL   (1 << 2)
#define CELL_GET_AGE(x)  (((x) & 0xFFFF0000) >> 16)
#define CELL_INC_AGE(x)  ((x) + 0x10000)

static void gol_create(struct gol* gol, const int ncells_horizontal, const int ncells_vertical) {
    gol->cell_nv = ncells_vertical;
    gol->cell_nh = ncells_horizontal;
    int ncells = gol->cell_nh * gol->cell_nv;
    gol->cell_array = malloc(sizeof(unsigned int) * ncells);
    memset(gol->cell_array, 0, sizeof(unsigned int) * ncells);

#if 0
    gol->cell_array[0] = CELL_ALIVE;
    gol->cell_array[gol->cell_nh -1] = CELL_ALIVE;
    gol->cell_array[((gol->cell_nv -1) * gol->cell_nh)] = CELL_ALIVE;
    gol->cell_array[(gol->cell_nv * gol->cell_nh) - 1] = CELL_ALIVE;
    gol->cell_array[2*gol->cell_nh + 2] = CELL_ALIVE;
    gol->cell_array[2*gol->cell_nh + 3] = CELL_ALIVE;
    gol->cell_array[2*gol->cell_nh + 4] = CELL_ALIVE;
#else
    for (int i = 0; i < (gol->cell_nh * gol->cell_nv); i++) {
        if ((rand() % 2) == 1) {
            gol->cell_array[i] = CELL_ALIVE;
        }
    }
#endif
}

static int gol_cell_index(struct gol* gol, const int col, const int line) {
    int col_ = (col % gol->cell_nh);
    if (col_ < 0) {
        col_ += gol->cell_nh;
    }
    int line_ = (line % gol->cell_nv);
    if (line_ < 0) {
        line_ += gol->cell_nv;
    }
    return col_ + (line_ * gol->cell_nh);
}

static bool gol_cell_is_alive_(struct gol *gol, const int col, const int line) {
    bool alive = false;
    int i = gol_cell_index(gol, col, line);
    if ((gol->cell_array[i] & CELL_ALIVE) != 0) {
        alive = true;
    }
    return alive;
}


static int gol_cell_age(struct gol* gol, const int col, const int line) {
    int i = gol_cell_index(gol, col, line);
    return CELL_GET_AGE(gol->cell_array[i]);
}

static void gol_cell_age_inc(struct gol* gol, const int col, const int line) {
    int i = gol_cell_index(gol, col, line);
    gol->cell_array[i] += 0x10000;
}

static void gol_cell_mark_kill(struct gol* gol, const int col, const int line) {
    int i = gol_cell_index(gol, col, line);
    gol->cell_array[i] |= CELL_KILL;
}

static void gol_cell_mark_born(struct gol* gol, const int col, const int line) {
    int i = gol_cell_index(gol, col, line);
    gol->cell_array[i] |= CELL_BORN;
}

static void gol_solve(struct gol* gol) {
    for (int line = 0; line < gol->cell_nv; line++) {
        for (int col = 0; col < gol->cell_nh; col++) {
            int n_alive = 0;

            if (gol_cell_is_alive_(gol, col -1, line -1)) { n_alive++; }
            if (gol_cell_is_alive_(gol, col   , line -1)) { n_alive++; }
            if (gol_cell_is_alive_(gol, col +1, line -1)) { n_alive++; }
            if (gol_cell_is_alive_(gol, col -1, line   )) { n_alive++; }
            if (gol_cell_is_alive_(gol, col +1, line   )) { n_alive++; }
            if (gol_cell_is_alive_(gol, col -1, line +1)) { n_alive++; }
            if (gol_cell_is_alive_(gol, col   , line +1)) { n_alive++; }
            if (gol_cell_is_alive_(gol, col +1, line +1)) { n_alive++; }
            

            // game of life rules https://en.wikipedia.org/wiki/Conway's_Game_of_Life#Rules
            // 1. Any live cell with fewer than two live neighbours dies, as if by underpopulation.
            // 2. Any live cell with two or three live neighbours lives on to the next generation.
            // 3. Any live cell with more than three live neighbours dies, as if by overpopulation.
            // 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
            // 5. Any live cell with that is alive for more than 100 cycles explodes and dies, giving its live to all other its neighbours.
            if (gol_cell_is_alive_(gol, col, line)) {
                gol_cell_age_inc(gol, col, line);
                if (n_alive < 2 || n_alive > 3) {
                    gol_cell_mark_kill(gol, col, line);
                } else {
                    int age = gol_cell_age(gol, col, line);
                    if (age > 100) {
                        gol_cell_mark_kill(gol, col, line);

                        gol_cell_mark_born(gol, col -1, line -1);
                        gol_cell_mark_born(gol, col, line -1);
                        gol_cell_mark_born(gol, col +1, line -1);
                        gol_cell_mark_born(gol, col -1, line);
                        gol_cell_mark_born(gol, col +1, line);
                        gol_cell_mark_born(gol, col -1, line +1);
                        gol_cell_mark_born(gol, col, line +1);
                        gol_cell_mark_born(gol, col +1, line +1);
                    }
                }
            } else {
                if (n_alive == 3) {
                    gol_cell_mark_born(gol, col, line);
                }
            }
        }
    }
    for (int i = 0; i < (gol->cell_nh * gol->cell_nv); i++) {
        if ((gol->cell_array[i] & CELL_KILL) != 0) {
            gol->cell_array[i] = 0;
        }
        if ((gol->cell_array[i] & CELL_BORN) != 0) {
            gol->cell_array[i] &= ~CELL_BORN;
            gol->cell_array[i] |= CELL_ALIVE;
        }
    }
}

bool gol_cell_is_alive(const int col, const int line) {
    return gol_cell_is_alive_(&_g.gol, col, line);
}

void gol_init(unsigned int width, unsigned int height, unsigned int *cols, unsigned int *rows, unsigned int *grid) {
    _g.display.width = width;
    _g.display.height = height;
    _g.grid.size = 5;
    _g.grid.nh = _g.display.width / _g.grid.size;
    _g.grid.nv = _g.display.height / _g.grid.size;
    gol_create(&_g.gol, _g.grid.nh, _g.grid.nv);

    *cols = _g.grid.nh;
    *rows = _g.grid.nv;
    *grid = _g.grid.size;
}

void gol_update(void) {
        gol_solve(&_g.gol);
}
