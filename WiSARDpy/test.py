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

w = WiSARD(2,bleaching=False,seed=424242)
# train
w.fit(X, y)

# classify
result = w.predict_proba(X)
# classify by enabling bleaching
w.setBleaching()
result_b = w.predict_proba(X)

print result
print result_b
