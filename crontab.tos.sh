target=10.67.109.54:7777
hostname=`hostname`
ip=`hostname -I | awk '{print $1}'` 
ts=`date +%Y%m%d:%H%M%S`
curl -X GET $target/$hostname/$ip/$ts
