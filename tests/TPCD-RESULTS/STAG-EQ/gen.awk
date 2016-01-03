BEGIN {
    for( j = 1 ; j <= 10 ; j+=2 ) {
	k = 2*j;
	for( i = 1 ; i <= k ; i++ ) {
	    printf "SELECT PSP%d_SELKEY\n", i > "stag-"k;
	    printf "FROM PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d\n", i, i+1, i+2, i+3, i+4 > "stag-"k;
	    printf "WHERE PSP%d_SELKEY = %d\n", i, i > "stag-"k; 
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i, i+1 > "stag-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+1, i+2 > "stag-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+2, i+3 > "stag-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY;\n", i+3, i+4 > "stag-"k;
	    print > "stag-"k;
	}
	for( i = 1 ; i <= k ; i++ ) {
	    printf "SELECT PSP%d_SELKEY\n", i > "stag-"k;
	    printf "FROM PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d\n", i, i+1, i+2, i+3, i+4 > "stag-"k;
	    printf "WHERE PSP%d_SELKEY = %d\n", i, i+1 > "stag-"k; 
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i, i+1 > "stag-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+1, i+2 > "stag-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+2, i+3 > "stag-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY;\n", i+3, i+4 > "stag-"k;
	    print > "stag-"k;
	}

	for( i = k ; i >= 1 ; i-- ) {
	    printf "SELECT PSP%d_SELKEY\n", i > "stagr-"k;
	    printf "FROM PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d\n", i, i+1, i+2, i+3, i+4 > "stagr-"k;
	    printf "WHERE PSP%d_SELKEY = %d\n", i, i > "stagr-"k; 
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i, i+1 > "stagr-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+1, i+2 > "stagr-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+2, i+3 > "stagr-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY;\n", i+3, i+4 > "stagr-"k;
	    print > "stagr-"k;
	}
	for( i = k ; i >= 1 ; i-- ) {
	    printf "SELECT PSP%d_SELKEY\n", i > "stagr-"k;
	    printf "FROM PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d\n", i, i+1, i+2, i+3, i+4 > "stagr-"k;
	    printf "WHERE PSP%d_SELKEY = %d\n", i, i+1 > "stagr-"k; 
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i, i+1 > "stagr-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+1, i+2 > "stagr-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+2, i+3 > "stagr-"k;
	    printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY;\n", i+3, i+4 > "stagr-"k;
	    print > "stagr-"k;
	}
    }
}
