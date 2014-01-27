make clean
make CC=clang CXX=clang++ TEST=TripCountProfiler report.csv

mv report.TripCountProfiler.csv TripCountProfiler.Vectors.report.csv

make clean
make CC=clang CXX=clang++ TCFLAGS=-usePericlesTripCount TEST=TripCountProfiler report.csv

mv report.TripCountProfiler.csv TripCountProfiler.Pericles.report.csv

make clean
make CC=clang CXX=clang++ TCFLAGS=-useHybridTripCount TEST=TripCountProfiler report.csv

mv report.TripCountProfiler.csv TripCountProfiler.Hybrid.report.csv
