set boxwidth 0.15
#set terminal postscript portrait dashed 20
set terminal fig 
set xlabel "Queries"
set xrange [0.8 to 11]
set xtics ("      HQ1" 1, "      HQ2" 2, "      HQ3" 3, "      HQ4" 4, "      HQ5" 5, "      HQ6" 6, "      HQ7" 7, "      HQ8" 8, "      HQ9" 9, "       HQ10" 10, "" 11)

# cost
set output "PS/hier-cost.fig"
set ylabel "Cost"
set title "Cost of Plans"
plot "DATA/Volcano-cost.dat" title "Volcano" with boxerrorbars, "DATA/Volcano-SH-cost.dat" title "Volcano-SH" with boxerrorbars, "DATA/Volcano-RU-cost.dat" title "Volcano-RU" with boxerrorbars, "DATA/Greedy-cost.dat" title "Greedy" with boxerrorbars

# time
set logscale y
set output "PS/hier-time.fig"
set ylabel "Time (seconds)"
set title "Optimization Time"
plot "DATA/Volcano-time.dat" title "Volcano" with boxes, "DATA/Volcano-SH-time.dat" title "Volcano-SH" with boxes, "DATA/Volcano-RU-time.dat" title "Volcano-RU" with boxes, "DATA/Greedy-time.dat" title "Greedy" with boxes
set nologscale y

# propcount
set output "PS/hier-propcount.fig"
set ylabel "Number of Eq. Nodes"
set title "Cost Propagation across Equivalence Nodes"
plot "DATA/Greedy-propcount.dat" title "Greedy" with boxes

