/*========================================================================
** Circular buffer
** ECEN5813
**========================================================================*/

/*
 * @file ring.c
 * @brief Library definitions for ring buffer manipulation.
 *
 * @author Shilpi Gupta
 * @date March 18, 2019
 *
 * ATTRIBUTIONS
 * Power of 2 check in init() taken from:
 * https://www.exploringbinary.com/ten-ways-to-check-if-an-integer-is-a-power-of-two-in-c/
 */

#include "ring.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MSG_EMPTY_RING "ERROR: Ring is NULL.\n"
#define MSG_EMPTY_RING_BUFFER "ERROR: Ring buffer is NULL.\n"

int is_ring_valid(ring_t *ring)
{
    if (ring == NULL)
    {
        printf(MSG_EMPTY_RING);
        return 0;
    }
    return 1;
}

int is_ring_buffer_valid(ring_t *ring)
{
    if (ring->Buffer == NULL)
    {
        printf(MSG_EMPTY_RING_BUFFER);
        return 0;
    }
    return 1;
}

ring_t* init(int length)
{
    // Confirm length is a power of 2.
    if (!((length != 0) && !(length & (length - 1))))
    {
        printf("init(): ERROR: Length of ring must be a power of 2.\n");
        exit(EXIT_FAILURE);
    }

    // Alloc and verify ring.
    ring_t *ring = malloc(sizeof(ring_t));
    if (!is_ring_valid(ring)) { exit(EXIT_FAILURE); }

    // Alloc and verify ring buffer.
    ring->Buffer = malloc(length * sizeof(char));
    if (!is_ring_buffer_valid(ring)) { exit(EXIT_FAILURE); }

    // Set ring parameters.
    ring->Length = length;
    ring->Ini = 0;
    ring->Outi = 0;
    ring->Adj_Len = (1 << (int)(log(length)/log(2))) - 1;

    // Return ring.
    return ring;
}

int insert(ring_t *ring, char data)
{
    // Verify.
    if (!is_ring_valid(ring) || !is_ring_buffer_valid(ring))
    { exit(EXIT_FAILURE); }

    // Insert.
    if ((ring->Ini - ring->Outi) < ring->Length)
    {
        ring->Buffer[ring->Ini++ & ring->Adj_Len] = data;

        printf("insert(): Inserted char=%c, Ini now=%d\n", data, ring->Ini);
        return 1;
    }
    return 0;
}

int my_remove(ring_t *ring, char *data)
{
    // Verify.
    if (!is_ring_valid(ring) || !is_ring_buffer_valid(ring))
    { exit(EXIT_FAILURE); }

    // Remove.
    if (ring->Outi != ring->Ini)
    {
        *data = ring->Buffer[ring->Outi++ & ring->Adj_Len];
        printf("my_remove(): Removed char=%c, Outi now=%d\n", *data, ring->Outi);
        return 1;
    }
    return 0;
}

int entries(ring_t *ring)
{
    // Verify.
    if (!is_ring_valid(ring)) { exit(EXIT_FAILURE); }

    return (ring->Ini - ring->Outi);
}

// For debugging.
void show(ring_t *ring)
{
    // Verify.
    if (!is_ring_valid(ring) || !is_ring_buffer_valid(ring))
    { exit(EXIT_FAILURE); }

    // Print content of buffer.
    for (int i = 0; i < ring->Length; i++)
    {
        printf("At [%d], value is: %c\n", i, ring->Buffer[i]);
    }
}

void clean(ring_t *ring)
{
    // Verify.
    if (!is_ring_valid(ring) || !is_ring_buffer_valid(ring))
    { exit(EXIT_FAILURE); }

    // Free memory.
    free(ring->Buffer);
    ring->Buffer = NULL;
    free(ring);
    ring = NULL;
}
