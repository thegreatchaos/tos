target=10.67.109.54:7777
hostname=`hostname`
ip=`hostname -I | awk '{print $1}'` 
curl -X GET $target/$hostname/$ip/magic0x7777
