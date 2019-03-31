
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
#include "CUnit/Basic.h"

#define RING_LEN 4
#define NUM_ITERATIONS 10 

int main(void)
{
    // Initialize ring.
    ring_t *ring = init(RING_LEN);

    // Mimic transmit and receive by inserting and removing data.
    for (int i = 0; i < NUM_ITERATIONS; i++)
    {
        printf("-------- At iteration=%d --------\n", i);
        // Insert data.
        insert(ring, 65 + i);

        show(ring);

        // Remove data.
        if ((i > 4 && i < 8))
        {
            char c;
            my_remove(ring, &c);
            printf("main(): Removed char=%c\n", c);

            my_remove(ring, &c);
            printf("main(): Removed char=%c\n", c);
        }
    }

    clean(ring);

    return (EXIT_SUCCESS);
}
