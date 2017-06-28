#!/bin/bash
# script info
# 
 
# CHECK INPUT PARAMETERS
reqparams=1
if [ $# -ne $reqparams ]; then
  echo "USAGE: $0 [continue (1/0)]"
  exit
fi
contLogical=$1

#
flatFile=Complete_v20170403_Mod_EQLocations_20170509.csv 
modificationFile=ver20170518/Complete_Reprocess_2.csv
outputFile=Complete_v20170403_20170521.csv
 
#
if (( $contLogical )); then 
  echo add_gms_to_flatfile $flatFile $modificationFile $outputFile
  add_gms_to_flatfile $flatFile $modificationFile $outputFile
fi
