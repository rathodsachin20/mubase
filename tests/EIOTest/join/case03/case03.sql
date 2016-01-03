select MIN(r1c1), r2c1, r3c1 from r1, r2, r3 where r1c1=r2c1 and r2c1=r3c1 and r1c2=r2c2 group by r2c1, r3c1;
