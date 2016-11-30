//
//  wnet_lib.cpp
//
//
//  Created by Maurizio Giordano on 20/03/2014
//
// a library for ram creation, initialization and print
// rams are circular lists of pairs key value
//

#include <stdio.h>
#include <string.h>
#include "wnet_lib.hpp"

unsigned long int wcounter;
unsigned long int mypowers[64] = {
    1UL, 2UL, 4UL, 8UL, 16UL, 32UL, 64UL, 128UL, 256UL, 512UL, 1024UL, 2048UL, 4096UL, 8192UL, 16384UL, 32768UL, 65536UL, 131072UL , 262144UL, 524288UL,
    1048576UL, 2097152UL, 4194304UL, 8388608UL, 16777216UL, 33554432UL, 67108864UL, 134217728UL, 268435456UL, 536870912UL, 1073741824UL, 2147483648UL,
    4294967296UL, 8589934592UL, 17179869184UL, 34359738368UL, 68719476736UL, 137438953472UL, 274877906944UL, 549755813888UL, 1099511627776UL, 2199023255552UL, 4398046511104UL, 8796093022208UL, 17592186044416UL, 35184372088832UL, 70368744177664UL, 140737488355328UL, 281474976710656UL, 562949953421312UL, 1125899906842624UL, 2251799813685248UL, 4503599627370496UL, 9007199254740992UL, 18014398509481984UL, 36028797018963968UL, 72057594037927936UL, 144115188075855872UL, 288230376151711744UL, 576460752303423488UL, 1152921504606846976UL, 2305843009213693952UL, 4611686018427387904UL, 9223372036854775808UL
};


extern "C"  //Tells the compile to use C-linkage for the next scope.
{
    
    // create (and init) a ram
    wentry_t *wram_create() {
        wentry_t *m;
        wcounter++;
        m = (wentry_t *)malloc(sizeof(wentry_t));
        m->key = -1;
        m->value = -1;
        m->next = m;
        m->prev = m;
        return m;
    }
    
    // set ram entry to "value"
    // if ram is empty, initalize it
    wvalue_t wram_set(wentry_t *m, wkey_t key,wvalue_t value) {
        wentry_t *p, *newp;
        
        // circulate in list for insertion
        p = m;
        for (;;) {
            p = p->next;
            if (p ==m || key <= p->key) break;
        }
        // the key already exists (update)
        if (key == p->key) {
            //printf("key matched\n");
            // delete item if value is null
            if (value==0) {
                (p->prev)->next = p->next;
                (p->next)->prev = p->prev;
                wcounter--;
                free(p);
            } else {
                p->value = value;
            }
            // the key does not exist (insertion)
        } else {
            wcounter++;
            //printf("new key\n");
            newp = (wentry_t *)malloc(sizeof(wentry_t));
            newp->key = key;
            newp->value = value;
            newp->next = p;
            newp->prev = p->prev;
            (p->prev)->next = newp;
            p->prev = newp;
        }
        return value;
    }
    
    // put ram entry to zero
    wvalue_t wram_del(wentry_t *m, wkey_t key) {
        wentry_t *p;
        
        // circulate in list for insertion
        p = m;
        for (;;) {
            p = p->next;
            if (key == p->key) {
                printf("delete key\n");
                (p->prev)->next = p->next;
                (p->next)->prev = p->prev;
                wcounter--;
                free(p);
                break;
            }
            if (p ==m) break;
        }
        return (wvalue_t)0;
    }
    
    // get ram entry
    wvalue_t wram_get(wentry_t *m, wkey_t key) {
        wentry_t *p;
        
        // circulate in list for insertion
        p = m;
        for (;;) {
            p = p->next;
            if (key == p->key) return p->value;
            if (p ==m) return (wvalue_t)0;
        }
    }
    
    // if ram entry exists increment its value by "incr"
    // otherwise ram entry is inserted and set to "value"
    wvalue_t wram_set_or_incr(wentry_t *m, wkey_t key, wvalue_t value, wvalue_t incr) {
        wentry_t *p, *newp;
        
        // circulate in list for insertion
        p = m;
        for (;;) {
            p = p->next;
            if (p ==m || key <= p->key) break;
            //if (key >= p->key) break;
        }
        // the key already exists (update)
        if (key == p->key) {
            //printf("key matched\n");
            p->value += incr;
            return p->value;
            // the key does not exist (insertion)
        } else {
            wcounter++;
            //printf("new key\n");
            newp = (wentry_t *)malloc(sizeof(wentry_t));
            newp->key = key;
            newp->value = value;
            newp->next = p;
            newp->prev = p->prev;
            (p->prev)->next = newp;
            p->prev = newp;
            return value;

        }
    }
    
    // if ram entry exists increment its value by "incr"
    // otherwise ram entry is inserted and set to "value"
    wvalue_t wram_incr(wentry_t *m, wkey_t key) {
        wentry_t *p, *newp;
        
        // circulate in list for insertion
        p = m;
        for (;;) {
            p = p->next;
            if (p ==m || key <= p->key) break;
            //if (key >= p->key) break;
        }
        // the key already exists (update)
        if (key == p->key) {
            //printf("key matched\n");
            p->value += (wvalue_t)1;
            return p->value;
            // the key does not exist (insertion)
        } else {
            wcounter++;
            //printf("new key\n");
            newp = (wentry_t *)malloc(sizeof(wentry_t));
            newp->key = key;
            newp->value = (wvalue_t)1;
            newp->next = p;
            newp->prev = p->prev;
            (p->prev)->next = newp;
            p->prev = newp;
            return (wvalue_t)1;
        }
    }

    wvalue_t wram_incr_top(wentry_t *m, wkey_t key, wvalue_t top) {
        wentry_t *p, *newp;
        
        // circulate in list for insertion
        p = m;
        for (;;) {
            p = p->next;
            if (p ==m || key <= p->key) break;
            //if (key >= p->key) break;
        }
        // the key already exists (update)
        if (key == p->key) {
            //printf("key matched\n");
            if ((p->value + (wvalue_t)1) < top) p->value += (wvalue_t)1;
            // the key does not exist (insertion)
            return p->value;
        } else {
            wcounter++;
            //printf("new key\n");
            newp = (wentry_t *)malloc(sizeof(wentry_t));
            newp->key = key;
            newp->value = (wvalue_t)1;
            newp->next = p;
            newp->prev = p->prev;
            (p->prev)->next = newp;
            p->prev = newp;
            return (wvalue_t)1;
        }
    }

    wvalue_t wram_decr(wentry_t *m,wkey_t key) {
        wentry_t *p;
        
        // circulate in list for insertion
        p = m;
        for (;;) {
            p = p->next;
            if (p ==m) break;
            if (key == p->key) {
                if ((p->value - (wvalue_t)1) <= 0) {
                    // remove entry
                    //printf("delete key\n");
                    (p->prev)->next = p->next;
                    (p->next)->prev = p->prev;
                    wcounter--;
                    free(p);
                } else {
                    // decrease the entry
                    //printf("decr 1 key\n");
                    p->value -= (wvalue_t)1;
                }
                return p->value;
            }
        }
        return (wvalue_t)0;
    }
    
    wvalue_t wram_decr_all_but_key_old(wentry_t *m,wkey_t key) {
        wentry_t *p, *newp;
        wvalue_t retval;
        bool notfound = true;
        // circulate in list for insertion
        p = m;
        for (;;) {
            p = p->next;
            if (p ==m) break;
            if (key == p->key) {
                notfound = false;
                //printf("key matched\n");
                p->value += (wvalue_t)1;
                retval = p->value;
                // the key does not exist (insertion)
            } else {
                if ((p->value - (wvalue_t)1) <= 0) {
                    // remove entry
                    //printf("delete key\n");
                    (p->prev)->next = p->next;
                    (p->next)->prev = p->prev;
                    wcounter--;
                    free(p);
                } else {
                    // decrease the entry
                    //printf("decr 1 key\n");
                    p->value -= (wvalue_t)1;
                }
            }
        }
        if (notfound) {
            wcounter++;
            //printf("new key\n");
            newp = (wentry_t *)malloc(sizeof(wentry_t));
            newp->key = key;
            newp->value = (wvalue_t)1;
            newp->next = m;
            newp->prev = m->prev;
            (m->prev)->next = newp;
            m->prev = newp;
            retval = (wvalue_t)1;
        }
        return retval;
    }
    
    wvalue_t wram_decr_all_but_key(wentry_t *m,wkey_t key, wvalue_t incr, wvalue_t decr) {
        wentry_t *p, *rp, *newp;
        bool notfound = true;
        // circulate in list for decrement
        p = m;
        for (;;) {
            p = p->next;
            if (p ==m) break;
            if (key == p->key) {  // the key exists (do nothing!)
                ;
            } else {              // the key does not match (decrease it!)
                if ((p->value - decr) < 1) {
                    // remove entry
                    rp = p;
                    (p->prev)->next = p->next;
                    (p->next)->prev = p->prev;
                    p = p->prev;
                    free(rp);
                    wcounter--;
                } else {
                    // decrease the entry
                    p->value -= decr;
                }
            }
        }
        // circulate in list for insertion
        p = m;
        for (;;) {
            p = p->next;
            if (p ==m || key <= p->key) break;
        }
        // the key already exists (update)
        if (key == p->key) {
            p->value += incr;
            // the key does not exist (insertion)
            return p->value;
        } else {
            wcounter++;
            newp = (wentry_t *)malloc(sizeof(wentry_t));
            newp->key = key;
            newp->value = incr;
            newp->next = p;
            newp->prev = p->prev;
            (p->prev)->next = newp;
            p->prev = newp;
            return incr;
        }
    }

    wvalue_t wram_decr_all_but_key_top(wentry_t *m,wkey_t key, wvalue_t incr, wvalue_t decr, wvalue_t top) {
        wentry_t *p, *rp, *newp;
        bool notfound = true;
        wvalue_t retval=0;
        // circulate in list for decrement
        p = m;
        for (;;) {
            p = p->next;
            if (p ==m) break;
            if (key == p->key) {  // the key exists (do nothing!)
                ;
            } else {              // the key does not match (decrease it!)
                if ((p->value - decr) < 1) {
                    // remove entry
                    rp = p;
                    (p->prev)->next = p->next;
                    (p->next)->prev = p->prev;
                    p = p->prev;
                    free(rp);
                    wcounter--;
                } else {
                    // decrease the entry
                    p->value -= decr;
                }
            }
        }
        // circulate in list for insertion
        p = m;
        for (;;) {
            p = p->next;
            if (p ==m || key <= p->key) break;
        }
        // the key already exists (update)
        if (key == p->key) {
            p->value += incr;
            // the key does not exist (insertion)
            if (p->value > top) p->value = top; // saturation
            return p->value;
        } else {
            wcounter++;
            newp = (wentry_t *)malloc(sizeof(wentry_t));
            newp->key = key;
            newp->value = incr;
            newp->next = p;
            newp->prev = p->prev;
            (p->prev)->next = newp;
            p->prev = newp;
            if (newp->value > top) newp->value = top; // saturation
            return newp->value;
        }
    }

    wvalue_t wram_decr_or_del(wentry_t *m,wkey_t key,wvalue_t decr) {
        wentry_t *p;
        
        // circulate in list for insertion
        p = m;
        for (;;) {
            p = p->next;
            if (p ==m) break;
            if (key == p->key) {
                if ((p->value - decr) <= 0) {
                    // remove entry
                    //printf("delete key\n");
                    (p->prev)->next = p->next;
                    (p->next)->prev = p->prev;
                    wcounter--;
                    free(p);
                    return (wvalue_t)0;
                } else {
                    // decrease the entry
                    //printf("decr 1 key\n");
                    p->value -= decr;
                    return p->value;
                }
            }
        }
        return (wvalue_t)0;
    }
    
    // count non-zero ram entries
    unsigned long wram_len(wentry_t *m) {
        wentry_t *p;
        int cnt=0;
        
        // circulate in list for insertion
        p = m;
        for (;;) {
            p = p->next;
            if (p ==m) break;
            else cnt++;
        }
        return cnt;
    }
    
    wentry_t *wram_copy(wentry_t *m) {
        wentry_t *p, *cp;
        
        cp = wram_create();
        for(p=m;;p=p->next) {
            if (p->next==m) break;
            wram_set(cp, p->next->key, p->next->value);
        }
        return cp;
    }
    
    // decrease all ram entries at once
    void wram_sink(wentry_t *m) {
        wentry_t *p;
        
        p = m;
        for (;;) {
            if (p->next==m) break;
            p = p->next;
            if (p->value > 1) p->value -= 1;
            else {
                (p->prev)->next = p->next;
                (p->next)->prev = p->prev;
                wcounter--;
                free(p);
            }
        }
    }
    
    // discriminator print utility functions
    void wram_print(wentry_t *m) {
        wentry_t *p;
        
        fprintf(stdout,"{");
        for(p=m;;p=p->next) {
            if (p->next==m) break;
            if (p->next->next==m) fprintf(stdout,"%lu:%f", p->next->key, p->next->value);
            else fprintf(stdout,"%lu:%f,", p->next->key, p->next->value);
        }
        fprintf(stdout,"}\n");
    }
    
    // free ram
    void wram_free(wentry_t *m) {
        wentry_t *p;
        
        // circulate in list for insertion
        p = m;
        for (;;) {
            p = p->next;
            (p->prev)->next = p->next;
            (p->next)->prev = p->prev;
            wcounter--;
            free(p);
            if (p ==m) break;
        };
    }
    
    void intuple_print(wkey_t *vector, long size) {
        long i;
        printf("[");
        for (i = 0; i < size; i++) {
            if (vector[i] > 0) printf("%lu", vector[i]);
            else printf(" ");
            if (i != size -1) printf(".");
        }
        printf("]\n");
    }

}
