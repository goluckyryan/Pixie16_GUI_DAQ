#!/bin/bash -l

ExpName="testClover"
ElogIP="elog.physics.fsu.edu"
DataPath="/home/tandem/XIAEventBuilder/data"
elogIDFile="/home/tandem/XIAEventBuilder/data/elogID.txt"

echo "======================================= making elog Entry"

elogIDFile=${DataPath}"/elogID.txt"
if [ -f ${elogIDFile} ]; then
  source ${elogIDFile}
else
  touch ${elogIDFile}
fi
isStart=$1
lastRunID=$2
Comment=$3
elogFileName=${DataPath}"/elogText.txt"

#Get file size
RUN=${lastRunID}
runLen=${#lastRunID}
if [ ${runLen} -eq 1 ]; then
  RUN="00"${RUN}
elif [ ${runLen} -eq 2 ]; then
  RUN="0"${RUN}
fi;


if [ ${isStart} == "1" ]; then 

  #make text file for elog text
  echo "************************************************************************** <br />" > ${elogFileName}
  echo "Run-${RUN} start at $(date) <br />" >> ${elogFileName}
  echo "Start Run Comment : "${Comment}" <br />">> ${elogFileName}
  echo "-------------------------------------------------------------------------- <br />" >> ${elogFileName}
  
  #push to elog
  IDStr=$(elog -h ${ElogIP} -l ${ExpName} -u GeneralFox fsuphysics888 -a Author="GeneralFox" -a Category="Run" -a RunNo=${lastRunID} -a Subject="Run Log" -n 2 -m ${elogFileName})
  
  echo ${IDStr}
  
  IDStr=$(echo ${IDStr} | tail -1 | awk '{print $4}')
  
  if [ ${IDStr:0:3} == "ID=" ] && [[ ${IDStr:3} =~ ${re} ]]; then
    elogIDStr="Elog"${IDStr}
    echo "Elog is succefully pushed at ${elogIDStr}"
    echo ${elogIDStr} >> ${elogIDFile}
  fi
  
else
  
  elogFileName2=${DataPath}"/elogText2.txt"

  #download elog entry
  echo "====== getting elog entry"
  elog -h ${ElogIP} -l elog/${ExpName} -u GeneralFox fsuphysics888 -w ${ElogID} > ${elogFileName2}

  cutLineNum=$(grep -n "==============" ${elogFileName2} | cut -d: -f1)
  #check encoding
  encoding=$(grep "Encoding" ${elogFileName2} | awk '{print $2}')
  if [ $encoding = "plain" ]; then encodingID=1 ; fi
  if [ $encoding = "HTML" ]; then encodingID=2 ; fi
  if [ $encoding = "ELcode" ]; then encodingID=0 ; fi
  #remove all header
  sed -i "1,${cutLineNum}d" ${elogFileName2}
  
  echo "Run Stop at $(date) <br />" >> ${elogFileName2}  
  echo "End Run Comment : "${Comment}" <br />" >> ${elogFileName2}  
  totalFileSize=$(du -hc ${DataPath}"/"testRun_${RUN}* | tail -1| awk {'print $1'})
  echo "File Size : "${totalFileSize}" <br />">> ${elogFileName2}
  echo "************************************************************************** <br />" >> ${elogFileName2}
  
  cat ${elogFileName2}
  
  elog -h ${ElogIP} -p 8080 -l ${ExpName} -u GeneralFox fsuphysics888 -e ${ElogID} -n ${encodingID} -m ${elogFileName2}

fi
