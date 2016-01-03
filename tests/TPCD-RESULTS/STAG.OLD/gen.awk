BEGIN {
    for( i = 1 ; i <= 14 ; i++ ) {
	printf "SELECT PSP%d_SELKEY\n", i;
	printf "FROM PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d, PARTSUBPART%d\n", i, i+1, i+2, i+3, i+4;
	printf "WHERE PSP%d_SELKEY < %d\n", i, (i+5)*10; 
	printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i, i+1;
	printf "AND PSP%d_SELKEY < %d\n", i+1, (i+5)*10; 
	printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+1, i+2;
	printf "AND PSP%d_SELKEY < %d\n", i+2, (i+5)*10; 
	printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+2, i+3;
	printf "AND PSP%d_SELKEY < %d\n", i+3, (i+5)*10; 
	printf "AND PSP%d_SUBPARTKEY = PSP%d_PARTKEY\n", i+3, i+4;
	printf "AND PSP%d_SELKEY < %d;\n", i+4, (i+5)*10; 
	print;
    }
}
