awk '{numLines++; total+=$3} END {printf "TripCount Accuracy	%f\n", total/numLines;}' $1
