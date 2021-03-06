/*
** Copyright 2015 K.J. Hermans (kees@pink-frog.com)
** This code is part of simpledbm, an API to a dbm on a finite resource.
** License: BSD
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "hd_private.h"

static
int hd_init_create
  (hd_t* hd, unsigned flags)
{
  unsigned buckets[] = { 0, 0, 0, 0, 0 };
  struct chunkhead emptychunk = {
    0,                          /* no next chunk */
    hd->header.size - (
      sizeof(struct hd_header) +
      (5 * sizeof(unsigned))
    )
  };
  struct hd_header header = {
    { 'h', 'd', 'i', '\0' },    /* magic */
    5,                          /* 5 buckets */
    flags,                      /* flags */
    hd->header.size,            /* our space */
    sizeof(struct hd_header),   /* bucket list starts just beyond us */
    sizeof(struct hd_header) + (5 * sizeof(unsigned)),
    0,                          /* scn */
    0,                          /* 0 entries */
    1,                          /* 1 empty chunk (the remainder) */
    0                           /* number of value chunks */
  };
  CHECK(hd->write(hd, 0, (char*)(&header), sizeof(struct hd_header), hd->ioarg));
  CHECK(
    hd->write(
      hd,
      sizeof(struct hd_header),
      (char*)buckets,
      (5 * sizeof(unsigned)),
      hd->ioarg
    )
  );
  CHECK(
    hd->write(
      hd,
      sizeof(struct hd_header) + (5 * sizeof(unsigned)),
      (char*)(&emptychunk),
      sizeof(struct chunkhead),
      hd->ioarg
    )
  );
  return 0;
}

static
int hd_compare
  (hd_t* hd, hdt_t* key1, hdt_t* key2, void* arg)
{
  unsigned size = key1->size;
  int c, key1bigger = 0;
  if (key2->size < size) {
    key1bigger = 1;
    size = key2->size;
  }
  if ((c = memcmp(key1->data, key2->data, size))) {
    return c;
  } else if (key1bigger) {
    return 1;
  } else {
    return 0;
  }
}

static
unsigned hd_hash_default
  (hd_t* hd, hdt_t* key, void* arg)
{
  unsigned hash = 0;
  hd_hash(key->data, key->size, &hash);
  return hash;
}

/**
 * \ingroup hashtable_private
 *
 * Second stage initializer for a hd_t structure.
 *
 * \param hd Non-NULL pointer to a somewhat initialized hd_t structure.
 * \param flags Permanent flags for the hashtable.
 *
 * \returns Zero on success, or non-zero on error.
 */
int hd_init2
  (hd_t* hd, unsigned flags)
{
  struct hd_header header;

  CHECK(hd->read(hd, 0, (char*)(&header), sizeof(struct hd_header), hd->ioarg));
  if (memcmp(header.magic, "hdi", 4)) {
    if (flags & HDFLG_MUSTEXIST) {
      return HDERR_INIT;
    } else {
      CHECK(hd_init_create(hd, flags));
    }
  } else {
    hd->header = header;
  }
  hd->compare = hd_compare;
  hd->hash = hd_hash_default;
  return 0;
}

#ifdef __cplusplus
}
#endif
