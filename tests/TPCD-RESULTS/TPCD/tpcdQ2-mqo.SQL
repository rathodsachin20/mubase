insert into tempdb.prasan.shared
SELECT S_ACCTBAL, S_NAME, N_REGIONKEY, N_NAME, 
    S_ADDRESS, S_PHONE, S_COMMENT, PS_PARTKEY, PS_SUPPLYCOST
FROM prasan.SUPPLIER S, prasan.PARTSUPP PS, prasan.NATION
WHERE
 S_SUPPKEY = PS_SUPPKEY AND
 S_NATIONKEY = N_NATIONKEY

create index sharindex on tempdb.prasan.shared(N_REGIONKEY)

SELECT S_ACCTBAL, S_NAME, N_NAME, P_PARTKEY, P_MFGR,
    S_ADDRESS, S_PHONE, S_COMMENT
FROM prasan.PART P, prasan.SUPPLIER S, prasan.PARTSUPP PS, prasan.NATION, prasan.REGION
WHERE
 P_PARTKEY = PS_PARTKEY AND
 S_SUPPKEY = PS_SUPPKEY AND
 P_SIZE = 10 AND
 S_NATIONKEY = N_NATIONKEY AND
 N_REGIONKEY = R_REGIONKEY AND
 R_NAME = 'AMERICA' AND
 PS_SUPPLYCOST =
        (SELECT MIN(T.PS_SUPPLYCOST)
         FROM tempdb.prasan.shared T, prasan.REGION R1
         WHERE
          P.P_PARTKEY = T.PS_PARTKEY AND
          T.N_REGIONKEY = R1.R_REGIONKEY AND
          R1.R_NAME = 'AMERICA'
          )                                                            

drop table tempdb.prasan.shared
