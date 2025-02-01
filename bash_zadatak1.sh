#!/bin/sh

#rm -rf last_seed_file
#rm -rf *.txt *.plt *.dat *.pcap

for packetSize in 150 300 500
do
       ./ns3 run "scratch/Zadaca2_1c --packetSize=${packetSize}" > results1.txt 2>&1 
        printf "${packetSize},$(grep "Sent (bytes)" results1.txt ), $(grep "Sent (Packets)" results1.txt) \n"
done
