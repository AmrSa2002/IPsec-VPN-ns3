set terminal png
set output "SPTM_test_delay.png"
set title "SPTM Test Delay"
set xlabel "Packet"
set ylabel "Time (second)"
set border linewidth 2
set style line 1 linecolor rgb 'red' linetype 1 linewidth 2
set style line 2 linecolor rgb 'blue' linetype 1 linewidth 2
set grid ytics
set grid xtics

plot "delay.dat" using 1:2 title "Delay (N2->N1):" with linespoints ls 1, \
     "delay.dat" using 1:3 title "Delay (N3->N2):" with linespoints ls 2

