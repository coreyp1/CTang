/**
 * @file
 *
 * Contains tests for this library's cutil allocator.
 */

#include <gtest/gtest.h>
#include <stdint.h>
#include <ghoti.io/cutil/allocator.h>
#include <ghoti.io/cutil/array.h>
#include <ghoti.io/cutil/memory.h>
#include <ghoti.io/tang/allocator.h>

using namespace std;

/*
 * The allocator is always there, and is the same one every time: containers
 * hold the pointer for as long as they live.
 */
TEST(Allocator, IsAStableSingleton) {
  const GCU_Allocator * a = gta_allocator();
  ASSERT_NE(nullptr, a);
  ASSERT_EQ(a, gta_allocator());
  ASSERT_NE(nullptr, a->malloc_fn);
  ASSERT_NE(nullptr, a->calloc_fn);
  ASSERT_NE(nullptr, a->realloc_fn);
  ASSERT_NE(nullptr, a->free_fn);
}

/*
 * A zero-size request yields a pointer rather than NULL.  A caller cannot
 * tell a NULL-for-zero apart from a NULL-for-out-of-memory, so the contract
 * cutil's own default allocator states is that NULL means failure and nothing
 * else.  An allocator that dropped that would be a subtly different one.
 */
TEST(Allocator, ZeroSizeIsNotAFailure) {
  const GCU_Allocator * a = gta_allocator();

  void * p = gcu_allocator_malloc(a, 0);
  ASSERT_NE(nullptr, p);
  gcu_allocator_free(a, p);

  void * q = gcu_allocator_calloc(a, 0, 0);
  ASSERT_NE(nullptr, q);
  gcu_allocator_free(a, q);
}

/*
 * calloc() is permitted to detect an overflowing product itself, but is not
 * required to on every platform, so the allocator checks rather than assumes.
 * Were it to wrap instead, the caller would get a block far smaller than it
 * asked for and would write off the end of it.
 */
TEST(Allocator, CallocRefusesAnOverflowingProduct) {
  const GCU_Allocator * a = gta_allocator();

  ASSERT_EQ(nullptr, gcu_allocator_calloc(a, SIZE_MAX, 2));
  ASSERT_EQ(nullptr, gcu_allocator_calloc(a, 2, SIZE_MAX));
  ASSERT_EQ(nullptr, gcu_allocator_calloc(a, SIZE_MAX / 4 + 1, 8));
}

/*
 * calloc() zeroes, and a product that does fit is honoured.
 */
TEST(Allocator, CallocZeroes) {
  const GCU_Allocator * a = gta_allocator();

  unsigned char * p = (unsigned char *)gcu_allocator_calloc(a, 16, 4);
  ASSERT_NE(nullptr, p);
  for (size_t i = 0; i < 64; ++i) {
    ASSERT_EQ(0, p[i]);
  }
  gcu_allocator_free(a, p);
}

/*
 * realloc() from NULL is an allocation, and a grow preserves what was there.
 */
TEST(Allocator, ReallocGrowsAndPreserves) {
  const GCU_Allocator * a = gta_allocator();

  char * p = (char *)gcu_allocator_realloc(a, nullptr, 4);
  ASSERT_NE(nullptr, p);
  memcpy(p, "abc", 4);

  char * q = (char *)gcu_allocator_realloc(a, p, 128);
  ASSERT_NE(nullptr, q);
  ASSERT_STREQ("abc", q);
  gcu_allocator_free(a, q);
}

/*
 * The reason this allocator exists: a buffer taken out of a cutil container
 * built on it must be releasable with gcu_free(), the way every other buffer
 * this library hands back is.  Built on cutil's default allocator instead,
 * the block would come from libc malloc() and the two would not match - which
 * is invisible until a memory-debug build reports the imbalance.
 */
TEST(Allocator, StolenArrayStorageIsReleasableWithGcuFree) {
  GCU_Array * array = gcu_array_create(1, 0, gta_allocator());
  ASSERT_NE(nullptr, array);

  const char text[] = "the quick brown fox";
  ASSERT_TRUE(gcu_array_append_n(array, text, sizeof(text)));

  size_t count = 0;
  char * stolen = (char *)gcu_array_steal(array, &count);
  gcu_array_destroy(array);

  ASSERT_NE(nullptr, stolen);
  ASSERT_EQ(sizeof(text), count);
  ASSERT_STREQ(text, stolen);
  gcu_free(stolen);
}

/*
 * Every live block is counted exactly once when it is allocated and exactly
 * once when it is released, whichever entry point it came in through.  cutil
 * does not give this for free: gcu_realloc() increments no allocation count
 * while gcu_free() always increments one, so a block first obtained by
 * reallocating from NULL - which is the only way GCU_Array ever allocates -
 * would be freed having never been counted.  The library's tests assert that
 * allocations and frees balance around every program run, and would report
 * the difference as a leak.
 */
TEST(Allocator, AllocationAndFreeCountsBalance) {
  const GCU_Allocator * a = gta_allocator();

  // A block that only ever passes through realloc().
  gcu_memory_reset_counts();
  void * p = gcu_allocator_realloc(a, nullptr, 16);
  ASSERT_NE(nullptr, p);
  p = gcu_allocator_realloc(a, p, 64);
  ASSERT_NE(nullptr, p);
  p = gcu_allocator_realloc(a, p, 256);
  ASSERT_NE(nullptr, p);
  gcu_allocator_free(a, p);
  ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());

  // Releasing nothing is not a release.
  gcu_memory_reset_counts();
  gcu_allocator_free(a, nullptr);
  ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());

  // A shrink to nothing keeps the block, so the free still has an allocation
  // to match it.
  gcu_memory_reset_counts();
  void * q = gcu_allocator_realloc(a, nullptr, 32);
  ASSERT_NE(nullptr, q);
  q = gcu_allocator_realloc(a, q, 0);
  ASSERT_NE(nullptr, q);
  gcu_allocator_free(a, q);
  ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
}

/*
 * The same, through the container that motivated it: build an array large
 * enough to grow several times, take its storage, and release both.
 */
TEST(Allocator, ArrayLifecycleBalances) {
  gcu_memory_reset_counts();

  GCU_Array * array = gcu_array_create(1, 0, gta_allocator());
  ASSERT_NE(nullptr, array);
  for (int i = 0; i < 1000; ++i) {
    ASSERT_TRUE(gcu_array_append_n(array, "0123456789", 10));
  }
  char * stolen = (char *)gcu_array_steal(array, nullptr);
  gcu_array_destroy(array);
  ASSERT_NE(nullptr, stolen);
  gcu_free(stolen);

  ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
}


int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
