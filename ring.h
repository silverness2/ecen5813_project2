/*========================================================================
** ring.h
** Circular buffer
** ECEN5813
**========================================================================*/

/*
 * @file ring.h
 * @brief Library declarations for ring buffer manipulation.
 *
 * @author Shilpi Gupta
 * @date March 18, 2019
 */

#ifndef RING_H
#define RING_H

typedef struct
{
    char *Buffer;
    int Length;
    int Ini;
    int Outi;
    int Adj_Len;
} ring_t;


int is_ring_valid(ring_t *ring);
int is_ring_buffer_valid(ring_t *ring);
ring_t* init(int length);
int insert(ring_t *ring, char data);
int my_remove(ring_t *ring, char *data);
int entries(ring_t *ring);
void show(ring_t *ring);
void clean(ring_t *ring);

#endif
