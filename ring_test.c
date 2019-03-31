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
 * ATTRIBUTIONS
 * Example CUnit code taken from: http://cunit.sourceforge.net/example.html
 */

#include <stdio.h>
#include <stdlib.h>
#include "ring.h"
#include "CUnit/Basic.h"

#define RING_LEN 4
#define NUM_ITERATIONS 10 

ring_t *ring;

// Return 0 on success, non-zero otherwise.
int init_suite()
{
    ring = init(RING_LEN);
    return 0;
}

// Return 0 on success, non-zero otherwise.
int clean_suite()
{
    clean(ring);
    return 0;
}

/* Insert characters into ring buffer and check whether they were successfully
   inserted and that there are the correct number of entries after given
   insertions.
*/
void testINSERT(void)
{
    CU_ASSERT(1 == insert(ring, 'A'));
    CU_ASSERT(1 == insert(ring, 'B'));
    CU_ASSERT(2 == entries(ring));
    CU_ASSERT(1 == insert(ring, 'C'));
    CU_ASSERT(1 == insert(ring, 'D'));
    CU_ASSERT(4 == entries(ring));
}

/* Remove characters from ring buffer and check that the first character
   removed is the first character inserted, the second character removed is the
   second character that was inserted, and so on. Must be run after 
   testINSERT. Also test for correct number of entries after removal.*/ 
void testREMOVE(void)
{
    char c;
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(1 == (c == 'A'))
    CU_ASSERT(3 == entries(ring));
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(1 == (c == 'B'))
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(1 == (c == 'C'))
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(1 == (c == 'D'))
    CU_ASSERT(0 == entries(ring));
}

/* Insert 4 characters and then remove 2 of them, then insert 2 more to check
   that the characters removed are in the expected FIFO order. */
void testINSERT_AND_REMOVE(void)
{
    char c;
    CU_ASSERT(1 == insert(ring, 'A'));
    CU_ASSERT(1 == insert(ring, 'B'));
    CU_ASSERT(1 == insert(ring, 'C'));
    CU_ASSERT(1 == insert(ring, 'D'));
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(1 == (c == 'A'))
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(1 == (c == 'B'))
    CU_ASSERT(1 == insert(ring, 'E'));
    CU_ASSERT(1 == insert(ring, 'F'));
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(1 == (c == 'C'))
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(1 == (c == 'D'))
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(1 == (c == 'E'))
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(1 == (c == 'F'))
}

/* Test that an error is generated when inserting a character into a full
   buffer. */
void testINSERT_INTO_FULL_BUFF(void)
{
    CU_ASSERT(1 == insert(ring, 'A'));
    CU_ASSERT(1 == insert(ring, 'B'));
    CU_ASSERT(1 == insert(ring, 'C'));
    CU_ASSERT(1 == insert(ring, 'D'));
    CU_ASSERT(0 == insert(ring, 'F'));
}

/* Test that an error is generated when trying to remove a character from an
   empty buffer. */
void testREMOVE_FROM_EMPTY_BUFF(void)
{
    char c;
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(1 == my_remove(ring, &c));
    CU_ASSERT(0 == my_remove(ring, &c));
}

int main(void)
{
    // Initialize the CUnit test registry.
    if (CUE_SUCCESS != CU_initialize_registry())
    {
        return CU_get_error();
    }

    // Add a suite to the registry.
    CU_pSuite pSuite = NULL;
    pSuite = CU_add_suite("Ring Buffer Suite", init_suite, clean_suite);
    if (NULL == pSuite)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Add tests to the suite.
    if ((NULL == CU_add_test(pSuite, "test of insert", testINSERT)) ||
        (NULL == CU_add_test(pSuite, "test of remove", testREMOVE)) ||
        (NULL == CU_add_test(pSuite, "test of mix of insert and remove", \
                                            testINSERT_AND_REMOVE)) ||
        (NULL == CU_add_test(pSuite, "test of insert into full buffer",  \
                                        testINSERT_INTO_FULL_BUFF)) ||
        (NULL == CU_add_test(pSuite, "test of remove from empty buffer", \
                                      testREMOVE_FROM_EMPTY_BUFF)))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Run all tests using the CUnit Basic interface.
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

///////////////////////////////////////////////////
// Example code for main without testing framework.
/*

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
*/
//////////////////////////
