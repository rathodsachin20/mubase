select R1.r1c1 from R1 where R1.r1c2 = 100 and R1.r1c3 IN (select R2.r2c1 from R2 where R2.r2c2 = 200 and R2.r2c3=R1.r1c4)
