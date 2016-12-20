"""
    WiSARD Python Class
    
    Created by Maurizio Giordano on 13/12/2016
    
"""

import wisardwrapper as wisapi
import numpy as np

class WiSARD:

    def __init__(self,
                 nobits,
                 bleaching=False,
                 default_bleaching=1,
                 confidence_bleaching=0.1,
                 randomized=True,
                 seed=424242):
        if (not isinstance(nobits, int)):
            raise Exception('number of bits must be an integer')
        if (not isinstance(bleaching, bool)):
            raise Exception('bleaching flag must be a boolean')
        if (not isinstance(default_bleaching, int)):
            raise Exception('bleaching downstep must be an integer')
        if (not isinstance(randomized, bool)):
            raise Exception('randomization flag must be a boolean')
        if (not isinstance(seed, int)):
            raise Exception('random seed must be an integer')
        if (not isinstance(confidence_bleaching, float)):
            raise Exception('bleaching confidence must be a float between 0 and 1')
        self._nobits = nobits
        self._bleaching = bleaching
        self._def_bleaching = default_bleaching
        self._conf_bleaching = confidence_bleaching
        self._randomized = randomized
        self._seed = seed

        # attributes set after initialization
        self._retina_size = 0
        self._norams_x_discr = 0
        self._discriminators = {}
        self._classes = []

    def getNoBits(self):
        return self._nobits

    def getSize(self):
        return self._retina_size

    def getNoRams(self):
        return self._discriminators * self._norams_x_discr

    def getDiscr(self,name):
        return self._discriminators[name]

    def isBleachingSet(self):
        return self._bleaching

    def setBleaching(self):
        self._bleaching = True

    def unsetBleaching(self):
        self._bleaching = False

    def printDiscr(self,name):
        return wisapi.printDiscr(self._discriminators[name])
    
    # make tuple input for discriminator (from array of bits)
    def _mkTuple(self,discr,sample):
        if (isinstance(sample, list)):
            sample = np.array(sample)
        if (isinstance(sample,np.ndarray)) and (len(sample.shape) == 1):
            raw_sample = sample
        else:
            raise Exception("sample must be flat list (or array) of integers: %s"%sample)
            
        intuple = [0 for i in range(self._norams_x_discr)]
        for i in range(self._norams_x_discr):
            for index in range(self._nobits):
                x = wisapi.getMapRefDiscr(discr, (i * self._nobits) + index) % self._retina_size
                intuple[i] += (2**(self._nobits -1 - index)) * sample[x]
        return intuple
            
    # create and train wisard on labeled dataset X,y
    # X is a matrix of retinas (each line will be a retina)
    # y is a list of label (each line defines a retina in the same position in Y)
    def fit(self, X, y):
        # creating discriminators
        self._retina_size = len(X[0])
        classes = set(y)
        
        for clname in y:
            # use WiSARD C wrapper
            d = wisapi.makeDiscr(self._nobits,
                                 self._retina_size,
                                 name=str(clname),
                                 maptype = 'random' if self._randomized else 'linear')
                              
            self._discriminators[str(clname)] = d
        
        self._norams_x_discr = wisapi.getNRamDiscr(self._discriminators[self._discriminators.keys()[0]])
        self._classes = self._discriminators.keys()
        
        # add training
        num_samples = len(y)
        for i in xrange(num_samples):
            retina = X[i]
            label = y[i]
            
            # make tuple from array of bits
            tuple = self._mkTuple(self._discriminators[label],retina)
            wisapi.trainDiscr(self._discriminators[label],tuple)

    #  X is a list of of lists of retinas (or a 2-dim array of retinas)
    def predict(self, X):
        final_result = []
        X = np.array(X)  # convert to numpy matrix
        results = self.predict_proba(X)
        for res in results:
            index = np.argmax(res)
            final_result.append(self._classes[index])
        
        return final_result

    def predict_proba(self, X):
        result = []
        if self._bleaching:
            return np.apply_along_axis(self._predict_bleaching,1,X)
        else:
            return np.apply_along_axis(self._predict_no_bleaching,1,X)

    def _predict_no_bleaching(self, x):
        return np.array([wisapi.classifyDiscr(self._discriminators[class_name],self._mkTuple(self._discriminators[class_name],x)) for class_name in self._classes])

    def _predict_bleaching(self, x):
        b = self._def_bleaching
        confidence = 0.0
        result_partial = None
        
        res_disc = np.array([wisapi.responseDiscr(self._discriminators[class_name],self._mkTuple(self._discriminators[class_name],x)) for class_name in self._classes])
        
        while confidence < self._conf_bleaching:
            result_partial = np.sum(res_disc >= b, axis=1)
            confidence = self._calc_confidence(result_partial)
            b += 1
            if(np.sum(result_partial) == 0):
                result_partial = np.sum(res_disc >= 1, axis=1)
                break
        result_sum = np.sum(result_partial, dtype=np.float32)
        result = np.array(result_partial)/result_sum
        
        return result

    def _calc_confidence(self, results):
        # get max value
        max_value = results.max()
        if(max_value == 0):  # if max is null confidence will be 0
            return 0

        # if there are two max values, confidence will be 0
        position = np.where(results == max_value)
        if position[0].shape[0]>1:
            return 0

        # get second max value
        second_max = results[results < max_value].max()
        if results[results < max_value].size > 0:
            second_max = results[results < max_value].max()
        
        # calculating new confidence value
        c = 1 - float(second_max) / float(max_value)
        
        return c
