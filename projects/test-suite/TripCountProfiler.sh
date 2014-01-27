export PATH=/home/raphael/bin:/home/raphael/llvm/Debug+Asserts/bin:/home/raphael/llvm/Debug+Asserts/lib:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/cuda/bin:/home/raphael/pagai3.3/pagai/src:/home/raphael/pagai3.3/pagai/external/z3/bin

make clean
make CC=clang CXX=clang++ TEST=TripCountProfiler report.csv

mv report.TripCountProfiler.csv TripCountProfiler.Vectors.report.csv

make clean
make CC=clang CXX=clang++ TCFLAGS=-usePericlesTripCount TEST=TripCountProfiler report.csv

mv report.TripCountProfiler.csv TripCountProfiler.Pericles.report.csv

make clean
make CC=clang CXX=clang++ TCFLAGS=-useHybridTripCount TEST=TripCountProfiler report.csv

mv report.TripCountProfiler.csv TripCountProfiler.Hybrid.report.csv
