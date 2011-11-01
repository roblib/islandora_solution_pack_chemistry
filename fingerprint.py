#!/usr/bin/python

from indigo import *
import sys

indigo = Indigo()

def dec2hex(n):
    """return the hexadecimal string representation of integer n"""
    return "%X" % n
 
def hex2dec(s):
    """return the integer value of a hexadecimal string s"""
    return int(s, 16)

mol1 = indigo.loadMoleculeFromFile(sys.argv[1]);

fp1 = mol1.fingerprint("sub");
fp1string = fp1.toString();

n = 8    
l1 = [hex2dec(fp1string[i:i+n]) for i in range(0, len(fp1string), n)]

print l1
