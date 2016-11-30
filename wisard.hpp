//
//  wisard.hpp
//
//
//  Created by Maurizio Giordano on 20/03/2014
//
//
#ifndef _wisard_h
#define _wisard_h

#define PI 3.1415926535

#include "wnet_lib.hpp"
#include <assert.h>
#include <stdio.h>


extern "C"  //Tells the compile to use C-linkage for the next scope.
{
    
#define BUFSIZE 1024
#define ceiling(X) (X-(int)(X) > 0 ? (int)(X+1) : (int)(X))
    typedef unsigned char uchar;
    typedef struct wisard wisard_t;
    
    /// discriminator data structure
    /**
     represent a disciminator and its configuration and size info
     */
    typedef struct {
        int n_ram;          /**< number of rams for the discriminator */
        int n_bit;          /**< number of bits (resolution) */
        wkey_t n_loc;          /**< number of location in each ram (minus 1) */
        int size;           /**< size of input binary image */
        unsigned long int tcounter;  /**< train counter */
        wentry_t **rams;    /**< the ram list of disciminator */
        wkeymax_t *maxkeys;    /**< the list ofmore frequent keys */
        int *map;           /**< pointer to the retina (mapping) */
        int *rmap;          /**< pointer to the inverse retina (mapping) */
        wvalue_t *mi;        /**< pointer to mental image */
        wvalue_t maxmi;      /**< max mental image value */
        char *name;         /**< the name of the discriminator */
    } discr_t;
    
    /// discriminator data structure
    /**
     represent a array of discriminators
     */
    typedef struct {
        int n_ram;          /**< number of rams per discriminator */
        int n_bit;          /**< number of bits (resolution) */
        wkey_t n_loc;          /**< number of location in each ram (minus 1) */
        int size;           /**< size of input binary image */
        unsigned long int tcounter;  /**< train counter */
        wentry_t ***rams;    /**< the ram matrix of the array discriminator */
        wkeymax_t **maxkeys;    /**< the matrix of more frequent keys */
        int *map;           /**< pointer to the retina (mapping) */
        int *rmap;          /**< pointer to the inverse retina (mapping) */
        wvalue_t *mi;        /**< pointer to mental image */
        wvalue_t maxmi;      /**< max mental image value */
        char *name;         /**< the name of the discriminator */
    } discrarray_t;
    //! discriminator constructor function.
    /*!
     \param n_bit number of bits (resolution).
     \param size input binary image size.
     \param name discriminator name.
     \param mode retina creation mode (linear, random).
     \return a pointer to the discriminator data structure
     */
    discr_t *makeDiscr(int n_bit, int size, char *name, char *mode);
    discr_t *makeTrainImgDiscr(int n_bit, int size, char *name, char *mode, const void * imgv, int cols, int bx, int by, int width, int tics);
    discr_t *makeTrainImgBinDiscr(int n_bit, int size, char *name, char *mode, const void * imgv, int cols);
    discrarray_t *makeDiscrArray(int n_bit, int size, char *name, char *mode, int tics);
    discrarray_t *makeTrainImgDiscrArray(int n_bit, int size, char *name, char *mode, const void * imgv, int cols, int bx, int by, int width, int tics);
    
    //! discriminator copying function.
    /*!
     \param discr pointer to the discriminato to duplicate
     \return a pointer to the copied discriminator
     */
    discr_t *copyDiscr(discr_t *discr);
    //! discriminator training function.
    /*!
     \param discr pointer to the discriminato to train
     \param in_tuples input tuple for training the discriminator
     \return void
     */
    void trainDiscr(discr_t *discr, wkey_t *in_tuples);
    void trainSvmHistoDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr);
    void trainSvmPointDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr);
    void trainSvmCursorDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr);
    void trainSvmBinDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr);
    void trainImgDiscr(discr_t *discr, const void * imgv, int cols, int bx, int by, int width, int tics);
    void trainImgBinDiscr(discr_t *discr, const void * imgv, int cols);
    void trainresetImgDiscr(discr_t *discr, const void * imgv, int cols, int bx, int by, int width, int tics);
    
    void trainImgDiscrArray(discrarray_t *discr, const void * imgv, int cols, int bx, int by, int width, int tics);
    //! discriminator training and forgetting function.
    /*!
     \param discr pointer to the discriminato to train
     \param in_tuples input tuple for training the discriminator (the rest wil be decremented)
     \return void
     */
    void trainforgetDiscr(discr_t *discr, wkey_t *in_tuples, wvalue_t incr, wvalue_t decr);
    void trainforgettopDiscr(discr_t *discr, wkey_t *in_tuples, wvalue_t incr, wvalue_t decr, wvalue_t top);
    void trainforgetImgDiscr(discr_t *discr, const void * imgv, int cols, int bx, int by, int width, int tics);
    void trainforgetImgBinDiscr(discr_t *discr, const void * imgv, int cols);
    void trainforgetImgDiscrArray(discrarray_t *discr, const void * imgv, int cols, int bx, int by, int width, int tics);
    
    //! discriminator classifyning functions.
    /*!
     \param discr pointer to the discriminato to train
     \param in_tuples input tuple for having response from the discriminator
     \return the classification response
     */
    double classifyDiscr(discr_t *discr, wkey_t *in_tuples);
    double classifySvmHistoDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr);
    double classifySvmPointDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr);
    double classifySvmCursorDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr);
    double classifySvmBinDiscr(discr_t *discr, double *data, double *den, double *off, int nt, int nattr);
    double classifyImgDiscr(discr_t *discr, const void * imgv, int cols, int bx, int by, int width, int tics);
    double classifyImgBinDiscr(discr_t *discr, const void * imgv, int cols);
    void classifyImgDiscrArray(discrarray_t *discr, const void * imgv, void * oimgv, int cols, int bx, int by, int width, int tics, double thresh);
    /*!
     \param discr pointer to the discriminato to train
     \param in_tuples input tuple for having response from the discriminator
     \param threshold value to enable neuron outputs
     \return the classification response
     */
    double classifyDiscrThresholded(discr_t *discr, wkey_t *in_tuples, double threshold);
    double classifySvmHistoDiscrThresholded(discr_t *discr, double *data, double *den, double *off, int nt, int nattr, double threshold);
    double classifySvmPointDiscrThresholded(discr_t *discr, double *data, double *den, double *off, int nt, int nattr, double threshold);
    double classifySvmCursorDiscrThresholded(discr_t *discr, double *data, double *den, double *off, int nt, int nattr, double threshold);
    double classifySvmBinDiscrThresholded(discr_t *discr, double *data, double *den, double *off, int nt, int nattr, double threshold);
    
    //! discriminator punishing function.
    /*!
     \param discr pointer to the discriminato to punish
     \param in_tuples input tuple for punishing the discriminator
     \return void
     */
    void punishDiscr(discr_t *discr, wkey_t *in_tuples, wvalue_t);
    
    //! discriminator printing function.
    /*!
     \param discr pointer to the discriminato to train
     \return void
     */
    void printDiscr(discr_t *discr);
    // ! discriminator mental image update function
    /*!
     \param discr the discriminator
     \return athe max value in the mentalimage
     */
    wvalue_t mentalDiscr(discr_t *discr);
    
    // ! Halfing Discriminator Resolution
    /*!
     \param discr the discriminator to be halfed
     \return a new discrminator with half resolution
     */
    discr_t *cut_ram(discr_t *discr);
    // ! Doubling Discriminator Resolution
    /*!
     \param discr the discriminator to be halfed
     \return a new discrminator with half resolution
     */
    discr_t *join_ram(discr_t *discr);
    
    // ! Shrinking Discriminator (for Bleaching)
    /*!
     \param discr the discriminator to be shrinked
     \return void
     */
    void shrinkDiscr(discr_t *discr);
    
    // ! Emptying Rams of Discriminator (for Bleaching)
    /*!
     \param discr the discriminator to be initialized
     \return void
     */
    void initDiscr(discr_t *discr);
    
    // ! Free Discriminator
    /*!
     \param discr the discriminator to be initialized
     \return void
     */
    void freeDiscr(discr_t *discr);
    
    void sink_discriminator(wentry_t **discr, int n_ram);
    int *filter(double *ls, int *idx, int n, int *cnt, double factor);
    int new_chsize(double *ls, int n, double factor);
    int bleaching(int *challengers, wentry_t ***discr_tobleach, int csize, double *responses, wisard_t *wiznet, double factor, wkey_t *in_tuples, double *bres, int flag);
}
#endif
