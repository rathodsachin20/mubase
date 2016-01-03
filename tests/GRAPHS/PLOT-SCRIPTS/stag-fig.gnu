set boxwidth 0.15
# set terminal postscript portrait dashed 20
set terminal fig 
set xlabel "Queries"
set xrange [0.8 to 6]
set xtics ("      CQ1" 1, "      CQ2" 2, "      CQ3" 3, "      CQ4" 4, "      CQ5" 5, "" 6)

# cost
set output "PS/stag-cost.fig"
set ylabel "Cost"
set title "Cost of Plans"
plot "DATA/Volcano-cost.dat" title "Volcano" with boxes, "DATA/Volcano-SH-cost.dat" title "Volcano-SH" with boxes, "DATA/Volcano-RU-cost.dat" title "Volcano-RU" with boxes, "DATA/Greedy-cost.dat" title "Greedy" with boxes

# time
set logscale y
set output "PS/stag-time.fig"
set ylabel "Time (seconds)"
set title "Optimization Time"
plot "DATA/Volcano-time.dat" title "Volcano" with boxes, "DATA/Volcano-SH-time.dat" title "Volcano-SH" with boxes, "DATA/Volcano-RU-time.dat" title "Volcano-RU" with boxes, "DATA/Greedy-time.dat" title "Greedy" with boxes
set nologscale y

# mem
set output "PS/stag-mem.fig"
set logscale y
set ylabel "Space (Bytes)"
set title "Space Utilization"
plot "DATA/Volcano-mem.dat" title "Volcano" with boxes, "DATA/Volcano-SH-mem.dat" title "Volcano-SH" with boxes, "DATA/Volcano-RU-mem.dat" title "Volcano-RU" with boxes, "DATA/Greedy-mem.dat" title "Greedy" with boxes
set nologscale y

# propcount
set output "PS/stag-propcount.fig"
set ylabel "Number of Eq. Nodes"
set title "Cost Propagation across Equivalence Nodes"
plot "DATA/Greedy-propcount.dat" title "Greedy" with boxes

# numprops
set output "PS/stag-numprops.fig"
set ylabel "Number of Recomputations"
set title "Number of Cost Recomputations"
plot "DATA/Greedy-numprops.dat" title "Greedy" with boxes

