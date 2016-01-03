set boxwidth 0.15
set terminal postscript portrait dashed 20
# set terminal fig 
set xtics ("      BQ1" 1, "      BQ2" 2, "      BQ3" 3, "      BQ4" 4, "      BQ5" 5, "" 6)
set xrange [0.8 to 6]

# cost
set output "PS/tpcd-comp-cost.ps"
set ylabel "Cost"
set xlabel "Queries"
set title "Cost of Plans"
plot "DATA/Volcano-cost.dat" title "Volcano" with boxes, "DATA/Volcano-SH-cost.dat" title "Volcano-SH" with boxes, "DATA/Volcano-RU-cost.dat" title "Volcano-RU" with boxes, "DATA/Greedy-cost.dat" title "Greedy" with boxes

# time
set logscale y
set output "PS/tpcd-comp-time.ps"
set ylabel "Time (seconds)"
set xlabel "Queries"
set title "Optimization Time"
plot "DATA/Volcano-time.dat" title "Volcano" with boxes, "DATA/Volcano-SH-time.dat" title "Volcano-SH" with boxes, "DATA/Volcano-RU-time.dat" title "Volcano-RU" with boxes, "DATA/Greedy-time.dat" title "Greedy" with boxes
set nologscale y

# mem
set logscale y
set output "PS/tpcd-comp-mem.ps"
set ylabel "Space (Bytes)"
set xlabel "Queries"
set title "Space Utilization"
plot "DATA/Volcano-mem.dat" title "Volcano" with boxes, "DATA/Volcano-SH-mem.dat" title "Volcano-SH" with boxes, "DATA/Volcano-RU-mem.dat" title "Volcano-RU" with boxes, "DATA/Greedy-mem.dat" title "Greedy" with boxes
set nologscale y

