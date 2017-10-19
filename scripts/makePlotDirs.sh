#!/bin/bash
mkdir -p ../plots/
cd ../plots/
#make directories for every run
for dir in all2017 RunA RunB RunC RunD RunE RunF
    do mkdir -p $dir
done
for dir in ./*
    do for subdir in inclusive ee em mm
        do mkdir -p $dir/$subdir
    done
done
