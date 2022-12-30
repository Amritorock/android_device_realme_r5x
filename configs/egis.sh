#!/system/bin/sh
#
# This script is Needed to replace the FP hal for egis devices
# Fix by @sarthakroy2002
#

mount -o rw /vendor

FP=$(cat /proc/fp_id)

if [ $FP = "E_520" ]; then
    echo "EGIS FP Detected"
    rm /vendor/bin/hw/android.hardware.biometrics.fingerprint@2.1-service.r5x
    rm /vendor/etc/init/android.hardware.biometrics.fingerprint@2.1-service.r5x.rc
    mv /vendor/odm/bin/hw/vendor.oppo.hardware.biometrics.fingerprint@2.1-service-RMX2030 /vendor/bin/hw/vendor.oppo.hardware.biometrics.fingerprint@2.1-service-RMX2030
    mv /vendor/odm/etc/init/android.hardware.biometrics.fingerprint@2.1-service-RMX2030.rc /vendor/etc/init/android.hardware.biometrics.fingerprint@2.1-service-RMX2030.rc
    mv /vendor/odm/etc/init/vendor.oppo.hardware.biometrics.fingerprint@2.1-service-RMX2030.rc /vendor/etc/init/vendor.oppo.hardware.biometrics.fingerprint@2.1-service-RMX2030.rc
    mv /vendor/odm/lib64/hw/libets_teeclient_v2.so /vendor/lib64/hw/libets_teeclient_v2.so
fi

rm -rf /vendor/odm/bin /vendor/odm/etc/init /vendor/odm/lib64
