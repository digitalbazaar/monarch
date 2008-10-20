#!/usr/bin/env python

# tf-graph.py
#
# Graph the output from tf-gen.sh.
#
# WARNING:
#    This is not a general purpose script and will need some hand holding
#    and customization to be useful.
#
# Copyright 2008 Digital Bazaar, Inc.
# David I. Lehn <dlehn@digitalbazaar.com>

import csv
import sys
import itertools
import pprint
from pylab import *

dataset = 'blog_0'

def getdata(f, fmt='%s'):
    csv_reader = csv.reader(f)
    data = []
    for row in csv_reader:
        if len(row) > 0 and not str(row[0]).startswith('#'):
            data.append([float(d) for d in row])
    data = array(data)
    pdata = {}
    for key, group in itertools.groupby(data, lambda d: d[0]):
        key = fmt % key
        pdata[key] = {}
        pdata[key]['operations'] = []
        pdata[key]['time'] = []
        for o in group:
            pdata[key]['operations'].append(o[1]) 
            pdata[key]['time'].append(o[4]) 
    #pprint.pprint(pdata)
    #pprint.pprint(pdata.keys())
    #pprint.pprint(sorted(pdata.keys()))
    #pprint.pprint([pdata[key]['time'][0] for key in sorted(pdata.keys())])
    return pdata

def _savefig(name):
    # 8px/dpi
    savefig(name + '.png')
    savefig(name + '.ps')
    savefig(name + '-500.png', dpi=62.5)
    savefig(name + '-1024.png', dpi=128)

if True:
    data = getdata(file('tf-data/s-ganesh-mfibers-d1-ttrue.0.1.8-ofalse.5.1.1000000000-ol1-%s.csv' % dataset), '%d threads')
    fdata = data
    for key in sorted(data.keys()):
        plot(data[key]['operations'], data[key]['time'], label=str(key))
        #semilogx(data[key]['operations'], data[key]['time'], label=str(key), basex=10)
        #loglog(data[key]['operations'], data[key]['time'], label=str(key))
    legend([str(k) for k in sorted(data.keys())], 'upper left', shadow=True)
    grid(1)
    xlabel('Work Units')
    ylabel('Time (s)')
    ylim(0,45)
    title('Workload Completion Time (small work units)\nMODEST Cooperative Concurrency Fibers + Thread Pool')
    _savefig('modest-fibers')
    clf()

if True:
    data = getdata(file('tf-data/s-ganesh-mmodest-d1-ttrue.0.1.8-ofalse.5.1.1000000000-ol1-%s.csv' % dataset), '%d threads')
    mdata = data
    for key in sorted(data.keys()):
        plot(data[key]['operations'], data[key]['time'], label=str(key))
        #semilogx(data[key]['operations'], data[key]['time'], label=str(key), basex=10)
        #loglog(data[key]['operations'], data[key]['time'], label=str(key))
    legend([str(k) for k in sorted(data.keys())], 'upper left', shadow=True)
    grid(1)
    xlabel('Work Units')
    ylabel('Time (s)')
    ylim(0,45)
    title('Workload Completion Time (small work units)\nMODEST Operations + Thread Pool')
    _savefig('modest-operations')
    clf()

if True:
    # this just works when thread count is linear from 1...N
    # http://matplotlib.sourceforge.net/pyplots/barchart_demo.py
    width = 0.35

    msthreads = sorted(mdata.keys())
    mthreads = arange(len(mdata))
    mtimes = [mdata[thr]['time'][-1] for thr in msthreads]
    mmax = max(mtimes)

    fsthreads = sorted(fdata.keys())
    fthreads = arange(len(fdata))
    ftimes = [fdata[thr]['time'][-1] for thr in fsthreads]
    fmax = max(ftimes)

    fmmax = max([mmax, fmax])
    mtimes /= fmmax
    ftimes /= fmmax

    mrects = bar(mthreads, mtimes, width, color='r')
    frects = bar(fthreads + width, ftimes, width, color='b')

    title('Speedup with 4 CPU Cores for 100000 Small Work Units')
    ylabel('Time (%)')
    xlabel('Threads')
    xticks(fthreads + width, [str(thr + 1) for thr in fthreads])

    legend((mrects[0], frects[0]), ('MODEST Operations', 'MODEST Fibers'))

    _savefig('thread-speedup')

    #legend([str(k) for k in sorted(data.keys())], 'upper left', shadow=True)
    clf()

if True:
    tags = (
        ('fibers', 'MODEST Fibers (4 threads)'),
        ('modest', 'MODEST Operations (4 threads)'),
        ('threads', 'POSIX Threads'))
    for test, name in tags:
        data = getdata(file('tf-data/s-ganesh-m%s-d1-ttrue.0.4.4-otrue.0.1.500-ol50-%s.csv' % (test, dataset)), '%s' % name)
        assert len(data.keys()) == 1
        for key in data.keys():
            plot(data[key]['operations'], data[key]['time'], label=str(key))
    legend([str(d[1]) for d in tags], 'upper left', shadow=True)
    grid(1)
    xlabel('Work Units')
    ylabel('Time (s)')
    ylim(0, 5.0)
    title('Workload Completion Time (large work units)')
    _savefig('comparison')
    clf()
