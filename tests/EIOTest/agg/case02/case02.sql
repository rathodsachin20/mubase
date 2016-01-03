select r1c1, r1.c3, MIN(r1c2) from R1 group by r1c1, r1c3 order by r1c1, r1c3;
