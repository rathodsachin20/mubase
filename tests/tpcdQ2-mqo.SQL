create index on prasan.PARTSUPP(PS_SUPPKEY);

create table tempdb.prasan.shared(S_ACCTBAL decimal(15,2),
	S_NAME varchar(25), N_REGIONKEY int, N_NAME varchar(25),
        P_PARTKEY int, P_MFGR char(25), 
        S_ADDRESS varchar(40), S_PHONE char(15),
        S_COMMENT varchar(101), PS_SUPPLYCOST decimal(15,2))

insert into tempdb.prasan.shared
SELECT S_ACCTBAL, S_NAME, N_REGIONKEY, N_NAME, 
    S_ADDRESS, S_PHONE, S_COMMENT, PS_SUPPLYCOST
FROM prasan.SUPPLIER S, prasan.PARTSUPP PS, prasan.NATION,
WHERE
 S_SUPPKEY = PS_SUPPKEY AND
 S_NATIONKEY = N_NATIONKEY;

create index on tempdb.prasan.shared(N_REGIONKEY);

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
         FROM tempdb.prasan.shar T, prasan.REGION R1
         WHERE
          P.P_PARTKEY = T.PS_PARTKEY AND
          T.N_REGIONKEY = R1.R_REGIONKEY AND
          R1.R_NAME = 'AMERICA'
          )                                                            
