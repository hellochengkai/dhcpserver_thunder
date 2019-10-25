#!/system/bin/sh
busybox date -s 201701010000.00
DOUBLEMAC=`getprop persist.dhcp.doublemac`
case $DOUBLEMAC in
 eth1)
  ip link add link eth0 eth1 type macvlan
  hidoublemac
  busybox ifconfig eth1 up
 ;;
 *)
 ;;
 esac

# OptimizedNetwork
test -f /system/bin/hioptimizednetwork && /system/bin/hioptimizednetwork

setprop sys.insmod.ko 1

insmod /system/lib/modules/hid-multitouch8000driver.ko

ts_usb_displayd&

#echo 1000000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
#echo interactive > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
#echo 200000 > /sys/devices/system/cpu/cpufreq/interactive/timer_rate

# DVFS support OptimizedBoot
echo 0 > /sys/module/mali/parameters/mali_avs_enable
echo 750000 > /sys/module/mali/parameters/mali_dvfs_max_freqency
DONGLE_GPU=`getprop persist.dongle.enable`
case $DONGLE_GPU in
 true)
  echo 400000 > /sys/module/mali/parameters/mali_dvfs_max_freqency
  ;;
 *)
  ;;
 esac

echo "\n\nWelcome to HiAndroid\n\n" > /dev/kmsg
LOW_RAM=`getprop ro.config.low_ram`
case $LOW_RAM in
 true)
  echo "\n\nenter low_ram mode\n\n" > /dev/kmsg
 #modules(memopt): Enable KSM in low ram device
  echo 1 > /sys/kernel/mm/ksm/run
  echo 300 > /sys/kernel/mm/ksm/sleep_millisecs
  ;;
 *)
  ;;
 esac

thunder_ts_daemon&
ts_sysmanager&
#stop adbd
thunder_dhcp.sh&
