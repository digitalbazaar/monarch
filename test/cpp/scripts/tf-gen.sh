#!/bin/sh

# tf-gen.sh
#
# Use various test-fiber options to generate data for pretty graphs.
# Also see tf-graph.py.
#
# WARNING:
#    This is not a general purpose script and will need some hand holding
#    and customization to be useful.
#
# Copyright 2008 Digital Bazaar, Inc.
# David I. Lehn <dlehn@digitalbazaar.com>

s() {
	host=$1
	mode=$2
	dyno=$3
	tmag=$4
	omag=$5
	fn=s-${host}-m${mode}-d${dyno}-t${tmag}-o${omag}.csv
	echo '#' ${fn}
	echo $ROOTDIR/dbcore-run $BINDIR/test-fiber \
		-i -l 0 -t jsonmatrix \
		--json-option csv true \
		--option mode ${mode} \
		--option dyno ${dyno} \
		--option tmag ${tmag} \
		--option omag ${omag}
		#| tee tf-data/${fn}
}

s1() {
	host=$1; shift;
	mode=$1; shift;
	dyno=$1; shift;
	tlin=$1; shift;
	tmag=$1; shift;
	tmin=$1; shift;
	tmax=$1; shift;
	olin=$1; shift;
	omag=$1; shift;
	omin=$1; shift;
	omax=$1; shift;
	oploops=$1; shift;
	tag=$1; shift;
	fn=s-${host}-m${mode}-d${dyno}-t${tlin}.${tmag}.${tmin}.${tmax}-o${olin}.${omag}.${omin}.${omax}-ol${oploops}-${tag}.csv
	echo '#' ${fn}
	$ROOTDIR/dbcore-run $BINDIR/test-fiber \
		-i -l 0 -t jsonmatrix \
		--json-option csv true \
		--option mode ${mode} \
		--option dyno ${dyno} \
		--option tlin ${tlin} \
		--option tmag ${tmag} \
		--option tmin ${tmin} \
		--option tmax ${tmax} \
		--option olin ${olin} \
		--option omag ${omag} \
		--option omin ${omin} \
		--option omax ${omax} \
		--option oploops ${oploops} \
		| tee tf-data/${fn}
}

t0() {
	#for dyno in 1 2; do
	for dyno in 1; do
		#for mode in fiber modest; do
		for mode in fibers modest; do
			s $HOST $mode $dyno 3 5
		done
		s $HOST threads $dyno 3 0
	done
}

tlow() {
	host=$1
	tag=$2
	#for dyno in 1 2; do
	for dyno in 1; do
		for mode in fibers modest threads; do
		#for mode in fibers ; do
			#s1 ${host} ${mode} ${dyno} true 0 1 400 false 4 1 1000000000 ${tag}
			ops=1000
			s1 ${host} ${mode} ${dyno} true 0 1 400 true 0 ${ops} ${ops} ${tag}
			#s1 ${host} ${mode} ${dyno} true 0 1 400 false 3 1 1000000 ${tag}
		done
	done
}

# test generation for blog post
tblog() {
	host=$1
	tag=blog_$2
	#for dyno in 1 2; do
	huge=1000000000 # dummy huge max value
	for dyno in 1; do
		for mode in fibers modest; do
			s1 ${host} ${mode} ${dyno} \
				true 0 1 8 \
				false 5 1 ${huge} \
				1 ${tag}
		done
	done
	# limit to 500 ops since threads will die at ~381 for 32bit cpu
	# increase oploops to avoid noise with low count of fast ops
	# setting threads to 4 to optimze fibers and modest on a four
	# core test machine
	cores=4
	for mode in fibers modest threads; do
		s1 ${host} ${mode} 1 \
			true 0 ${cores} ${cores} \
			true 0 1 500 \
			50 ${tag}
	done
}

ROOTDIR=../../..
BINDIR=../dist/linux
OUTDIR=tf-data
mkdir -p $OUTDIR
HOST=`hostname`
#t0
#tlow $HOST 0
tblog $HOST 0
