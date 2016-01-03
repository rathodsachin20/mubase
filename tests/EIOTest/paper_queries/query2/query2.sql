SELECT l_partkey, l_suppkey, l2_partkey FROM lineitem, lineitem2 WHERE l_partkey=l2_partkey AND l_suppkey=l2_suppkey AND l_orderkey=l2_orderkey ORDER BY l_partkey, l_suppkey;
