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
	CA[algo, proc] = MIN(cost);
	UA[algo, proc] = MIN(usrt);
	SA[algo, proc] = MIN(syst);

	PHYOPA[algo, proc] = MIN(phyopnodes);
	PHYEQA[algo, proc] = MIN(phyeqnodes);
	LOGOPA[algo, proc] = MIN(logopnodes);
	EQUIVA[algo, proc] = MIN(equivnodes);

	LDAGUA[algo, proc] = MIN(ldagusrt);
	LDAGSA[algo, proc] = MIN(ldagsyst);

	PDAGUA[algo, proc] = MIN(pdagusrt);
	PDAGSA[algo, proc] = MIN(pdagsyst);

	SHARCOUNTA[algo, proc] = MIN(sharcount);
	PROPCOUNTA[algo, proc] = MIN(propcount);
	NUMPROPSA[algo, proc] = MIN(numprops);

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
	}
	printf "\n";

	printf "%-30s", "USRT:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", UA[x, y];
	}
	printf "\n";

	printf "%-30s", "SYST:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", SA[x, y];
	}
	printf "\n";

	printf "%-30s", "PHYDAGUSR:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", PDAGUA[x, y];
	}
	printf "\n";

	printf "%-30s", "PHYDAGSYS:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", PDAGSA[x, y];
	}
	printf "\n";

	printf "%-30s", "LOGDAGUSR:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", LDAGUA[x, y];
	}
	printf "\n";

	printf "%-30s", "LOGDAGSYS:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", LDAGSA[x, y];
	}
	printf "\n";

	printf "%-30s", "PHYOPNODECOUNT:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", PHYOPA[x, y];
	}
	printf "\n";

	printf "%-30s", "PHYEQNODECOUNT:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", PHYEQA[x, y];
	}
	printf "\n";

	printf "%-30s", "LOGOPNODECOUNT:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", LOGOPA[x, y];
	}
	printf "\n";

	printf "%-30s", "EQUIVNODECOUNT:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", EQUIVA[x, y];
	}
	printf "\n";

	printf "%-30s", "NUMPROPS:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", NUMPROPSA[x, y];
	}
	printf "\n";

	printf "%-30s", "PROPCOUNT:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", PROPCOUNTA[x, y];
	}
	printf "\n";

	printf "%-30s", "SHARCOUNT:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", SHARCOUNTA[x, y];
	}
	printf "\n";

	printf "\n";
    }
}

function MIN(x) { return x[0] < x[1] ? x[0] :x[1];}
