#!/bin/bash

# GPIO driver has issues on unload. Avoiding unload for now.

# GPIO30=1 -- low
# GPIO31=1 -- medium (overrides low)
# GPIO32=1 -- high (overrides low and medium)


BASEDIR=/sources/lenovoEMC-300d

if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root" 1>&2
    exit 1
fi

if [[ ! $(lsmod | grep gpio_f7188x) ]]; then
    pushd $BASEDIR/gpio-f7188x
    make clean
    make
    # Unloading the driver breaks things!
    #sudo rmmod gpio_f7188x 2>1 >/dev/null
    sleep 2
    sudo insmod gpio-f7188x.ko
    popd
fi

for i in {30..32}; do
    echo "$i" > /sys/class/gpio/export
    echo "out" > /sys/class/gpio/gpio${i}/direction
    echo "0" > /sys/class/gpio/gpio${i}/value
    echo "$i" > /sys/class/gpio/unexport
done
# Unloading the driver breaks things!
#sudo rmmod gpio-f7188x
echo "Lights should be out."
