//
//  wnet_lib.hpp
//
//
//  Created by Maurizio Giordano on 20/03/2014
//
//


#ifndef wnet_LIB_H
#define wnet_LIB_H

#include <stdlib.h>

extern "C"  //Tells the compile to use C-linkage for the next scope.
{
    
    typedef struct wentry wentry_t;
    typedef struct wkeymax wkeymax_t;
    typedef unsigned long int wkey_t;
    typedef double wvalue_t;

    struct wkeymax {
        wkey_t key;
        wvalue_t value;
    };
    
    struct wentry {
        wkey_t key;
        wvalue_t value;
        wentry_t *next;
        wentry_t *prev;
    };

    wentry_t *wram_create();
    wvalue_t wram_set(wentry_t *m, wkey_t key, wvalue_t value);
    wvalue_t wram_set_or_incr(wentry_t *m, wkey_t key, wvalue_t value, wvalue_t incr);
    wvalue_t wram_del(wentry_t *m, wkey_t key);
    wvalue_t wram_get(wentry_t *m, wkey_t key);
    wvalue_t wram_incr(wentry_t *m, wkey_t key);
    wvalue_t wram_incr_top(wentry_t *m, wkey_t key, wvalue_t top);
    wvalue_t wram_decr(wentry_t *m, wkey_t key);
    wvalue_t wram_decr_or_del(wentry_t *m,wkey_t key,wvalue_t decr);
    wvalue_t wram_decr_all_but_key_old(wentry_t *m, wkey_t key);
    wvalue_t wram_decr_all_but_key(wentry_t *m, wkey_t key,wvalue_t incr,wvalue_t decr);
    wvalue_t wram_decr_all_but_key_top(wentry_t *m, wkey_t key,wvalue_t incr,wvalue_t decr, wvalue_t top);
    unsigned long wram_len(wentry_t *m);
    void wram_print(wentry_t *m);
    void wram_free(wentry_t *m);
    void intuple_print(wkey_t *vector, long size);
    wentry_t *wram_copy(wentry_t *m);
    void wram_sink(wentry_t *m);
}
#endif