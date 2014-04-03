custom_clean () { 
	find . -type f -name "*.linked.rbc.instr.bc" -delete
	find . -type f -name "*.instrumentation.stats" -delete
	find . -type f -name "*.llc-instr.o" -delete
	find . -type f -name "*.llc-instr.s" -delete
	find . -type f -name "*.llc-instr" -delete
	find . -type f -name "*.out-llc-instr" -delete
	find . -type f -name "*.loops.out" -delete
	find . -type f -name "*.out-llc-instr-loops" -delete
	find . -type f -name "*.instr-output" -delete
	find . -type f -name "*.clean-out-llc-instr" -delete
	find . -type f -name "*.diff-llc-instr" -delete
	find . -type f -name "*.exe-llc-instr" -delete
	find . -type f -name "*.nightly.instr.report.txt" -delete
	find . -type f -name "*.TripCountProfiler.report.txt" -delete
	find . -type f -name "*report.TripCountProfiler.csv" -delete
	find . -type f -name "*report.TripCountProfiler.txt" -delete
	find . -type f -name "*report.TripCountProfiler.raw.out" -delete
}



export PATH=/home/raphael/bin:/home/raphael/llvm/Debug+Asserts/bin:/home/raphael/llvm/Debug+Asserts/lib:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/cuda/bin:/home/raphael/pagai3.3/pagai/src:/home/raphael/pagai3.3/pagai/external/z3/bin

#make clean
#make CC=clang CXX=clang++ TEST=TripCountProfiler report.csv

#mv report.TripCountProfiler.csv TripCountProfiler.Vectors.report.csv

custom_clean
make CC=clang CXX=clang++ TCFLAGS=-usePericlesTripCount TEST=TripCountProfiler report.csv
mv report.TripCountProfiler.csv TripCountProfiler.Pericles.report.csv

custom_clean
make CC=clang CXX=clang++ TCFLAGS=-useHybridTripCount TEST=TripCountProfiler report.csv
mv report.TripCountProfiler.csv TripCountProfiler.Hybrid.report.csv
