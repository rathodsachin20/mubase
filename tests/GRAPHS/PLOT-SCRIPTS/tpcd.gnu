set boxwidth 0.15
set terminal postscript portrait dashed 20
# set terminal fig 
set xtics ("          Q2" 1, "          Q2-D" 2, "          Q11" 3, "          Q15" 4, "" 5)
set xrange [0.8 to 5]

# cost
set output "PS/tpcd-cost.ps"
set ylabel "Cost"
set title "Cost of Plans"
plot "DATA/Volcano-cost.dat" title "Volcano" with boxes, "DATA/Volcano-SH-cost.dat" title "Volcano-SH" with boxes, "DATA/Volcano-RU-cost.dat" title "Volcano-RU" with boxes, "DATA/Greedy-cost.dat" title "Greedy" with boxes

# time
set logscale y
set output "PS/tpcd-time.ps"
set ylabel "Time (seconds)"
set title "Optimization Time"
plot "DATA/Volcano-time.dat" title "Volcano" with boxes, "DATA/Volcano-SH-time.dat" title "Volcano-SH" with boxes, "DATA/Volcano-RU-time.dat" title "Volcano-RU" with boxes, "DATA/Greedy-time.dat" title "Greedy" with boxes
set nologscale y

# MSSQL Numbers
set output "PS/mssql-cost.ps"
set ylabel "Time (seconds)"
set title "MS-SQL Execution Time"
plot "DATA/MSSQL-cost.dat" title "No-MQO" with boxes, "DATA/MSSQL-mqo-cost.dat" title "MQO" with boxes

# mem
set logscale y
set output "PS/tpcd-mem.ps"
set ylabel "Space (Bytes)"
set title "Space Utilization"
plot "DATA/Volcano-mem.dat" title "Volcano" with boxes, "DATA/Volcano-SH-mem.dat" title "Volcano-SH" with boxes, "DATA/Volcano-RU-mem.dat" title "Volcano-RU" with boxes, "DATA/Greedy-mem.dat" title "Greedy" with boxes
set nologscale y
