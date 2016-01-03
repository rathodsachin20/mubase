-- Sccsid:     @(#)dss.ddl	2.1.8.1

use tpch1;

CREATE TABLE REGION  ( R_REGIONKEY  INTEGER NOT NULL PRIMARY KEY CLUSTERED,
                            R_NAME       CHAR(25) NOT NULL,
                            R_COMMENT    VARCHAR(152));

CREATE TABLE NATION  ( N_NATIONKEY  INTEGER NOT NULL PRIMARY KEY ClUSTERED,
                            N_NAME       CHAR(25) NOT NULL,
                            N_REGIONKEY  INTEGER NOT NULL REFERENCES REGION(R_REGIONKEY),
                            N_COMMENT    VARCHAR(152));

CREATE TABLE PART  ( P_PARTKEY     INTEGER NOT NULL PRIMARY KEY CLUSTERED,
                          P_NAME        VARCHAR(55) NOT NULL,
                          P_MFGR        CHAR(25) NOT NULL,
                          P_BRAND       CHAR(10) NOT NULL,
                          P_TYPE        VARCHAR(25) NOT NULL,
                          P_SIZE        INTEGER NOT NULL,
                          P_CONTAINER   CHAR(10) NOT NULL,
                          P_RETAILPRICE DECIMAL(15,2) NOT NULL,
                          P_COMMENT     VARCHAR(23) NOT NULL );

CREATE TABLE SUPPLIER ( S_SUPPKEY     INTEGER NOT NULL PRIMARY KEY CLUSTERED,
                             S_NAME        CHAR(25) NOT NULL,
                             S_ADDRESS     VARCHAR(40) NOT NULL,
                             S_NATIONKEY   INTEGER NOT NULL REFERENCES NATION(N_NATIONKEY),
                             S_PHONE       CHAR(15) NOT NULL,
                             S_ACCTBAL     DECIMAL(15,2) NOT NULL,
                             S_COMMENT     VARCHAR(101) NOT NULL);

CREATE TABLE PARTSUPP ( PS_PARTKEY     INTEGER NOT NULL REFERENCES PART(P_PARTKEY),
                             PS_SUPPKEY     INTEGER NOT NULL REFERENCES SUPPLIER(S_SUPPKEY),
                             PS_AVAILQTY    INTEGER NOT NULL,
                             PS_SUPPLYCOST  DECIMAL(15,2)  NOT NULL,
                             PS_COMMENT     VARCHAR(199) NOT NULL 
		         CONSTRAINT PK_PARTSUPP PRIMARY KEY(PS_PARTKEY,PS_SUPPKEY));

CREATE TABLE CUSTOMER ( C_CUSTKEY     INTEGER NOT NULL PRIMARY KEY CLUSTERED,
                             C_NAME        VARCHAR(25) NOT NULL,
                             C_ADDRESS     VARCHAR(40) NOT NULL,
                             C_NATIONKEY   INTEGER NOT NULL REFERENCES NATION(N_NATIONKEY),
                             C_PHONE       CHAR(15) NOT NULL,
                             C_ACCTBAL     DECIMAL(15,2)   NOT NULL,
                             C_MKTSEGMENT  CHAR(10) NOT NULL,
                             C_COMMENT     VARCHAR(117) NOT NULL);

CREATE TABLE ORDERS  ( O_ORDERKEY       INTEGER NOT NULL PRIMARY KEY CLUSTERED,
                           O_CUSTKEY        INTEGER NOT NULL REFERENCES CUSTOMER(C_CUSTKEY),
                           O_ORDERSTATUS    CHAR(1) NOT NULL,
                           O_TOTALPRICE     DECIMAL(15,2) NOT NULL,
                           O_ORDERDATE      DATETIME NOT NULL,
                           O_ORDERPRIORITY  CHAR(15) NOT NULL,  
                           O_CLERK          CHAR(15) NOT NULL, 
                           O_SHIPPRIORITY   INTEGER NOT NULL,
                           O_COMMENT        VARCHAR(79) NOT NULL);

CREATE TABLE LINEITEM ( L_ORDERKEY    INTEGER NOT NULL REFERENCES ORDERS(O_ORDERKEY),
                             L_PARTKEY     INTEGER NOT NULL,
                             L_SUPPKEY     INTEGER NOT NULL,
                             L_LINENUMBER  INTEGER NOT NULL,
                             L_QUANTITY    DECIMAL(15,2) NOT NULL,
                             L_EXTENDEDPRICE  DECIMAL(15,2) NOT NULL,
                             L_DISCOUNT    DECIMAL(15,2) NOT NULL,
                             L_TAX         DECIMAL(15,2) NOT NULL,
                             L_RETURNFLAG  CHAR(1) NOT NULL,
                             L_LINESTATUS  CHAR(1) NOT NULL,
                             L_SHIPDATE    DATETIME NOT NULL,
                             L_COMMITDATE  DATETIME NOT NULL,
                             L_RECEIPTDATE DATETIME NOT NULL,
                             L_SHIPINSTRUCT CHAR(25) NOT NULL,
                             L_SHIPMODE     CHAR(10) NOT NULL,
                             L_COMMENT      VARCHAR(44) NOT NULL
			   CONSTRAINT FK_LINEITEM FOREIGN KEY(L_PARTKEY,L_SUPPKEY) REFERENCES PARTSUPP);



select count(*) from region;
select count(*) from nation;
select count(*) from part;
select count(*) from supplier;
select count(*) from partsupp;
select count(*) from customer;
select count(*) from orders;
select count(*) from lineitem;

select * from lineitem l inner merge join partsupp ps on (l.l_partkey=ps.ps_partkey and l.l_suppkey=ps.ps_suppkey) inner merge join supplier s on  l.l_suppkey=s.s_suppkey order by l.l_suppkey, l.l_partkey;
select * from lineitem l inner merge join supplier s  on (l.l_suppkey=s.s_suppkey) inner merge join partsupp ps on (l.l_partkey=ps.ps_partkey and l.l_suppkey=ps.ps_suppkey) order by l.l_suppkey, l.l_partkey;
select l1.l_suppkey, l1.l_partkey, ps.ps_availqty, l1.l_orderkey  from lineitem l1 inner merge join partsupp ps on (l1.l_partkey=ps.ps_partkey and l1.l_suppkey=ps.ps_suppkey) inner merge join lineitem l2 on  (l2.l_suppkey=l1.l_suppkey and l2.l_partkey=l1.l_partkey and l2.l_orderkey=l1.l_orderkey);

select l1.l_suppkey, l1.l_partkey, ps.ps_availqty, l1.l_orderkey from lineitem l1 inner merge join partsupp ps on (l1.l_partkey=ps.ps_partkey and l1.l_suppkey=ps.ps_suppkey) order by l1.l_suppkey, l1.l_partkey;

create index idx_li_supp_part on lineitem(l_suppkey) include (l_partkey, l_orderkey)


