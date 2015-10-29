import numpy
import sys

values = sys.argv[1:]
values_double = map(float, values)
print "Mean: " + str(numpy.mean(values_double))
print "Std:  " + str(numpy.std(values_double))
