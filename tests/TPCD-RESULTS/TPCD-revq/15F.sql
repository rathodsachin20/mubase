{ SCCS: @(#)15F.sql	9.1.1.3     9/11/95  15:16:07 }
CREATE VIEW REVENUE:S (SUPPLIER_NO, TOTAL_REVENUE) AS
    SELECT L_SUPPKEY, SUM(L_EXTENDEDPRICE * (1 - L_DISCOUNT))
    FROM LINEITEM
    WHERE
	L_SHIPDATE >= DATE ':1' AND
	L_SHIPDATE < DATE ':1' + 3 MONTH
    GROUP BY L_SUPPKEY;

:o
SELECT S_SUPPKEY, S_NAME, S_ADDRESS, S_PHONE, TOTAL_REVENUE
FROM SUPPLIER, REVENUE:S
WHERE S_SUPPKEY = SUPPLIER_NO AND
	TOTAL_REVENUE = 
		(SELECT MAX(TOTAL_REVENUE) FROM REVENUE:S)
ORDER BY S_SUPPKEY;
:n -1

DROP VIEW REVENUE:S;
