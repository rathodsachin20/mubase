BEGIN {
    for( j = 1 ; j <= 10 ; j++ ) {
	k = 2*j-1;
	for( i = 1 ; i <= k ; i++ ) {
	    printf "SELECT PSP%d_SELKEY\n", i > "stag-"k;
	    printf "FROM PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d\n", i, i+1, i+2, i+3, i+4 > "stag-"k;
	    printf "WHERE PSP%d_SELKEY < %d\n", i, (i+5)*10 > "stag-"k; 
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i, i+1 > "stag-"k;
	    printf "AND PSP%d_SELKEY < %d\n", i+1, (i+5)*10 > "stag-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+1, i+2 > "stag-"k;
	    printf "AND PSP%d_SELKEY < %d\n", i+2, (i+5)*10 > "stag-"k; 
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+2, i+3 > "stag-"k;
	    printf "AND PSP%d_SELKEY < %d\n", i+3, (i+5)*10 > "stag-"k; 
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+3, i+4 > "stag-"k;
	    printf "AND PSP%d_SELKEY < %d;\n", i+4, (i+5)*10 > "stag-"k; 
	    print > "stag-"k;
	}
    }
}
