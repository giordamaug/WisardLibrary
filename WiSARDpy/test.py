from wisard import *

X = np.array(
   [ [0, 1, 0, 0, 0, 0, 0, 0],
     [0, 0, 1, 1, 1, 1, 0, 0],
     [0, 0, 1, 0, 0, 0, 1, 0],
     [1, 0, 0, 0, 0, 0, 0, 1],
     [1, 1, 0, 1, 1, 1, 1, 1],
     [1, 0, 0, 0, 0, 0, 0, 0],
     [0, 0, 0, 0, 1, 0, 0, 1],
     [1, 0, 0, 0, 0, 0, 0, 1]])

y = np.array(['A','A','B','B','A','A','B','A',])

w = WiSARD(2,bleaching=False,seed=424242)  # set seed 0 (or skip it) for nondeterminism
# train
w.fit(X, y)

# classify
print w.predict_proba(X), w.predict(X)
# classify by enabling bleaching
w.setBleaching()
print w.predict_proba(X), w.predict(X)
