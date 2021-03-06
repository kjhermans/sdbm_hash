/*
** Copyright 2015 K.J. Hermans (kees@pink-frog.com)
** This code is part of simpledbm, an API to a dbm on a finite resource.
** License: BSD
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "hd_private.h"

/*
static
int hd_sort_empties
  (hd_t* hd, unsigned level)
{
  unsigned ptr1 = hd->header.off_e, last = ptr1, ptr2;
  struct chunkhead head1, head2;
  if (ptr1) {
    CHECK(hd_read_chunkhead(hd, ptr1, &head1));
    ptr2 = head1.next;
    while (ptr2 && level) {
      CHECK(hd_read_chunkhead(hd, ptr2, &head2));
      if (ptr1 > ptr2) {
        unsigned next = head2.next;
        if (ptr1 == last) {
          hd->header.off_e = ptr2;
        } else {
          CHECK(hd_write_uint(hd, last, ptr2));
        }
        CHECK(hd_write_uint(hd, ptr2, ptr1));
        CHECK(hd_write_uint(hd, ptr1, next));
        --level;
        last = ptr2;
      } else {
        last = ptr1;
        ptr1 = ptr2;
        head1 = head2;
      }
      ptr2 = head2.next;
    }
  }
  return 0;
}
*/

static
int hd_sort_empties_from
  (hd_t* hd, unsigned level, unsigned orig, unsigned elt1, unsigned elt2)
{
  unsigned list[ level ], length = 1;
  unsigned ptr = elt2, last = 0;
  list[0] = elt1;
  while (length < level && ptr) {
    list[length++] = ptr;
    CHECK(hd_read_uint(hd, ptr, &last));
    ptr = last;
  }
  if (length < 2) {
    return 0;
  }
  hd_qsort(list, length);
  if (orig == 0) {
    hd->header.off_e = list[0];
  } else {
    CHECK(hd_write_uint(hd, orig, list[0]));
  }
  orig = list[0];
  unsigned i;
  for (i=1; i < length; i++) {
    CHECK(hd_write_uint(hd, list[i-1], list[i]));
  }
  CHECK(hd_write_uint(hd, list[i-1], last));
  return 0;
}

static
int hd_sort_empties
  (hd_t* hd, unsigned level)
{
  if (level < 2) {
    return 0;
  }
  unsigned ptr = hd->header.off_e, orig = 0;
  while (ptr) {
    unsigned next;
    CHECK(hd_read_uint(hd, ptr, &next));
    if (next && next < ptr) {
      CHECK(hd_sort_empties_from(hd, level, orig, ptr, next));
      return 0;
    }
    orig = ptr;
    ptr = next;
  }
  return 0; /* sorted already */
}

static
int hd_merge_empties
  (hd_t* hd, unsigned level)
{
  unsigned ptr1 = hd->header.off_e, ptr2;
  struct chunkhead head1, head2;
  if (ptr1) {
    CHECK(hd_read_chunkhead(hd, ptr1, &head1));
    ptr2 = head1.next;
    while (ptr2 && level) {
      CHECK(hd_read_chunkhead(hd, ptr2, &head2));
      if (ptr1 + head1.size == ptr2) {
        head1.size += head2.size;
        head1.next = head2.next;
        CHECK(hd_write_chunkhead(hd, ptr1, &head1));
        --level;
        --(hd->header.nempties);
      } else {
        ptr1 = ptr2;
        head1 = head2;
      }
      ptr2 = head2.next;
    }
  }
  return 0;
}

/**
 * \ingroup hashtable_private
 *
 * Defragments the resource, both alignment of empty chunks and fragmented
 * values.
 *
 * \param hd Non-NULL pointer to an initialized hd_t structure.
 *
 * \returns Zero on success, or non-zero on error.
 */
int hd_defrag
  (hd_t* hd)
{
  if (!(hd->header.nentries)) {
    return 0;
  }
  double entryf = (double)(hd->header.nchunks) / (double)(hd->header.nentries);
  double emptyf = (double)(hd->header.nempties) / (double)(hd->header.nentries);
  unsigned level = (unsigned)(entryf + emptyf + 0.5);
  CHECK(hd_sort_empties(hd, level));
  CHECK(hd_merge_empties(hd, level));
  return 0;
}

#ifdef __cplusplus
}
#endif
