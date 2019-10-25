#!/system/bin/sh

while [[ true ]]; do
	sleep 5
	ps | grep "dhcpserver"
	if [ $? != 0 ];then
	    dhcpserver -d eth0 null
  fi
done
