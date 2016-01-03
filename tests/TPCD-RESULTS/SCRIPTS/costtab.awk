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

/^TOTPHYOPNODES/ {
    totphyopnodes = $2;
}

/^TOTPHYEQNODES/ {
    totphyeqnodes = $2;
}

/^TOTLOGOPNODES/ {
    totlogopnodes = $2;
}

/^TOTEQUIVNODES/ {
    totequivnodes = $2;
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

    TOTPHYOPA[algo, proc] = totphyopnodes;
    TOTPHYEQA[algo, proc] = totphyeqnodes;
    TOTLOGOPA[algo, proc] = totlogopnodes;
    TOTEQUIVA[algo, proc] = totequivnodes;

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

    cost = -1;
    usrt = -1;
    syst = -1;

    phyopnodes = -1;
    phyeqnodes = -1;
    logopnodes = -1;
    equivnodes = -1;

    totphyopnodes = -1;
    totphyeqnodes = -1;
    totlogopnodes = -1;
    totequivnodes = -1;

    ldagusrt = -1;
    ldagsyst = -1;
    pdagusrt = -1;
    pdagsyst = -1;

    sharcount = -1;
    propcount = -1;
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

	printf "\n";
    }
}

