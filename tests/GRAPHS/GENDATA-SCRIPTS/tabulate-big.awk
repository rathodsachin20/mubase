BEGIN {
    FS="[ \t]";
    ac = 0;
    pc = 0;
}

/^PROC/ {
    proc = $2;
}

/^ALGO/ {
    algo = $2;
}

/^COST/ {
    cost = $2;
}

/^USRT/ {
    usrt = $2;
}

/^SYST/ {
    syst = $2;
}

/^PHYOPNODES/ {
    phyopnodes = $2;
}

/^PHYEQNODES/ {
    phyeqnodes = $2;
}

/^LOGOPNODES/ {
    logopnodes = $2;
}

/^EQUIVNODES/ {
    equivnodes = $2;
}

/^LDAGUSR/ {
    ldagusrt = $2;
}

/^LDAGSYS/ {
    ldagsyst = $2;
}

/^PDAGUSR/ {
    pdagusrt = $2;
}

/^PDAGSYS/ {
    pdagsyst = $2;
}

/^PROPCOUNT/ {
    propcount = $2;
}

/^SHARCOUNT/ {
    sharcount = $2;
}

/^END/ {
    CA[algo, proc] = cost;
    UA[algo, proc] = usrt;
    SA[algo, proc] = syst;

    PHYOPA[algo, proc] = phyopnodes;
    PHYEQA[algo, proc] = phyeqnodes;
    LOGOPA[algo, proc] = logopnodes;
    EQUIVA[algo, proc] = equivnodes;

    LDAGUA[algo, proc] = ldagusrt;
    LDAGSA[algo, proc] = ldagsyst;

    PDAGUA[algo, proc] = pdagusrt;
    PDAGSA[algo, proc] = pdagsyst;

    SHARCOUNTA[algo, proc] = sharcount;
    PROPCOUNTA[algo, proc] = propcount;

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

    cost = 0;
    usrt = 0;
    syst = 0;

    phyopnodes = 0;
    phyeqnodes = 0;
    logopnodes = 0;
    equivnodes = 0;

    totphyopnodes = 0;
    totphyeqnodes = 0;
    totlogopnodes = 0;
    totequivnodes = 0;

    ldagusrt = 0;
    ldagsyst = 0;
    pdagusrt = 0;
    pdagsyst = 0;

    sharcount = 0;
    propcount = 0;
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
	    printf "%f %-16.2f\n", 1.05+j+i/9.0, CA[x, y] > "DATA/"x"-cost.dat";
	}
	printf "\n";

	printf "%-30s", "TIME:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", UA[x, y] + SA[x, y];
	    printf "%f %-16d\n", 1.05+j+i/9.0, (UA[x, y] + SA[x, y])/1000.0 > "DATA/"x"-time.dat";
	}
	printf "\n";

	printf "%-30s", "PHYDAGSIZE:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", PHYOPA[x, y] + PHYEQA[x, y];
	    printf "%f %-16d\n", 1.05+j+i/9.0, PHYOPA[x, y] + PHYEQA[x, y] > "DATA/"x"-phydagsize.dat";
	}
	printf "\n";

	printf "%-30s", "LOGDAGSIZE:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", LOGOPA[x, y] + EQUIVA[x, y];
	    printf "%f %-16d\n", 1.05+j+i/9.0, LOGOPA[x, y] + EQUIVA[x, y] > "DATA/"x"-logdagsize.dat";
	}
	printf "\n";

	printf "%-30s", "MEM:" y;
	for( i = 0 ; i < ac ; i++ ) {
	    x = AL[i];
	    printf "%-16d", LOGOPA[x, y]*36 + EQUIVA[x, y]*76 + PHYOPA[x, y]*32 + PHYEQA[x, y]*128;
	    printf "%f %-16d\n", 1.05+j+i/9.0, LOGOPA[x, y]*36 + EQUIVA[x, y]*76 + PHYOPA[x, y]*32 + PHYEQA[x, y]*128 > "DATA/"x"-mem.dat";
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

	printf "\n";
    }
}

