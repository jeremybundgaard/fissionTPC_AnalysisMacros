#!/bin/bash
nEvents=-1
azMin=-180
azMax=180

if [ $# -gt 0 ];then
	nEvents=$1
fi
if [ $# -gt 1 ];then
	azMin=$2
fi
if [ $# -gt 2 ];then
	azMax=$3
fi

$myana/anahists/cuts/azimuth/azCuts.cfNoMask.anaHistMaker.sh $nEvents $azMin $azMax
$myana/anahists/cuts/azimuth/azCuts.cfCFT.anaHistMaker.sh $nEvents $azMin $azMax