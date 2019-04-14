/*******************************************************************************
 *
 * Copyright (C) 2019 by Shilpi Gupta
 *
 ******************************************************************************/

/*
 * @file ring_test.c
 * @brief A program for implementing a ring buffer. Uses CUnit for testing.
 *
 * @author Shilpi Gupta
 * @date April 13, 2019
 * @version Project
 *
 * ATTRIBUTIONS
 * CUnit code based off of example from:
 * http://cunit.sourceforge.net/example.html
 */

#include <stdio.h>
#include <stdlib.h>
#include "ring.h"
#include "CUnit/Basic.h"

#define RING_LEN 4 // test suite 1
#define MAX_NUM_RINGS 2  // test suite 2

// Global variables.
ring_t *ring; // test suite 1
ring_t *rings[MAX_NUM_RINGS]; // test suite 2
int ring_length[MAX_NUM_RINGS] = {4, 2}; // test suite 2

// TEST SUITE 1

// Return 0 on success, non-zero otherwise.
int init_suite_1()
{
    ring = init(RING_LEN);
    return 0;
}

// Return 0 on success, non-zero otherwise.
int clean_suite_1()
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

// TEST SUITE 2
// Return 0 on success, non-zero otherwise.
int init_suite_2()
{
    rings[0] = init(ring_length[0]);
    rings[1] = init(ring_length[1]);
    return 0;
}

// Return 0 on success, non-zero otherwise.
int clean_suite_2()
{
    clean(rings[0]);
    clean(rings[1]);
    return 0;
}

/* Test that multiple ring buffers can be initialized and used with the same
   functions. */
void testMULTIPLE_BUFFS_INSERT(void)
{
    CU_ASSERT(1 == insert(rings[0], 'A'));
    CU_ASSERT(1 == insert(rings[0], 'B'));
    CU_ASSERT(2 == entries(rings[0]));
    CU_ASSERT(1 == insert(rings[0], 'C'));
    CU_ASSERT(1 == insert(rings[0], 'D'));
    CU_ASSERT(4 == entries(rings[0]));

    CU_ASSERT(1 == insert(rings[1], '1'));
    CU_ASSERT(1 == insert(rings[1], '2'));
    CU_ASSERT(2 == entries(rings[1]));
    CU_ASSERT(0 == insert(rings[1], '3'));
    CU_ASSERT(0 == insert(rings[1], '4'));
    CU_ASSERT(2 == entries(rings[1]));
}

void testMULTIPLE_BUFFS_REMOVE(void)
{
    char c;
    CU_ASSERT(1 == my_remove(rings[0], &c));
    CU_ASSERT(1 == (c == 'A'))
    CU_ASSERT(3 == entries(rings[0]));
    CU_ASSERT(1 == my_remove(rings[0], &c));
    CU_ASSERT(1 == (c == 'B'))
    CU_ASSERT(1 == my_remove(rings[0], &c));
    CU_ASSERT(1 == (c == 'C'))
    CU_ASSERT(1 == my_remove(rings[0], &c));
    CU_ASSERT(1 == (c == 'D'))
    CU_ASSERT(0 == entries(rings[0]));

    CU_ASSERT(1 == my_remove(rings[1], &c));
    CU_ASSERT(1 == (c == '1'))
    CU_ASSERT(1 == entries(rings[1]));
    CU_ASSERT(1 == my_remove(rings[1], &c));
    CU_ASSERT(1 == (c == '2'))
    CU_ASSERT(0 == my_remove(rings[1], &c));
    CU_ASSERT(0 == my_remove(rings[1], &c));
    CU_ASSERT(0 == entries(rings[1]));
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
    pSuite = CU_add_suite("Single Ring Buffer, Suite 1", init_suite_1, \
                          clean_suite_1);
    pSuite = CU_add_suite("Multiple Ring Buffers, Suite 2", init_suite_2, \
                          clean_suite_2);
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
        (NULL == CU_add_test(pSuite, "test of remove from empty buffer",
                                      testREMOVE_FROM_EMPTY_BUFF)) ||
        (NULL == CU_add_test(pSuite, "test of insert to mult buffers", \
                                      testMULTIPLE_BUFFS_INSERT)) ||
        (NULL == CU_add_test(pSuite, "test of remove from mult buffers", \
                                      testMULTIPLE_BUFFS_REMOVE)))
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
