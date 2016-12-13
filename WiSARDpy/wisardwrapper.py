"""
    WiSARD C Library Wrapper in Python
    
    Created by Maurizio Giordano on 13/12/2016
    
"""

from ctypes import *
import os
import random
import platform
if platform.system() == 'Linux':
    suffix = '.so'
elif platform.system() == 'Windows':
    suffix = '.dll'
elif platform.system() == 'Darwin':
    suffix = '.dylib'
else:
    raise Error("Unsupported Platform")

libpath = "../libwisard-cxx_3.0" + suffix
wizl = CDLL(os.path.join(os.environ['PWD'], libpath))
HEADKEY = 18446744073709551615L

""" Mapping data structure """

c_value = c_double
c_key = c_ulong

class Wentry(Structure):
    pass

class Wkeymax(Structure):
    pass

Wkeymax._fields_ = [("key", c_key),
                    ("value", c_value)]

Wentry._fields_ = [("key", c_key),
                   ("value", c_value),
                   ("next", POINTER(Wentry)),
                   ("prev", POINTER(Wentry))]

class Discr(Structure):
    _fields_ = [("n_ram", c_int),
                ("n_bit", c_int),
                ("n_loc", c_key),
                ("size", c_int),
                ("tcounter", c_ulong),
                ("rams", POINTER(POINTER(Wentry))),
                ("maxkeys", POINTER(Wkeymax)),
                ("map", POINTER(c_int)),
                ("rmap", POINTER(c_int)),
                ("mi", POINTER(c_value)),
                ("maxmi", c_value),
                ("name", c_char_p)]

def discr_n_ram(discr):
    return discr.contents.n_ram

""" RAM interface """
_wram_create = wizl.wram_create
_wram_create.restype =  POINTER(Wentry)
_wram_set = wizl.wram_set
_wram_set.argtypes = [ POINTER(Wentry), c_key, c_value ]
_wram_set.restype =  c_value
_wram_incr = wizl.wram_incr
_wram_incr.argtypes = [ POINTER(Wentry), c_key ]
_wram_incr.restype =  c_value
_wram_decr = wizl.wram_decr
_wram_decr.argtypes = [ POINTER(Wentry), c_key ]
_wram_decr.restype =  c_value
_wram_decr_all_but_key = wizl.wram_decr_all_but_key
_wram_decr_all_but_key.argtypes = [ POINTER(Wentry), c_key ]
_wram_decr_all_but_key.restype =  c_value
_wram_del = wizl.wram_del
_wram_del.argtypes = [ POINTER(Wentry), c_key ]
_wram_del.restype =  c_value
_wram_get = wizl.wram_get
_wram_get.argtypes = [ POINTER(Wentry), c_key ]
_wram_get.restype =  c_value
_wram_len = wizl.wram_len
_wram_len.argtypes = [ POINTER(Wentry) ]
_wram_len.restype =  c_int

def wram_build(n,dict):
    ram = _wram_create()
    maxkey = 2**n
    for key,value in dict.items():
        if key > 0:
            _wram_set(ram,key,value)
    return ram

def wram_get(ram, key):
    return _wram_get(ram,c_key(key))

def wram_incr(ram, key):
    return _wram_incr(ram,c_key(key))

def wram_decr(ram, key):
    return _wram_decr(ram,c_key(key))

def wram_decr_all_but_key(ram,key):
    return _wram_decr_all_but_key(ram,c_key(key))

def wram_del(ram, key):
    return _wram_del(ram,c_key(key))

def wram_set(ram, key, value):
    return _wram_set(ram,c_key(key),c_value(value))

def showRam(ram):
    size = _wram_len(ram)
    p = ram.contents.next
    res = {}
    while p.contents.key != HEADKEY:
        res[int(p.contents.key)] = p.contents.value
        p = p.contents.next
    return res

""" Constructor interface """
_makeDiscr =  wizl.makeDiscr
_makeDiscr.restype =  POINTER(Discr)
_makeDiscr.argtypes = [ c_int, c_int, c_char_p, c_char_p ]

def makeDiscr(nbit, size, name='anonym', maptype='random'):
    return _makeDiscr(c_int(nbit), c_int(size), c_char_p(name), c_char_p(maptype))

""" Train/Classify wrappers"""
_trainDiscr = wizl.trainDiscr
_trainDiscr.argtypes = [ POINTER(Discr), POINTER(c_key) ]
_trainforgetDiscr = wizl.trainforgetDiscr
_trainforgetDiscr.argtypes = [ POINTER(Discr), POINTER(c_key) , c_value, c_value]
_classifyDiscr = wizl.classifyDiscr
_classifyDiscr.argtypes = [ POINTER(Discr), POINTER(c_key) ]
_classifyDiscr.restype = c_double
_classifyDiscrThresholded = wizl.classifyDiscrThresholded
_classifyDiscrThresholded.argtypes = [ POINTER(Discr), POINTER(c_key), c_value ]
_classifyDiscrThresholded.restype = c_double

_responseDiscr = wizl.responseDiscr
_responseDiscr.argtypes = [ POINTER(Discr), POINTER(c_key) ]
_responseDiscr.restype = POINTER(c_double)

def trainDiscr(discr, intuple):
    n = len(intuple)
    if n != discr.contents.n_ram:
        raise NameError('Wrong InTuple size')
    c_tuple = (c_ulong * n)(*intuple)
    _trainDiscr(discr, c_tuple)

def trainforgetDiscr(discr, intuple, incr, decr):
    n = len(intuple)
    if n != discr.contents.n_ram:
        raise NameError('Wrong InTuple size')
    c_tuple = (c_ulong * n)(*intuple)
    _trainforgetDiscr(discr, c_tuple, incr, decr)

def classifyDiscr(discr, intuple):
    n = len(intuple)
    if n != discr.contents.n_ram:
        raise NameError('Wrong InTuple size')
    c_tuple = (c_ulong * n)(*intuple)
    return _classifyDiscr(discr, c_tuple)

def classifyDiscrThresholded(discr, intuple, threshold):
    n = len(intuple)
    if n != discr.contents.n_ram:
        raise NameError('Wrong InTuple size')
    c_tuple = (c_ulong * n)(*intuple)
    return _classifyDiscrThresholded(discr, c_tuple, threshold)

def responseDiscr(discr, intuple):
    n = len(intuple)
    if n != discr.contents.n_ram:
        raise NameError('Wrong InTuple size')
    rawres = _responseDiscr(discr, (c_ulong * n)(*intuple))
    return [ rawres[i] for i in range(discr.contents.n_ram)]


""" Mental Image methods """
_mentalDiscr = wizl.mentalDiscr
_mentalDiscr.argtypes = [ POINTER(Discr) ]
_mentalDiscr.restype = c_value

def mentalDiscr(discr):
    return _mentalDiscr(discr)

""" Utilities wrapper """
_printDiscr =  wizl.printDiscr

def printDiscr(discr):
    _printDiscr(discr)

def getNBitDiscr(discr):
    return discr.contents.n_bit

def getNRamDiscr(discr):
    return discr.contents.n_ram

def getNameDiscr(discr):
    if not discr.contents.name:
        return ''
    return discr.contents.name

def getTcounterDiscr(discr):
    return discr.contents.tcounter

def getMapRefDiscr(discr, index):
    return discr.contents.map[index]

def getRMapRefDiscr(discr, index):
    return discr.contents.rmap[index]

def getSizeDiscr(discr):
    return discr.contents.size

def getMaxMIDiscr(discr):
    return discr.contents.maxmi

def getMIRefDiscr(discr,index):
    return discr.contents.mi[index]

def getMapDiscr(discr):
    if not discr.contents.map:
        return []
    res = [ None for i in range(discr.contents.size)]
    for i in range(discr.contents.size):
        res[i] = discr.contents.map[i]
    return res

def setMapDiscr(discr,map):
    if discr.contents.size != len(map):
        raise Error("Error: input map has wrong size")
    for i in range(discr.contents.size):
        discr.contents.map[i] = map[i]
        discr.contents.rmap[map[i]] = i
    return

def getRMapDiscr(discr):
    if not discr.contents.rmap:
        return []
    res = [ None for i in range(discr.contents.size)]
    for i in range(discr.contents.size):
        res[i] = discr.contents.rmap[i]
    return res

def getMaxKeysDiscr(discr):
    if not discr.contents.maxkeys:
        return []
    return [ (discr.contents.maxkeys[i].key,discr.contents.maxkeys[i].value) for i in range(discr.contents.n_ram)]

def getMIDiscr(discr):
    if not discr.contents.mi:
        return []
    res = [ None for i in range(discr.contents.size)]
    for i in range(discr.contents.size):
        res[i] = discr.contents.mi[i]
    return res

def getRamsDiscr(discr):
    res = [ None for i in range(discr.contents.n_ram)]
    for i in range(discr.contents.n_ram):
        ram = discr.contents.rams[i]
        p = ram.contents.next
        res[i] = {}
        while p.contents.key != HEADKEY:
            res[i][int(p.contents.key)] = p.contents.value
            p = p.contents.next
    return res

