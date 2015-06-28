/*
** Copyright 2015 K.J. Hermans (kees@pink-frog.com)
** This code is part of simpledbm, an API to a dbm on a finite resource.
** License: BSD
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "hd_private.h"

/**
 * \ingroup hashtable
 * \param hd Non-NULL pointer to an initialized hd_t structure.
 * \returns Zero on success, or non-zero on error.
 */
int hd_read_keydata
  (hd_t* hd, unsigned keyoff, struct keyhead* keyhead, hdt_t* key)
{
  int reallocating = (
    key->data == 0 &&
    key->size == 0 &&
    (hd->header.flags & HDFLG_ALLOCHDT) &&
    hd->realloc != 0
  );
  unsigned keysize = keyhead->size - sizeof(struct keyhead);
  if (reallocating) {
    void* mem = hd->realloc(hd, key->data, keysize, hd->reallocarg);
    if (mem) {
      key->data = mem;
      key->size = keysize;
    } else {
      return HDERR_NOMEM;
    }
  } else {
    if (keysize > key->size) {
      keysize = key->size;
    } else {
      key->size = keysize;
    }
  }
  CHECK(hd_read(hd, keyoff + sizeof(struct keyhead), key->data, keysize));
  return 0;
}

#ifdef __cplusplus
}
#endif
