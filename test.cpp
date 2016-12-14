//
//  test.cpp
//
//
//  Created by Maurizio Giordano on 20/03/2014
//
// the WISARD C++ implementation
//

#include "wisard.hpp"
#include <iostream>
#include <string>

wkey_t *mkTuple(discr_t *discr, int *sample) {
    int i,j;
    /* alloc tuple array */
    wkey_t *intuple = (wkey_t *)malloc(discr->n_ram * sizeof(wkey_t));
    int x;
    for (i = 0; i < discr->n_ram; i++)
        for (j = 0; j < discr->n_bit; j++) {
            x = discr->map[(i * discr->n_bit) + j] % discr->size;
            intuple[i] += (2^(discr->n_bit -1 - j)) * sample[x];
    }
    return intuple;
}

int main() {
   
    int X[8][8] ={{0, 1, 0, 0, 0, 0, 0, 0},
                  {0, 0, 1, 1, 1, 1, 0, 0},
                  {0, 0, 1, 0, 0, 0, 1, 0},
                  {1, 0, 0, 0, 0, 0, 0, 1},
                  {1, 1, 0, 1, 1, 1, 1, 1},
                  {1, 0, 0, 0, 0, 0, 0, 0},
                  {0, 0, 0, 0, 1, 0, 0, 1},
                  {1, 0, 0, 0, 0, 0, 0, 1}};
    std::string y[8] = {"A","A","B","B","A","A","B","A"};
    wvalue_t responses[2];
    int s;
    int test[8] = {0, 0, 1, 0, 0, 0, 1, 0};
    
    // init WiSARD (create discriminator for each class "A" and "B")
    discr_t wisard[2];
    wisard[0] = *makeDiscr(2,8,"A","random");
    wisard[1] = *makeDiscr(2,8,"B","random");

    // train WiSARD
    for (s=0; s < 8; s++)
        if (y[s] == "A")
            trainDiscr(wisard,mkTuple(wisard,X[s]));
        else
            trainDiscr(wisard+1,mkTuple(wisard+1,X[s]));
    
    // predict by WiSARD
    responses[0] = classifyDiscr(wisard,mkTuple(wisard,test));
    responses[1] = classifyDiscr(wisard+1,mkTuple(wisard+1,test));
    if (responses[0] > responses[1])
        std::cout << "Response is A" << std::endl;
    else
        std::cout << "Response is B" << std::endl;
}
