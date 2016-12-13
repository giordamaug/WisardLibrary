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

w = WiSARD(2,bleaching=False)
# train
w.fit(X, y)

# classify
w.predict(X)
# classify by enabling bleaching
w.setBleaching()
w.predict(X)
