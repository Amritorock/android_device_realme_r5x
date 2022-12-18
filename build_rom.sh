#!/usr/bin/env bash

rm -rf /cache/*

BASE_DIR="$(pwd)"
SOURCEDIR="${BASE_DIR}/crdroid"

git config --global user.email "amritokarmokar5i@gmail.com" && git config --global user.name "Amritorock"
df -h
mkdir -p "${SOURCEDIR}"
cd "${SOURCEDIR}"

# Sync Source
repo init -u https://github.com/crdroidandroid/android.git -b 13.0
repo sync

# Source Patches (Optional)
cd frameworks/base
wget https://raw.githubusercontent.com/sarthakroy2002/random-stuff/main/Patches/Fix-brightness-slider-curve-for-some-devices -a12l.patch
patch -p1 <*.patch
cd "${SOURCEDIR}"

# Sync Trees
git clone --depth=1 https://github.com/Amritorock/android_device_realme_r5x -b rice device/realme/r5x
git clone --depth=1 https://github.com/Amritorock/vendor_realme_r5x -b Trinket vendor/realme/r5x
git clone --depth=1 https://github.com/Amritorock/kernel_realme_r5x -b OSS kernel/realme/r5x
rm -rf frameworks/opt/telephony
git clone --depth=1 https://github.com/Amritorock/android_frameworks_opt_telephony -b 13.0 frameworks/opt/telephony

# Start Build
source build/envsetup.sh
lunch lineage_r5x-userdebug
make bacon

# Upload
cd out/target/product/r5x
curl -sL https://git.io/file-transfer | sh
./transfer wet crDroid*.zip
