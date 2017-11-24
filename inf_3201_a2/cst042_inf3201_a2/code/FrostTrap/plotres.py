#!/usr/bin/env python3
# WILL NOT WORK ON THE CLUSTER. The required modules are not installed


import numpy as np
import matplotlib.pyplot as plt

ind = np.arange(17)

samples = [eval(l) for l in open("results1_c3.out").readlines() if 'usecs' in l]

xvals = np.array(list(sorted(list({s['max_threads'] for s in samples}))))
# assume sorted order of experiment results.

res_rb     = [s['secs'] for s in samples if s['scheme'] is 'rb']
res_simple = [s['secs'] for s in samples if s['scheme'] is 'simple']
res_dbuf   = [s['secs'] for s in samples if s['scheme'] is 'dbuf']

fig, ax = plt.subplots()

width = 0.25
rects1 = ax.bar(xvals,         res_rb, width, color='r') # no yerr
rects2 = ax.bar(xvals+width,   res_simple, width, color='g') # no yerr
rects3 = ax.bar(xvals+2*width, res_dbuf, width, color='b') # no yerr

plt.title('Execution time with Frost Trap algorithms')
plt.ylabel('Time used in seconds')
plt.xlabel('Number of threads')
plt.legend([rects1, rects2, rects3], ['Red black', 'Simple', 'Double buffering'], loc='upper mid')


plt.show()
