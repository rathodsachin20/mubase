BEGIN {
    FS="[ \t]";
    ac = 0;
    pc = 0;
    k = 0;
}

/^PROC/ {
    proc = $2;
}

/^ALGO/ {
    algo = $2;
}

/^COST/ {
    cost[k] = $2;
}

/^USRT/ {
    usrt[k] = $2;
}

/^SYST/ {
    syst[k] = $2;
}

/^PHYOPNODES/ {
    phyopnodes[k] = $2;
}

/^PHYEQNODES/ {
    phyeqnodes[k] = $2;
}

/^LOGOPNODES/ {
    logopnodes[k] = $2;
}

/^EQUIVNODES/ {
    equivnodes[k] = $2;
}

/^LDAGUSR/ {
    ldagusrt[k] = $2;
}

/^LDAGSYS/ {
    ldagsyst[k] = $2;
}

/^PDAGUSR/ {
    pdagusrt[k] = $2;
}

/^PDAGSYS/ {
    pdagsyst[k] = $2;
}

/^PROPCOUNT/ {
    propcount[k] = $2;
}

/^NUMPROPS/ {
    numprops[k] = $2;
}

/^SHARCOUNT/ {
    sharcount[k] = $2;
}

/^END/ {
    if( k == 1 ) {
	CA[algo, proc] = AVG(cost);
	UA[algo, proc] = AVG(usrt);
	SA[algo, proc] = AVG(syst);

	PHYOPA[algo, proc] = AVG(phyopnodes);
	PHYEQA[algo, proc] = AVG(phyeqnodes);
	LOGOPA[algo, proc] = AVG(logopnodes);
	EQUIVA[algo, proc] = AVG(equivnodes);

	LDAGUA[algo, proc] = AVG(ldagusrt);
	LDAGSA[algo, proc] = AVG(ldagsyst);

	PDAGUA[algo, proc] = AVG(pdagusrt);
	PDAGSA[algo, proc] = AVG(pdagsyst);

	SHARCOUNTA[algo, proc] = AVG(sharcount);
	PROPCOUNTA[algo, proc] = AVG(propcount);
	NUMPROPSA[algo, proc] = AVG(numprops);

	if( !(algo in A) ) {
	    A[algo] = algo;
	    AL[ac] = algo;
	    ac++;
	}
	if( !(proc in P) ) {
	    P[proc] = proc;
	    PL[pc] = proc;
	    pc++;
	}

	for( i = 0 ; i < 2 ; i++ ) {
	    cost[i] = -1;
	    usrt[i] = -1;
	    syst[i] = -1;

	    phyopnodes[i] = -1;
	    phyeqnodes[i] = -1;
	    logopnodes[i] = -1;
	    equivnodes[i] = -1;

	    totphyopnodes[i] = -1;
	    totphyeqnodes[i] = -1;
	    totlogopnodes[i] = -1;
	    totequivnodes[i] = -1;

	    ldagusrt[i] = -1;
	    ldagsyst[i] = -1;
	    pdagusrt[i] = -1;
	    pdagsyst[i] = -1;

	    sharcount[i] = -1;
	    propcount[i] = -1;
	}
    }

    k = 1 - k;
}

END {
    printf "%-30s", "";
    for( i = 0 ; i < ac ; i++ )
	printf "%-16s", AL[i];
    printf "\n";
    printf "\n";

    for( j = 0 ; j < pc ; j++ ) {
	y = PL[j];
	printf "%-30s", "COST:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16.2f", CA[x, y];
	    printf "%f %-16.2f\n", 1.05+j+i/5.0, CA[x, y] > "DATA/"x"-cost.dat";
	}
	printf "\n";

	printf "%-30s", "TIME:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", UA[x, y] + SA[x, y];
	    printf "%f %-16.2f\n", 1.05+j+i/5.0, (UA[x, y] + SA[x, y])/1000.0 > "DATA/"x"-time.dat";
	}
	printf "\n";

	printf "%-30s", "PHYDAGSIZE:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", PHYOPA[x, y] + PHYEQA[x, y];
#printf "%f %-16d\n", 1.05+j+i/5.0, PHYOPA[x, y] + PHYEQA[x, y] # > "DATA/"x"-phydagsize.dat";
	}
	printf "\n";

	printf "%-30s", "LOGDAGSIZE:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", LOGOPA[x, y] + EQUIVA[x, y];
#printf "%f %-16d\n", 1.05+j+i/5.0, LOGOPA[x, y] + EQUIVA[x, y] # > "DATA/"x"-logdagsize.dat";
	}
	printf "\n";

	printf "%-30s", "MEM:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", LOGOPA[x, y]*36 + EQUIVA[x, y]*76 + PHYOPA[x, y]*32 + PHYEQA[x, y]*128;
	    printf "%f %-16d\n", 1.05+j+i/5.0, LOGOPA[x, y]*36 + EQUIVA[x, y]*76 + PHYOPA[x, y]*32 + PHYEQA[x, y]*128 > "DATA/"x"-mem.dat";
	}
	printf "\n";

	printf "%-30s", "SHARCOUNT:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", SHARCOUNTA[x, y];
	    printf "%f %-16d\n", 1.05+j, SHARCOUNTA[x, y] > "DATA/"x"-sharcount.dat";
	}
	printf "\n";

	printf "%-30s", "PROPCOUNT:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", PROPCOUNTA[x, y];
	    printf "%f %-16d\n", 1.05+j, PROPCOUNTA[x, y] > "DATA/"x"-propcount.dat";
	}
	printf "\n";

	printf "%-30s", "NUMPROPS:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", NUMPROPSA[x, y];
	    printf "%f %-16d\n", 1.05+j, NUMPROPSA[x, y] > "DATA/"x"-numprops.dat";
	}
	printf "\n";

	printf "\n";
    }
}

function AVG(x) { return (x[0]+x[1])/2.0; }
