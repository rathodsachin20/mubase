select empid from emp where sal > 40 AND depid IN (select depid from dep where dep.head=10 AND dep.depid=emp.depid)
