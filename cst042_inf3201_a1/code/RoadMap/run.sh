#!/bin/bash -l

#Specifies the number of hosts to use
# -Max 52 for quad, 4 procs per host
# -Max 32 for dual, 2 procs per host
# -Number does not apply for when running all hosts
# original 4
NUM_HOSTS=20

if [ $# -eq 0 ]
  then
    echo "Not specified which nodes to run on. Use either: dual, quad, all"
    exit
fi

rm hostfile
touch hostfile

if [ $# != 2 ]; then
  echo "localhost slots=1" > hostfile
fi
if [ "$1" = "dual" ]; then
  head -n $NUM_HOSTS hostfile_dual >> hostfile
elif [ "$1" = "quad" ]; then
  head -n $NUM_HOSTS hostfile_quad >> hostfile
elif [ "$1" = "all" ]; then
  cat hostfile_quad >> hostfile
  echo -e "\n" >> hostfile
  cat hostfile_dual >> hostfile
fi


if [ "$2" = "profile" ]; then
  mpirun -x VT_MAX_FLUSHES=50 -x VT_FILTER_SPEC=mpivt_filter -hostfile hostfile RoadMapProf
  rm result.tex
  touch result.tex
  otfprofile RoadMapProf.otf
  pdflatex result.tex
elif [ "$2" = "gprof" ]; then
  mpirun -hostfile hostfile RoadMapGProf
  gprof -l RoadMapGProf gmon.*
elif [ "$2" = "gperf" ]; then
  LD_LIBRARY_PATH=/state/partition1/apps/lib:$LD_LIBRARY_PATH CPUPROFILE=profile.out mpirun -hostfile hostfile RoadMapGPerf
  /state/partition1/apps/bin/pprof --lines --text RoadMapGPerf profile.out
else
  mpirun -hostfile hostfile RoadMap
fi
