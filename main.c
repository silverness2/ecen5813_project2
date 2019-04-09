
/*========================================================================
** ring_test.c
** Circular buffer testing
** ECEN5813
**========================================================================*/

/*
 * @file ring_test.c
 * @brief A program for implementing a ring buffer. Uses CUnit for testing.
 *
 * @author Shilpi Gupta
 * @date March 18, 2019
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "ring.h"

#define MAX_NUM_RINGS 3 
#define NUM_ITERATIONS 3
#define ASCII_START_NUM 65

ring_t *ring[MAX_NUM_RINGS];
int ring_length[MAX_NUM_RINGS] = {256, 4, 1};

void transmit(ring_t *ring)
{

    for (int i = 0; i < NUM_ITERATIONS; i++)
    {
        // Insert data.
        insert(ring, ASCII_START_NUM + i);
        printf("Num entries at iteration=%d are: %d\n", i, entries(ring));
        show(ring);
    }
}

void receive(ring_t *ring)
{
    for (int i = 0; i < NUM_ITERATIONS; i++)
    {
        char c;
        my_remove(ring, &c);
        printf("Num entries at iteration=%d are: %d\n", i, entries(ring));
        show(ring);
    }
}

int main(void)
{
    for (int i = 0; i < MAX_NUM_RINGS; i++)
    {
        printf("------- For ring=%d with size=%d -------\n", i, ring_length[i]);

        // Initialize ring.
        ring[i] = init(ring_length[i]);

        // Transmit data.
        transmit(ring[i]);

        // Receive data (i.e. remove data).
        receive(ring[i]);

        // Clean ring.
        clean(ring[i]);
    }

    return (EXIT_SUCCESS);
}
