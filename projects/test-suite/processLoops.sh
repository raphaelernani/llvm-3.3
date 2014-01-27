awk '{
        if ($2 == 0) {
            if ($4 == 0) {
                totalT0G0+=$5;
                numLinesT0G0++;
            } else if ($4 == 1) {
                totalT0G1+=$5;
                numLinesT0G1++;
            } else if ($4 == 2) {
                totalT0G2+=$5;
                numLinesT0G2++;
            } else if ($4 == 3) {
                totalT0G3+=$5;
                numLinesT0G3++;
            } else if ($4 == 4) {
                totalT0G4+=$5;
                numLinesT0G4++;
            } else if ($4 == 5) {
                totalT0G5+=$5;
                numLinesT0G5++;
            } else if ($4 == 6) {                                                                      
                totalT0G6+=$5;
                numLinesT0G6++;    
            }    
        } else if($2 == 1) {
            if ($4 == 0) {
                totalT1G0+=$5;
                numLinesT1G0++;
            } else if ($4 == 1) {
                totalT1G1+=$5;
                numLinesT1G1++;
            } else if ($4 == 2) {
                totalT1G2+=$5;
                numLinesT1G2++;
            } else if ($4 == 3) {
                totalT1G3+=$5;
                numLinesT1G3++;
            } else if ($4 == 4) {
                totalT1G4+=$5;
                numLinesT1G4++;
            } else if ($4 == 5) {
                totalT1G5+=$5;
                numLinesT1G5++;
            } else if ($4 == 6) {                                                                     
                totalT1G6+=$5;
                numLinesT1G6++;
            }   
        } else if($2 == 2) {
            if ($4 == 0) {
                totalT2G0+=$5;
                numLinesT2G0++;
            } else if ($4 == 1) {
                totalT2G1+=$5;
                numLinesT2G1++;
            } else if ($4 == 2) {
                totalT2G2+=$5;
                numLinesT2G2++;
            } else if ($4 == 3) {
                totalT2G3+=$5;
                numLinesT2G3++;
            } else if ($4 == 4) {
                totalT2G4+=$5;
                numLinesT2G4++;
            } else if ($4 == 5) {
                totalT2G5+=$5;
                numLinesT2G5++;
            } else if ($4 == 6) {                                                                        
                totalT2G6+=$5;
                numLinesT2G6++; 
            }
        }
        numLines++; 
        total+=$5
     } END {
        if (numLinesT0G0 > 0) {
            printf "NumLoops (Interval Loops, Group 0)	%d\n", numLinesT0G0;
            printf "NumInstances (Interval Loops, Group 0)	%d\n", totalT0G0;            
        } 
        if (numLinesT0G1 > 0) {
            printf "NumLoops (Interval Loops, Group 1)	%d\n", numLinesT0G1;
            printf "NumInstances (Interval Loops, Group 1)	%d\n", totalT0G1;  
        } 
         if (numLinesT0G2 > 0) {
            printf "NumLoops (Interval Loops, Group 2)	%d\n", numLinesT0G2;
            printf "NumInstances (Interval Loops, Group 2)	%d\n", totalT0G2;  
        } 
        if (numLinesT0G3 > 0) {
            printf "NumLoops (Interval Loops, Group 3)	%d\n", numLinesT0G3;
            printf "NumInstances (Interval Loops, Group 3)	%d\n", totalT0G3;  
        } 
        if (numLinesT0G4 > 0) {
            printf "NumLoops (Interval Loops, Group 4)	%d\n", numLinesT0G4;
            printf "NumInstances (Interval Loops, Group 4)	%d\n", totalT0G4;  
        } 
        if (numLinesT0G5 > 0) {
            printf "NumLoops (Interval Loops, Group 5)	%d\n", numLinesT0G5;
            printf "NumInstances (Interval Loops, Group 5)	%d\n", totalT0G5;  
        } 
        if (numLinesT0G6 > 0) {
            printf "NumLoops (Interval Loops, Group 6)	%d\n", numLinesT0G6;
            printf "NumInstances (Interval Loops, Group 6)	%d\n", totalT0G6;  
        } 
        if (numLinesT1G0 > 0) {
            printf "NumLoops (Equality Loops, Group 0)	%d\n", numLinesT1G0;
            printf "NumInstances (Equality Loops, Group 0)	%d\n", totalT1G0;            
        } 
        if (numLinesT1G1 > 0) {
            printf "NumLoops (Equality Loops, Group 1)	%d\n", numLinesT1G1;
            printf "NumInstances (Equality Loops, Group 1)	%d\n", totalT1G1;  
        } 
         if (numLinesT1G2 > 0) {
            printf "NumLoops (Equality Loops, Group 2)	%d\n", numLinesT1G2;
            printf "NumInstances (Equality Loops, Group 2)	%d\n", totalT1G2;  
        } 
        if (numLinesT1G3 > 0) {
            printf "NumLoops (Equality Loops, Group 3)	%d\n", numLinesT1G3;
            printf "NumInstances (Equality Loops, Group 3)	%d\n", totalT1G3;  
        } 
        if (numLinesT1G4 > 0) {
            printf "NumLoops (Equality Loops, Group 4)	%d\n", numLinesT1G4;
            printf "NumInstances (Equality Loops, Group 4)	%d\n", totalT1G4;  
        } 
        if (numLinesT1G5 > 0) {
            printf "NumLoops (Equality Loops, Group 5)	%d\n", numLinesT1G5;
            printf "NumInstances (Equality Loops, Group 5)	%d\n", totalT1G5;  
        } 
        if (numLinesT1G6 > 0) {
            printf "NumLoops (Equality Loops, Group 6)	%d\n", numLinesT1G6;
            printf "NumInstances (Equality Loops, Group 6)	%d\n", totalT1G6;  
        } 
        if (numLinesT2G0 > 0) {
            printf "NumLoops (Equality Loops, Group 0)	%d\n", numLinesT2G0;
            printf "NumInstances (Equality Loops, Group 0)	%d\n", totalT2G0;            
        } 
        if (numLinesT2G1 > 0) {
            printf "NumLoops (Equality Loops, Group 1)	%d\n", numLinesT2G1;
            printf "NumInstances (Equality Loops, Group 1)	%d\n", totalT2G1;  
        } 
         if (numLinesT2G2 > 0) {
            printf "NumLoops (Equality Loops, Group 2)	%d\n", numLinesT2G2;
            printf "NumInstances (Equality Loops, Group 2)	%d\n", totalT2G2;  
        } 
        if (numLinesT2G3 > 0) {
            printf "NumLoops (Equality Loops, Group 3)	%d\n", numLinesT2G3;
            printf "NumInstances (Equality Loops, Group 3)	%d\n", totalT2G3;  
        } 
        if (numLinesT2G4 > 0) {
            printf "NumLoops (Equality Loops, Group 4)	%d\n", numLinesT2G4;
            printf "NumInstances (Equality Loops, Group 4)	%d\n", totalT2G4;  
        } 
        if (numLinesT2G5 > 0) {
            printf "NumLoops (Equality Loops, Group 5)	%d\n", numLinesT2G5;
            printf "NumInstances (Equality Loops, Group 5)	%d\n", totalT2G5;  
        } 
        if (numLinesT2G6 > 0) {
            printf "NumLoops (Equality Loops, Group 6)	%d\n", numLinesT2G6;
            printf "NumInstances (Equality Loops, Group 6)	%d\n", totalT2G6;  
        }
        
        if (numLines > 0) {
            printf "NumLoops (All Loops)	%d\n", numLines;
            printf "NumInstances (All Loops)	%d\n", total;            
        }              
     }' $1
