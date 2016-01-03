BEGIN {
    for( k = 0 ; k < 32 ; k++ ) {
	for( i = 0 ; i < k ; i++ ) {
	    A[0] = (i/16)%2;
	    A[1] = 2 + (i/8)%2;
	    A[2] = 4 + (i/4)%2;
	    A[3] = 6 + (i/2)%2;
	    A[4] = 8 + i%2;
	    printf "SELECT MAX(PSP%d_SELKEY)\n", A[0] > "stag-"k;
	    printf "FROM PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d\n", A[0], A[1], A[2], A[3], A[4] > "stag-"k;
	    printf "WHERE PSP%d_SELKEY < %d\n", A[0], (i+5)*10 > "stag-"k; 
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", A[0], A[1] > "stag-"k;
	    printf "AND PSP%d_SELKEY < %d\n", A[1], (i+5)*10 > "stag-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", A[1], A[2] > "stag-"k;
	    printf "AND PSP%d_SELKEY < %d\n", A[2], (i+5)*10 > "stag-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", A[2], A[3] > "stag-"k;
	    printf "AND PSP%d_SELKEY < %d\n", A[3], (i+5)*10 > "stag-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", A[3], A[4] > "stag-"k;
	    printf "AND PSP%d_SELKEY < %d\n", A[4], (i+5)*10 > "stag-"k;
	    printf "GROUP BY PSP%d_SELKEY;\n\n", A[0] > "stag-"k;
	}
    }
}
