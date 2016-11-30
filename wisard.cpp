//
//  wisard.cpp
//
//
//  Created by Maurizio Giordano on 20/03/2014
//
// the WISARD C++ implementation for background extraction
//

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "wnet_lib.hpp"
#include "wisard.hpp"
#include <iostream>
#include <stdexcept>

extern unsigned long int wcounter;
extern unsigned long int mypowers[];
using namespace std;

extern "C"  //Tells the compile to use C-linkage for the next scope.
{
    
    // exchange sort
    void simplesort(double *ar, int *idx, int n) {
        int i, j;
        double tmp;
        int tmp2;
        
        for(i=0;i<n;i++) {
            for(j=0;j<n-i;j++) {
                if(ar[j]<ar[j+1]) {
                    tmp=ar[j+1];
                    ar[j+1]=ar[j];
                    ar[j]=tmp;
                    tmp2 = idx[j+1];
                    idx[j+1] = idx[j];
                    idx[j] = tmp2;
                }
            }
        }
    }
    
    int **mapping(int size, char *mode) {
        register int i;
        int **maps;
        srand (time(NULL));
        maps = (int **)malloc(2 * sizeof(int *));
        int *list = (int *)malloc(size * sizeof(int));
        // revers mapping (not necessary yet!)
        int *rlist = (int *)malloc(size * sizeof(int));
        if (strcmp("linear", mode)==0) {
            for (i = 0; i < size; i++) {
                list[i] = i;
                rlist[i] = i;
            }
            maps[0] = (int *)list;
            maps[1] = (int *)rlist;
            return maps;
        } else if (strcmp("empty", mode)==0) {
            for (i = 0; i < size; i++) {
                list[i] = 0;
                rlist[i] = 0;
            }
            maps[0] = (int *)list;
            maps[1] = (int *)rlist;
            return maps;
        } else if (strcmp("random", mode)==0) {
            for (i = 0; i < size; i++) {
                list[i] = i;
                rlist[i] = i;
            }
            int *vektor = (int *)malloc(size * sizeof(int));
            for (i = 0; i < size; i++) {
                int j = i + rand() % (size - i);
                int temp = list[i];
                list[i] = list[j];
                rlist[list[j]] = i;
                list[j] = temp;
                vektor[i] = list[i];
            }
            maps[0] = (int *)vektor;
            maps[1] = (int *)rlist;
            return maps;
        } else {
            throw std::invalid_argument( "received wrong mapping mode" );
        }
    }
    
    discr_t *makeDiscr(int n_bit, int size, char *name, char *mode) {
        int i;
        discr_t *p = (discr_t *)malloc(sizeof(discr_t));
        int **maps;
        wkeymax_t *newpk;
        p->n_bit = n_bit;
        p->n_loc = (wkey_t)pow(2,n_bit) - (wkey_t)1;
        //p->n_loc = mypowers[n_bit] -1;
        p->size = size;
        p->tcounter = 0;
        if (name != NULL) {
            p->name = (char *)malloc(strlen(name) * sizeof(char));
            strcpy(p->name, name);
        } else p->name = NULL;
        p->mi = (wvalue_t *)malloc(size * sizeof(wvalue_t));
        for (i=0; i<size;i++) { p->mi[i] = (wvalue_t) 0; }
        if (size % n_bit == 0)
        p->n_ram = (int)(size / n_bit);
        else
        p->n_ram = (int)(size / n_bit) + 1;
        maps = (int **)mapping(size, mode);
        p->map = maps[0];
        p->rmap = maps[1];
        p->rams = (wentry_t **)malloc(p->n_ram * sizeof(wentry_t *));
        p->maxkeys = (wkeymax_t *)malloc(p->n_ram * sizeof(wkeymax_t));
        for (i=0;i<p->n_ram;i++) {
            p->rams[i] = (wentry_t *)wram_create();
            p->maxkeys[i].key = (wkey_t)-1;
            p->maxkeys[i].value = (wvalue_t)-1;
        }
        return p;
    }
    
    // works on grey level images
    discr_t *makeTrainImgDiscr(int n_bit, int size, char *name, char *mode, const void * imgv, int cols, int bx, int by, int width, int tics) {
        discr_t *p = (discr_t *)malloc(sizeof(discr_t));
        int **maps, i, neuron;
        wkeymax_t *newpk;
        int index;
        unsigned char delta = (unsigned char)(256 / tics);
        int X, x, k, I, J;
        int val;
        wkey_t address;
        wvalue_t retval;
        wkey_t key;
        const unsigned char * img = (unsigned char *) imgv;
        
        p->n_bit = n_bit;
        p->n_loc = (wkey_t)mypowers[n_bit] - (wkey_t)1;
        p->size = size;
        p->tcounter = 1;
        if (name != NULL) {
            p->name = (char *)malloc(strlen(name) * sizeof(char));
            strcpy(p->name, name);
        } else p->name = NULL;
        p->mi = (wvalue_t *)malloc(size * sizeof(wvalue_t));
        for (i=0; i<size;i++) { p->mi[i] = (wvalue_t) 0; }
        if (size % n_bit == 0) p->n_ram = (int)(size / n_bit);
        else p->n_ram = (int)(size / n_bit) + 1;
        maps = (int **)mapping(size, mode);
        p->map = maps[0];
        p->rmap = maps[1];
        p->rams = (wentry_t **)malloc(p->n_ram * sizeof(wentry_t *));
        p->maxkeys = (wkeymax_t *)malloc(p->n_ram * sizeof(wkeymax_t));
        for (neuron=0;neuron<p->n_ram;neuron++) {
            p->rams[neuron] = (wentry_t *)wram_create();
            address=(wkey_t)0;
            // make ram key to be fired
            for (i=0; i < n_bit; ++i) {
                index = neuron * n_bit + i;
                X = p->map[index % size];
                x = X / tics;
                k = X % tics;
                J = x % width + bx;
                I = x / width + by;
                val = (int)(img[I * cols + J] / delta);
                if (k < val)
                address += (wkey_t)mypowers[n_bit - i - 1];
            }
            retval = wram_incr(p->rams[neuron],address);
            // update max key for ram
            p->maxkeys[neuron].key = address;
            p->maxkeys[neuron].value = retval;
        }
        return p;
    }

    // works on binay images
    discr_t *makeTrainImgBinDiscr(int n_bit, int size, char *name, char *mode, const void * imgv, int cols) {
        discr_t *p = (discr_t *)malloc(sizeof(discr_t));
        int **maps, i, neuron;
        wkeymax_t *newpk;
        int index;
        int x, I, J;
        wkey_t address;
        wvalue_t retval;
        wkey_t key;
        const unsigned char * img = (unsigned char *) imgv;
        
        p->n_bit = n_bit;
        p->n_loc = (wkey_t)mypowers[n_bit] - (wkey_t)1;
        p->size = size;
        p->tcounter = 1;
        if (name != NULL) {
            p->name = (char *)malloc(strlen(name) * sizeof(char));
            strcpy(p->name, name);
        } else p->name = NULL;
        p->mi = (wvalue_t *)malloc(size * sizeof(wvalue_t));
        for (i=0; i<size;i++) { p->mi[i] = (wvalue_t) 0; }
        if (size % n_bit == 0) p->n_ram = (int)(size / n_bit);
        else p->n_ram = (int)(size / n_bit) + 1;
        maps = (int **)mapping(size, mode);
        p->map = maps[0];
        p->rmap = maps[1];
        p->rams = (wentry_t **)malloc(p->n_ram * sizeof(wentry_t *));
        p->maxkeys = (wkeymax_t *)malloc(p->n_ram * sizeof(wkeymax_t));
        for (neuron=0;neuron<p->n_ram;neuron++) {
            p->rams[neuron] = (wentry_t *)wram_create();
            address=(wkey_t)0;
            // make ram key to be fired
            for (i=0; i < n_bit; ++i) {
                index = neuron * n_bit + i;
                x = p->map[index % size];
                J = x % cols;
                I = x / cols;
                if (img[I * cols + J] == 0)
                    address += (wkey_t)mypowers[n_bit - i - 1];
            }
            retval = wram_incr(p->rams[neuron],address);
            // update max key for ram
            p->maxkeys[neuron].key = address;
            p->maxkeys[neuron].value = retval;
        }
        return p;
    }

    discrarray_t *makeDiscrArray(int n_bit, int npixels, char *name, char *mode, int tics) {
        discrarray_t *p = (discrarray_t *)malloc(sizeof(discrarray_t));
        int **maps, i, neuron, n;
        wkeymax_t *newpk;
        int index;
        wkey_t key;
        wentry_t **net;
        
        assert(n_bit <= tics);
        p->n_bit = n_bit;
        p->n_loc = (wkey_t)mypowers[n_bit] - (wkey_t)1;
        p->size = npixels;
        p->tcounter = 1;
        if (name != NULL) {
            p->name = (char *)malloc(strlen(name) * sizeof(char));
            strcpy(p->name, name);
        } else p->name = NULL;
        p->rams = (wentry_t ***)malloc(npixels * sizeof(wentry_t **));
        p->maxkeys = (wkeymax_t **)malloc(npixels * sizeof(wkeymax_t *));
        if (tics % n_bit == 0) p->n_ram = (int)(tics / n_bit);
        else p->n_ram = (int)(tics / n_bit) + 1;
        maps = (int **)mapping(tics, mode);
        p->map = maps[0];
        p->rmap = maps[1];
        for (n=0; n<npixels; n++) {
            p->rams[n] = (wentry_t **)malloc(p->n_ram * sizeof(wentry_t *));
            p->maxkeys[n] = (wkeymax_t *)malloc(p->n_ram * sizeof(wkeymax_t));
            for (neuron=0;neuron<p->n_ram;neuron++) {
                p->rams[n][neuron] = (wentry_t *)wram_create();
                p->maxkeys[n][neuron].key = (wkey_t)0;
                p->maxkeys[n][neuron].value = (wvalue_t)-1;
            }
        }
        return p;
    }

    discrarray_t *makeTrainImgDiscrArray(int n_bit, int npixels, char *name, char *mode, const void * imgv, int cols, int bx, int by, int width, int tics) {
        discrarray_t *p = (discrarray_t *)malloc(sizeof(discrarray_t));
        int **maps, i, neuron,n;
        wkeymax_t *newpk;
        int index;
        unsigned char delta = (unsigned char)(256 / tics);
        int X, I, J;
        int val;
        wkey_t address;
        wvalue_t retval;
        wkey_t key;
        const unsigned char * img = (unsigned char *) imgv;
        
        assert(n_bit <= tics);
        p->n_bit = n_bit;
        p->n_loc = (wkey_t)mypowers[n_bit] - (wkey_t)1;
        p->size = npixels;
        p->tcounter = 1;
        if (name != NULL) {
            p->name = (char *)malloc(strlen(name) * sizeof(char));
            strcpy(p->name, name);
        } else p->name = NULL;
        p->rams = (wentry_t ***)malloc(npixels * sizeof(wentry_t **));
        p->maxkeys = (wkeymax_t **)malloc(npixels * sizeof(wkeymax_t *));
        if (tics % n_bit == 0) p->n_ram = (int)(tics / n_bit);
        else p->n_ram = (int)(tics / n_bit) + 1;
        maps = (int **)mapping(tics, mode);
        p->map = maps[0];
        p->rmap = maps[1];
        for (n=0; n<npixels; n++) {
            p->rams[n] = (wentry_t **)malloc(p->n_ram * sizeof(wentry_t *));
            p->maxkeys[n] = (wkeymax_t *)malloc(p->n_ram * sizeof(wkeymax_t));
            J = n % width + bx;
            I = n / width + by;
            val = (int)(img[I * cols + J] / delta);
            for (neuron=0;neuron<p->n_ram;neuron++) {
                p->rams[n][neuron] = (wentry_t *)wram_create();
                address=(wkey_t)0;
                // make ram key to be fired
                for (i=0; i < n_bit; ++i) {
                    index = neuron * n_bit + i;
                    X = p->map[index % tics];
                    if (X < val)
                        address += (wkey_t)mypowers[n_bit - i - 1];
                }
                retval = wram_incr(p->rams[n][neuron],address);
                // update max key for ram
                p->maxkeys[n][neuron].key = address;
                p->maxkeys[n][neuron].value = retval;
            }
        }
        return p;
    }

    discr_t *copyDiscr(discr_t *discr) {
        int neuron, i;
        wkeymax_t *newpk;
        discr_t *copydiscr = (discr_t *)malloc(sizeof(discr_t));
        copydiscr->n_bit = discr->n_bit;
        copydiscr->size = discr->size;
        copydiscr->n_ram = discr->n_ram;
        copydiscr->n_loc = discr->n_loc;
        copydiscr->tcounter = discr->tcounter;
        copydiscr->name = (char *)malloc(strlen(discr->name) * sizeof(char));
        strcpy(copydiscr->name, discr->name);
        copydiscr->map = (int *)malloc(discr->size * sizeof(int));
        copydiscr->rmap = (int *)malloc(discr->size * sizeof(int));
        copydiscr->mi = (wvalue_t *)malloc(discr->size * sizeof(wvalue_t));
        for (i=0; i < discr->size; i++) {
            copydiscr->rmap[i] = discr->rmap[i];
            copydiscr->map[i] = discr->map[i];
            copydiscr->mi[i] = discr->mi[i];
        }
        copydiscr->rams = (wentry_t **)malloc(copydiscr->n_ram * sizeof(wentry_t *));
        copydiscr->maxkeys = (wkeymax_t *)malloc(copydiscr->n_ram * sizeof(wkeymax_t));
        for (neuron=0; neuron < copydiscr->n_ram; neuron++) {
            copydiscr->rams[neuron] = (wentry_t *)wram_copy(discr->rams[neuron]);
            copydiscr->maxkeys[neuron].key = discr->maxkeys[neuron].key;
            copydiscr->maxkeys[neuron].value = discr->maxkeys[neuron].value;
        }
        copydiscr->maxmi = discr->maxmi;
        return copydiscr;
    }
    
    void trainDiscr(discr_t *discr, wkey_t *in_tuples) {
        int neuron;
        wvalue_t retval;
        discr->tcounter++;
        for (neuron=0;neuron<discr->n_ram;neuron++) {
            retval = wram_incr(discr->rams[neuron],in_tuples[neuron]);
            // update max key for ram
            if (retval > discr->maxkeys[neuron].value) {
                discr->maxkeys[neuron].key = in_tuples[neuron];
                discr->maxkeys[neuron].value = retval;
            }
        }
    }
    
    void trainSvmHistoDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr) {
        int neuron;
        wvalue_t retval;
        discr->tcounter++;
        wkey_t address;
        int x, i, index, npixels=nt * nattr, n_bit = discr->n_bit, value;
        for (neuron=0;neuron<discr->n_ram;neuron++) {
            // compute neuron simulus
            address=(wkey_t)0;
            // decompose record data values into wisard input
            for (i=0;i<n_bit;i++) {
                x = discr->map[((neuron * n_bit) + i) % npixels];
                index = x/nt;
                value = (int) ((data[index] - off[index]) * nt / den[index]);
                if ((x % nt) < value) {
                    address |= mypowers[n_bit -1 - i];
                }
            }
            
            retval = wram_incr(discr->rams[neuron],address);
            // update max key for neuron
            if (retval > discr->maxkeys[neuron].value) {
                discr->maxkeys[neuron].key = address;
                discr->maxkeys[neuron].value = retval;
            }
        }
    }
    
    void trainSvmPointDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr) {
        int neuron;
        wvalue_t retval;
        discr->tcounter++;
        wkey_t address;
        int x, i, index, npixels=nt * nattr, n_bit = discr->n_bit, value;
        for (neuron=0;neuron<discr->n_ram;neuron++) {
            // compute neuron simulus
            address=(wkey_t)0;
            // decompose record data values into wisard input
            for (i=0;i<n_bit;i++) {
                x = discr->map[((neuron * n_bit) + i) % npixels];
                index = x/nt;
                value = (int) ((data[index] - off[index]) * nt / den[index]);
                if ((x % nt) == value -1) {
                    address |= mypowers[n_bit -1 - i];
                }
            }
            
            retval = wram_incr(discr->rams[neuron],address);
            // update max key for neuron
            if (retval > discr->maxkeys[neuron].value) {
                discr->maxkeys[neuron].key = address;
                discr->maxkeys[neuron].value = retval;
            }
        }
    }

    void trainSvmCursorDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr) {
        int neuron;
        wvalue_t retval;
        discr->tcounter++;
        wkey_t address;
        int x, i, index, npixels=nt * nattr, n_bit = discr->n_bit, value;
        int c1, c2, cursor, xr;
        for (neuron=0;neuron<discr->n_ram;neuron++) {
            // compute neuron simulus
            address=(wkey_t)0;
            // decompose record data values into wisard input
            for (i=0;i<n_bit;i++) {
                x = discr->map[((neuron * n_bit) + i) % npixels];
                index = x/nt;
                value = (int) (data[index] - off[index]);
                if (den[index] == value) {
                    c1 = c2 = nt - 1;
                } else {
                    cursor = (int)((data[index] - off[index]) * (2 * nt - 1) / den[index]);
                    c1 = (int)(cursor/2.0);
                    if ((cursor % 2) > 0) c2 = (int)(cursor/2.0 + 1);
                    else c2 = (int)(cursor/2.0);
                }
                xr = x % nt;
                if (xr == c1 or xr == c2) {
                    address |= mypowers[n_bit -1 - i];
                }

            }
            
            retval = wram_incr(discr->rams[neuron],address);
            // update max key for neuron
            if (retval > discr->maxkeys[neuron].value) {
                discr->maxkeys[neuron].key = address;
                discr->maxkeys[neuron].value = retval;
            }
        }
    }

    void trainSvmBinDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr) {
        int neuron;
        wvalue_t retval;
        discr->tcounter++;
        wkey_t address;
        int x, i, index, npixels=nt * nattr, n_bit = discr->n_bit, value;
        int c1, c2, cursor, xr;
        for (neuron=0;neuron<discr->n_ram;neuron++) {
            // compute neuron simulus
            address=(wkey_t)0;
            // decompose record data values into wisard input
            for (i=0;i<n_bit;i++) {
                x = discr->map[((neuron * n_bit) + i) % npixels];
                index = x/nt;
                value = (int)data[x/nt];
                if (((value >> (x % nt)) & 1) > 0) {
                    address |= mypowers[n_bit -1 - i];
                }
            }
            
            retval = wram_incr(discr->rams[neuron],address);
            // update max key for neuron
            if (retval > discr->maxkeys[neuron].value) {
                discr->maxkeys[neuron].key = address;
                discr->maxkeys[neuron].value = retval;
            }
        }
    }
    
    void trainImgDiscr(discr_t *discr, const void * imgv, int cols, int bx, int by, int width, int tics) {
        const unsigned char * img = (unsigned char *) imgv;
        int neuron;
        int n_bit = discr->n_bit;
        int n_ram = discr->n_ram;
        int index;
        int npixels = discr->size;
        unsigned char delta = (unsigned char)(256 / tics);
        int X, x, k, I, J;
        int val;
        wkey_t address;
        wvalue_t retval;
        wkey_t key;
        discr->tcounter++;
        for (neuron=0;neuron<n_ram;neuron++) {
            address=(wkey_t)0;
            // make ram key to be fired
            for (int i=0; i < n_bit; ++i) {
                index = neuron * n_bit + i;
                X = discr->map[index % npixels];
                x = X / tics;
                k = X % tics;
                J = x % width + bx;
                I = x / width + by;
                val = (int)(img[I * cols + J] / delta);
                if (k < val)
                    address += (wkey_t)mypowers[n_bit - i - 1];
            }
            retval = wram_incr(discr->rams[neuron],address);
            // update max key for ram
            if (retval > discr->maxkeys[neuron].value) {
                discr->maxkeys[neuron].key = address;
                discr->maxkeys[neuron].value = retval;
            }
        }
    }

    void trainImgBinDiscr(discr_t *discr, const void * imgv, int cols) {
        const unsigned char * img = (unsigned char *) imgv;
        int neuron;
        int n_bit = discr->n_bit;
        int n_ram = discr->n_ram;
        int index;
        int npixels = discr->size;
        int x, I, J;
        wkey_t address;
        wvalue_t retval;
        wkey_t key;
        discr->tcounter++;
        for (neuron=0;neuron<n_ram;neuron++) {
            address=(wkey_t)0;
            // make ram key to be fired
            for (int i=0; i < n_bit; ++i) {
                index = neuron * n_bit + i;
                x = discr->map[index % npixels];
                J = x % cols;
                I = x / cols;
                if (img[I * cols + J] == 0)
                    address += (wkey_t)mypowers[n_bit - i - 1];
            }
            retval = wram_incr(discr->rams[neuron],address);
            // update max key for ram
            if (retval > discr->maxkeys[neuron].value) {
                discr->maxkeys[neuron].key = address;
                discr->maxkeys[neuron].value = retval;
            }
        }
    }
    
    void trainImgDiscrArray(discrarray_t *discr, const void * imgv, int cols, int bx, int by, int width, int tics) {
        const unsigned char * img = (unsigned char *) imgv;
        int neuron, n;
        int n_bit = discr->n_bit;
        int n_ram = discr->n_ram;
        int index;
        int npixels = discr->size;
        unsigned char delta = (unsigned char)(256 / tics);
        int X, I, J;
        int val;
        wkey_t address;
        wvalue_t retval;
        wkey_t key;
        discr->tcounter++;
        for (n=0;n<npixels;n++) {
            J = n % width + bx;
            I = n / width + by;
            val = (int)(img[I * cols + J] / delta);
            for (neuron=0;neuron<n_ram;neuron++) {
                address=(wkey_t)0;
                // make ram key to be fired
                for (int i=0; i < n_bit; ++i) {
                    index = neuron * n_bit + i;
                    X = discr->map[index % tics];
                    if (X < val)
                    address += (wkey_t)mypowers[n_bit - i - 1];
                }
                retval = wram_incr(discr->rams[n][neuron],address);
                // update max key for ram
                if (retval > discr->maxkeys[n][neuron].value) {
                    discr->maxkeys[n][neuron].key = address;
                    discr->maxkeys[n][neuron].value = retval;
                }
            }
        }
    }

    // init discriminator
    void trainresetImgDiscr(discr_t *discr, const void * imgv, int cols, int bx, int by, int width, int tics) {
        const unsigned char * img = (unsigned char *) imgv;
        int neuron;
        int n_bit = discr->n_bit;
        int n_ram = discr->n_ram;
        int index;
        int npixels = discr->size;
        unsigned char delta = (unsigned char)(256 / tics);
        int X, x, k, I, J;
        int val;
        wkey_t address;
        wvalue_t retval;
        wkey_t key;
        discr->tcounter++;
        wentry_t *p;
        
        for (neuron=0; neuron < discr->n_ram; neuron++) {
            // init
            p = discr->rams[neuron];
            discr->rams[neuron] = (wentry_t *) wram_create();
            discr->maxkeys[neuron].key = (wkey_t)-1;
            discr->maxkeys[neuron].value = (wvalue_t)-1;
            free(p);
            // then train
            address=(wkey_t)0;
            // make ram key to be fired
            for (int i=0; i < n_bit; ++i) {
                index = neuron * n_bit + i;
                X = discr->map[index % npixels];
                x = X / tics;
                k = X % tics;
                J = x % width + bx;
                I = x / width + by;
                val = (int)(img[I * cols + J] / delta);
                if (k < val)
                    address += (wkey_t)mypowers[n_bit - i - 1];
            }
            retval = wram_incr(discr->rams[neuron],address);
            // update max key for ram
            if (retval > discr->maxkeys[neuron].value) {
                discr->maxkeys[neuron].key = address;
                discr->maxkeys[neuron].value = retval;
            }

        }
    }

    void trainforgetDiscrOld(discr_t *discr, wkey_t *in_tuples) {
        int neuron;
        wvalue_t retval;
        discr->tcounter++;
        for (neuron=0;neuron<discr->n_ram;neuron++) {
            //wram_decr_all_but_key(discr->rams[neuron],in_tuples[neuron] % discr->n_loc);
            retval = wram_decr_all_but_key(discr->rams[neuron],in_tuples[neuron],1,1);
            // update max key for ram
            discr->maxkeys[neuron].value--;
            if (retval > discr->maxkeys[neuron].value) {
                discr->maxkeys[neuron].key = in_tuples[neuron];
                discr->maxkeys[neuron].value = retval;
            }
        }
    }
    
    void trainforgetDiscr(discr_t *discr, wkey_t *in_tuples, wvalue_t incr, wvalue_t decr) {
        int neuron;
        wvalue_t retval;
        discr->tcounter++;
        for (neuron=0;neuron<discr->n_ram;neuron++) {
            //wram_decr_all_but_key(discr->rams[neuron],in_tuples[neuron] % discr->n_loc);
            retval = wram_decr_all_but_key(discr->rams[neuron],in_tuples[neuron],incr,decr);
            // update max key for ram
            discr->maxkeys[neuron].value--;
            if (retval > discr->maxkeys[neuron].value) {
                discr->maxkeys[neuron].key = in_tuples[neuron];
                discr->maxkeys[neuron].value = retval;
            }
        }
    }

    void trainforgettopDiscr(discr_t *discr, wkey_t *in_tuples, wvalue_t incr, wvalue_t decr, wvalue_t top) {
        int neuron;
        wvalue_t retval;
        discr->tcounter++;
        for (neuron=0;neuron<discr->n_ram;neuron++) {
            retval = wram_decr_all_but_key_top(discr->rams[neuron],in_tuples[neuron],incr,decr,top);
            // update max key for ram
            discr->maxkeys[neuron].value--;
            if (retval > discr->maxkeys[neuron].value) {
                discr->maxkeys[neuron].key = in_tuples[neuron];
                discr->maxkeys[neuron].value = retval;
            }
        }
    }
    
    void trainforgetImgDiscrArray(discrarray_t *discr, const void * imgv, int cols, int bx, int by, int width, int tics) {
        const unsigned char * img = (unsigned char *) imgv;
        int neuron, n;
        int n_bit = discr->n_bit;
        int n_ram = discr->n_ram;
        int npixels = discr->size;
        unsigned char delta = (unsigned char)(256 / tics);
        int I, J, val;
        wkey_t address;
        wvalue_t retval;
        discr->tcounter++;
        for (n=0;n<npixels;n++) {
            J = n % width + bx;
            I = n / width + by;
            val = (int)(img[I * cols + J] / delta);
            for (neuron=0;neuron<n_ram;neuron++) {
                address=(wkey_t)0;
                // make ram key to be fired
                for (int i=0; i < n_bit; ++i) {
                    if (discr->map[(neuron * n_bit + i) % tics] < val)
                        address += (wkey_t)mypowers[n_bit - i - 1];
                }
                retval = wram_decr_all_but_key(discr->rams[n][neuron],address,1,1);
                // update max key for ram
                if (retval > discr->maxkeys[n][neuron].value) {
                    discr->maxkeys[n][neuron].key = address;
                    discr->maxkeys[n][neuron].value = retval;
                }
            }
        }
    }
    
    void trainforgetImgDiscr(discr_t *discr, const void * imgv, int cols, int bx, int by, int width, int tics) {
        const unsigned char * img = (unsigned char *) imgv;
        int neuron;
        int n_bit = discr->n_bit;
        int n_ram = discr->n_ram;
        int index;
        int npixels = discr->size;
        unsigned char delta = (unsigned char)(256 / tics);
        int X, x, k, I, J;
        int val;
        wkey_t address;
        wvalue_t retval;
        wkey_t key;
        discr->tcounter++;
        for (neuron=0;neuron<n_ram;neuron++) {
            address=(wkey_t)0;
            // make ram key to be fired
            for (int i=0; i < n_bit; ++i) {
                index = neuron * n_bit + i;
                X = discr->map[index % npixels];
                x = X / tics;
                k = X % tics;
                J = x % width + bx;
                I = x / width + by;
                val = (int)(img[I * cols + J] / delta);
                if (k < val)
                    address += (wkey_t)mypowers[n_bit - i - 1];
            }
            retval = wram_decr_all_but_key(discr->rams[neuron],address,1,1);
            // update max key for ram
            if (retval > discr->maxkeys[neuron].value) {
                discr->maxkeys[neuron].key = address;
                discr->maxkeys[neuron].value = retval;
            }
        }
    }

    void trainforgetImgBinDiscr(discr_t *discr, const void * imgv, int cols) {
        const unsigned char * img = (unsigned char *) imgv;
        int neuron;
        int n_bit = discr->n_bit;
        int n_ram = discr->n_ram;
        int index;
        int npixels = discr->size;
        int x, I, J;
        wkey_t address;
        wvalue_t retval;
        wkey_t key;
        discr->tcounter++;
        for (neuron=0;neuron<n_ram;neuron++) {
            address=(wkey_t)0;
            // make ram key to be fired
            for (int i=0; i < n_bit; ++i) {
                index = neuron * n_bit + i;
                x = discr->map[index % npixels];
                J = x % cols;
                I = x / cols;
                if (img[I * cols + J] == 0)
                    address += (wkey_t)mypowers[n_bit - i - 1];
            }
            retval = wram_decr_all_but_key(discr->rams[neuron],address,1,1);
            // update max key for ram
            if (retval > discr->maxkeys[neuron].value) {
                discr->maxkeys[neuron].key = address;
                discr->maxkeys[neuron].value = retval;
            }
        }
    }
    
    double classifyDiscr(discr_t *discr, wkey_t *in_tuples) {
        int neuron, sum;
        
        for (neuron=0, sum=0;neuron<discr->n_ram;neuron++) {
            //if (wram_get(discr->rams[neuron],in_tuples[neuron] % discr->n_loc) > 0) {
            if (wram_get(discr->rams[neuron],in_tuples[neuron]) > 0) {
                sum++;
            }
        }
        // store responses
        return (double)sum/(double)discr->n_ram;
    }
    
    double classifySvmHistoDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr) {
        int neuron, sum=0;
        wkey_t address;
        int x, i, index, npixels=nt * nattr, n_bit = discr->n_bit, value;
        
        for (neuron=0;neuron<discr->n_ram;neuron++) {
            // compute neuron simulus
            address=(wkey_t)0;
            // decompose record data values into wisard input
            for (i=0;i<n_bit;i++) {
                x = discr->map[((neuron * n_bit) + i) % npixels];
                index = x/nt;
                value = (int) ((data[index] - off[index]) * nt / den[index]);
                if ((x % nt) < value) {
                    address |= (wkey_t)mypowers[n_bit -1 - i];
                }
            }
            if (wram_get(discr->rams[neuron],address) > 0) {
                sum++;
            }
        }
        // store responses
        return (double)sum/(double)discr->n_ram;
    }

    double classifySvmHistoDiscrThresholded(discr_t *discr, double *data, double *den, double *off, int nt, int nattr, double threshold) {
        int neuron, sum=0;
        wkey_t address;
        int x, i, index, npixels=nt * nattr, n_bit = discr->n_bit, value;
        
        for (neuron=0;neuron<discr->n_ram;neuron++) {
            // compute neuron simulus
            address=(wkey_t)0;
            // decompose record data values into wisard input
            for (i=0;i<n_bit;i++) {
                x = discr->map[((neuron * n_bit) + i) % npixels];
                index = x/nt;
                value = (int) ((data[index] - off[index]) * nt / den[index]);
                if ((x % nt) < value) {
                    address |= (wkey_t)mypowers[n_bit -1 - i];
                }
            }
            if (wram_get(discr->rams[neuron],address) > threshold) {
                sum++;
            }
        }
        // store responses
        return (double)sum/(double)discr->n_ram;
    }
    
    double classifySvmPointDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr) {
        int neuron, sum=0;
        
        wkey_t address;
        int x, i, index, npixels=nt * nattr, n_bit = discr->n_bit, value;
        for (neuron=0, sum=0;neuron<discr->n_ram;neuron++) {
            // compute neuron simulus
            address=(wkey_t)0;
            // decompose record data values into wisard input
            for (i=0;i<n_bit;i++) {
                x = discr->map[((neuron * n_bit) + i) % npixels];
                index = x/nt;
                value = (int) ((data[index] - off[index]) * nt / den[index]);
                if ((x % nt) == value -1) {
                    address |= mypowers[n_bit -1 - i];
                }
            }
            if (wram_get(discr->rams[neuron],address) > 0) {
                sum++;
            }
        }
        // store responses
        return (double)sum/(double)discr->n_ram;
    }
    
    double classifySvmPointDiscrThresholded(discr_t *discr, double *data, double *den, double *off, int nt, int nattr, double threshold) {
        int neuron, sum=0;
        wkey_t address;
        int x, i, index, npixels=nt * nattr, n_bit = discr->n_bit, value;
        
        for (neuron=0;neuron<discr->n_ram;neuron++) {
            // compute neuron simulus
            address=(wkey_t)0;
            // decompose record data values into wisard input
            for (i=0;i<n_bit;i++) {
                x = discr->map[((neuron * n_bit) + i) % npixels];
                index = x/nt;
                value = (int) ((data[index] - off[index]) * nt / den[index]);
                if ((x % nt) == value -1) {
                    address |= mypowers[n_bit -1 - i];
                }
            }
            if (wram_get(discr->rams[neuron],address) > threshold) {
                sum++;
            }
        }
        // store responses
        return (double)sum/(double)discr->n_ram;
    }
    
    double classifySvmCursorDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr) {
        int neuron, sum=0;
        wkey_t address;
        int x, i, index, npixels=nt * nattr, n_bit = discr->n_bit, value;
        int c1, c2, cursor, xr;

        for (neuron=0, sum=0;neuron<discr->n_ram;neuron++) {
            // compute neuron simulus
            address=(wkey_t)0;
            // decompose record data values into wisard input
            for (i=0;i<n_bit;i++) {
                x = discr->map[((neuron * n_bit) + i) % npixels];
                index = x/nt;
                value = (int) (data[index] - off[index]);
                if (den[index] == value) {
                    c1 = c2 = nt - 1;
                } else {
                    cursor = (int)((data[index] - off[index]) * (2 * nt - 1) / den[index]);
                    c1 = (int)(cursor/2.0);
                    if ((cursor % 2) > 0) c2 = (int)(cursor/2.0 + 1);
                    else c2 = (int)(cursor/2.0);
                }
                xr = x % nt;
                if (xr == c1 or xr == c2) {
                    address |= (wkey_t)mypowers[n_bit -1 - i];
                }
            }
            if (wram_get(discr->rams[neuron],address) > 0) {
                sum++;
            }
        }
        // store responses
        return (double)sum/(double)discr->n_ram;
    }
    
    double classifySvmCursorDiscrThresholded(discr_t *discr, double *data, double *den, double *off, int nt, int nattr, double threshold) {
        int neuron, sum=0;
        wkey_t address;
        int x, i, index, npixels=nt * nattr, n_bit = discr->n_bit, value;
        int c1, c2, cursor, xr;
        
        for (neuron=0;neuron<discr->n_ram;neuron++) {
            // compute neuron simulus
            address=(wkey_t)0;
            // decompose record data values into wisard input
            for (i=0;i<n_bit;i++) {
                x = discr->map[((neuron * n_bit) + i) % npixels];
                index = x/nt;
                value = (int) (data[index] - off[index]);
                if (den[index] == value) {
                    c1 = c2 = nt - 1;
                } else {
                    cursor = (int)((data[index] - off[index]) * (2 * nt - 1) / den[index]);
                    c1 = (int)(cursor/2.0);
                    if ((cursor % 2) > 0) c2 = (int)(cursor/2.0 + 1);
                    else c2 = (int)(cursor/2.0);
                }
                xr = x % nt;
                if (xr == c1 or xr == c2) {
                    address |= (wkey_t)mypowers[n_bit -1 - i];
                }
            }
            if (wram_get(discr->rams[neuron],address) > threshold) {
                sum++;
            }
        }
        // store responses
        return (double)sum/(double)discr->n_ram;
    }
    
    double classifySvmBinDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr) {
        int neuron, sum=0;
        wkey_t address;
        int x, i, index, npixels=nt * nattr, n_bit = discr->n_bit, value;
        int c1, c2, cursor, xr;
        
        for (neuron=0, sum=0;neuron<discr->n_ram;neuron++) {
            // compute neuron simulus
            address=(wkey_t)0;
            // decompose record data values into wisard input
            for (i=0;i<n_bit;i++) {
                x = discr->map[((neuron * n_bit) + i) % npixels];
                index = x/nt;
                value = (int)data[x/nt];
                if (((value >> (x % nt)) & 1) > 0) {
                    address |= (wkey_t)mypowers[n_bit -1 - i];
                }
            }
            if (wram_get(discr->rams[neuron],address) > 0) {
                sum++;
            }
        }
        // store responses
        return (double)sum/(double)discr->n_ram;
    }
    
    double classifyImgDiscr(discr_t *discr, const void * imgv, int cols, int bx, int by, int width, int tics) {
        const unsigned char * img = (unsigned char *) imgv;
        int neuron, n, sum;
        int n_bit = discr->n_bit;
        int n_ram = discr->n_ram;
        int index;
        int npixels = discr->size;
        unsigned char delta = (unsigned char)(256 / tics);
        int X, x, k, I, J;
        int val;
        wkey_t address;
        wkey_t key;
        for (neuron=0,sum=0;neuron<n_ram;neuron++) {
            address=(wkey_t)0;
            // make ram key to be fired
            for (int i=0; i < n_bit; ++i) {
                index = neuron * n_bit + i;
                X = discr->map[index % npixels];
                x = X / tics;
                k = X % tics;
                J = x % width + bx;
                I = x / width + by;
                val = (int)(img[I * cols + J] / delta);
                if (k < val)
                address += (wkey_t)mypowers[n_bit - i - 1];
            }
            if (wram_get(discr->rams[neuron],address) > 0) {
                sum++;
            }
        }
        // store responses
        return (double)sum/(double)discr->n_ram;
    }

    double classifyImgBinDiscr(discr_t *discr, const void * imgv, int cols) {
        const unsigned char * img = (unsigned char *) imgv;
        int neuron, n, sum;
        int n_bit = discr->n_bit;
        int n_ram = discr->n_ram;
        int index;
        int npixels = discr->size;
        int x, I, J;
        int val;
        wkey_t address;
        wkey_t key;
        for (neuron=0,sum=0;neuron<n_ram;neuron++) {
            address=(wkey_t)0;
            // make ram key to be fired
            for (int i=0; i < n_bit; ++i) {
                index = neuron * n_bit + i;
                x = discr->map[index % npixels];
                J = x % cols;
                I = x / cols;
                if (img[I * cols + J] == 0)
                    address += (wkey_t)mypowers[n_bit - i - 1];
            }
            if (wram_get(discr->rams[neuron],address) > 0) {
                sum++;
            }
        }
        // store responses
        return (double)sum/(double)discr->n_ram;
    }

    void classifyImgDiscrArray(discrarray_t *discr, const void * imgv, void * omgv, int cols, int bx, int by, int width, int tics, double thresh) {
        const unsigned char * img = (unsigned char *) imgv;
        unsigned char * omg = (unsigned char *) omgv;
        int n;
        int n_bit = discr->n_bit;
        int n_ram = discr->n_ram;
        int sum, neuron;
        int npixels = discr->size;
        unsigned char delta = (unsigned char)(256 / tics);
        int I, J, val;
        wkey_t address;
        
        for (n=0;n<npixels;n++) {
            J = n % width + bx;
            I = n / width + by;
            val = (int)(img[I * cols + J] / delta);
            for (neuron=0,sum=0;neuron<n_ram;neuron++) {
                address=(wkey_t)0;
                // make ram key to be fired
                for (int i=0; i < n_bit; ++i) {
                    if (discr->map[(neuron * n_bit + i) % tics] < val)
                        address += (wkey_t)mypowers[n_bit - i - 1];
                }
                if (wram_get(discr->rams[n][neuron],address) > 0) {
                    sum++;
                }
            }
            // store responses
            if ((double)sum/(double)n_ram > thresh) {
                omg[I * cols + J] = (unsigned char)255;
            } else {
                omg[I * cols + J] = (unsigned char)0;
            }
        }
    }
    
    double classifyDiscrThresholded(discr_t *discr, wkey_t *in_tuples, double threshold) {
        int neuron, sum;
        
        for (neuron=0, sum=0;neuron<discr->n_ram;neuron++) {
            if (wram_get(discr->rams[neuron],in_tuples[neuron]) > threshold) {
                sum++;
            }
        }
        // store responses
        return (double)sum/(double)discr->n_ram;
    }
    
    double classifySvmBinDiscrThresholded(discr_t *discr, double *data, double *den, double *off, int nt, int nattr, double threshold) {
        int neuron, sum;
        wkey_t address;
        int x, i, index, npixels=nt * nattr, n_bit = discr->n_bit, value;
        
        for (neuron=0;neuron<discr->n_ram;neuron++) {
            // compute neuron simulus
            address=(wkey_t)0;
            // decompose record data values into wisard input
            for (i=0;i<n_bit;i++) {
                x = discr->map[((neuron * n_bit) + i) % npixels];
                index = x/nt;
                value = (int)data[x/nt];
                if (((value >> (x % nt)) & 1) > 0) {
                    address |= mypowers[n_bit -1 - i];
                }
            }
            if (wram_get(discr->rams[neuron],address) > threshold) {
                sum++;
            }
        }
        // store responses
        return (double)sum/(double)discr->n_ram;
    }
    
    // wisard print function
    void printDiscr(discr_t *discr) {
        int neuron;
        fprintf(stdout,"<");
        for (neuron=0;neuron<discr->n_ram;neuron++)
        wram_print(discr->rams[neuron]);
        fprintf(stdout,">\n");
    }
    
    wvalue_t mentalDiscr(discr_t *discr) {
        wentry_t *p, *m;
        int neuron, i, offset=0, b;
        wvalue_t maxvalue=0, value;
        for (i=0; i< discr->size; i++) discr->mi[i] = 0;
        for (neuron=0,offset=0;neuron<discr->n_ram;neuron++,offset+=discr->n_bit) {
            m = discr->rams[neuron];
            for(p=m;;p=p->next) {
                if (p->next==m) break;
                for (b=0;b<discr->n_bit;b++) {
                    if (((p->next->key)>>(wkey_t)(discr->n_bit - 1 - b) & 1) > 0) {
                        value = discr->mi[discr->map[offset + b]] += p->next->value;
                        if (maxvalue < value) maxvalue = value;
                    }
                }
            }
        }
        discr->maxmi = maxvalue;
        return maxvalue;
    }
    
    // count non-zero ram entries
    int wnet_len(int *m, int size) {
        int i,cnt=0;
        
        for (i=0;i<size;i++) {
            if (m[i]>0) cnt++;
        }
        return cnt;
    }
    
    void sink_discriminator(wentry_t **discr, int n_ram) {
        int neuron;
        for (neuron=0; neuron < n_ram; neuron++) {
            wram_sink(discr[neuron]);
        }
    }
    
    
    // shrink discriminator
    void shrinkDiscr(discr_t *discr) {
        int neuron;
        for (neuron=0; neuron < discr->n_ram; neuron++) {
            wram_sink(discr->rams[neuron]);
            if (discr->maxkeys[neuron].value > 1 )
                discr->maxkeys[neuron].value -= (wvalue_t)1;
            else {
                discr->maxkeys[neuron].key = (wkey_t)-1;
                discr->maxkeys[neuron].value = (wvalue_t)-1;
            }
        }
    }
    
    // init discriminator
    void initDiscr(discr_t *discr) {
        int neuron;
        wentry_t *p;
        for (neuron=0; neuron < discr->n_ram; neuron++) {
            p = discr->rams[neuron];
            discr->rams[neuron] = (wentry_t *) wram_create();
            discr->maxkeys[neuron].key = (wkey_t)-1;
            discr->maxkeys[neuron].value = (wvalue_t)-1;
            free(p);
        }
    }
    
    // init discriminator
    void freeDiscr(discr_t *discr) {
        int neuron;
        wentry_t *p;
        for (neuron=0; neuron < discr->n_ram; neuron++) {
            wram_free(discr->rams[neuron]);
        }
        free(discr->rams);
        free(discr->maxkeys);
        free(discr->name);
        free(discr->map);
        free(discr->rmap);
        free(discr->mi);
        free(discr);
    }

}
