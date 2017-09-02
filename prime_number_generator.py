#!/usr/bin/env python

from __future__ import division
import math

primes = []
#stop = 1000100
stop = (1 << 20) + 100
for i in xrange(2, stop):
    limit = int(math.sqrt(i) + 1)
    prime = True
    for j in xrange(2, limit):
        if i % j == 0:
            prime = False
            break
    if prime:
        primes.append(i)

# print ' '.join([str(x) for x in primes])
# print ""
# print primes[-1]

cutoff = 1 << 20
print ' '.join([str(x) for x in primes if x > cutoff])
