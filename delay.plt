set terminal png size 1024,768
set output "SPTM_test_delay.png"
set title "Uporedna analiza kašnjenja saobraćaja"
set xlabel "Kašnjenje (s)"
set ylabel "Broj paketa"
set border linewidth 2
set style line 1 linecolor rgb 'red' linetype 1 linewidth 2
set style line 2 linecolor rgb 'blue' linetype 1 linewidth 2
set style line 3 linecolor rgb 'green' linetype 1 linewidth 2
set style line 4 linecolor rgb 'purple' linetype 1 linewidth 2
set grid ytics
set grid xtics

plot "delay.dat" using 1:2 title "Bez AH" with linespoints ls 1, \
     "delay.dat" using 1:3 title "MD5" with linespoints ls 2, \
     "delay.dat" using 1:4 title "SHA-256" with linespoints ls 3, \
     "delay.dat" using 1:5 title "SHA-512" with linespoints ls 4

