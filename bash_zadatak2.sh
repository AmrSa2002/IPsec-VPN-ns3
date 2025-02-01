#!/bin/sh

#rm -rf last_seed_file
#rm -rf *.txt *.plt *.dat *.pcap


# Petlja za 5 iteracija
for broj in 1 2 3 4 5
do
    # Generisanje različitih netanim fajlova
    output_file="anim_zadatak2_${broj}.xml"
    
    # Pokretanje programa s različitim vrijednostima za --broj i postavljanje izlaznog fajla
    ./ns3 run "scratch/brite-generic-example --broj=${broj}" > results_${broj}.txt 2>&1
    
    # Pretpostavimo da program generiše animaciju u 'anim.xml' i preimenujemo je
    if [ -f "anim_zadatak2.xml" ]; then
        mv anim_zadatak2.xml ${output_file}
        echo "Generisan fajl: ${output_file}"
    else
        echo "Greška: anim.xml nije generisan u iteraciji ${broj}"
    fi
done

