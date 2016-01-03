create table tempdb.prasan.temp15( l_suppkey int, l_quantity decimal(15,2))

insert into tempdb.prasan.temp15
select l_suppkey, sum(l_quantity)
from lineitem
group by l_suppkey

select * from tempdb.prasan.temp15

select  sum(l_quantity)
from tempdb.prasan.temp15

drop table tempdb.prasan.temp15