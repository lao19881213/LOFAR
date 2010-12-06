#!/bin/sh
# 1.4 subband statistics HBA test 
# 03-12-10, M.J Norden
# HBA input with antennas

rspctl --rcuprsg=0
rspctl --wg=0
rspctl --splitter=0
killall beamctl

swlevel 3
sleep 5
beamctl --antennaset=HBA_JOINED --rcus=0:95 --rcumode=5 --subbands=100:110 --beamlets=0:10 --anadir=0,1.5708,AZEL --digdir=0,1.5708,AZEL&
sleep 15

echo ==========================
echo "Subband Statistics HBA rcumode=5" `hostname -s`
echo ==========================
rspctl --stati& 
sleep 20 && kill $!
killall beamctl
