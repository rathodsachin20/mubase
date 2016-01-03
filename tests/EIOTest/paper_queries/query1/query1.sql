select l_suppkey, l_partkey, ps_supplycost FROM lineitem, partsupp WHERE l_partkey=ps_partkey AND l_suppkey=ps_suppkey;
