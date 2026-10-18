set -e
cd /home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser
bash build/webos-oe/in-container.sh --arm build/webos-oe/build-goanna-arm.sh build > build/webos-oe/out-arm/libxul-final.log 2>&1 || { grep -n ' error: ' build/webos-oe/out-arm/libxul-final.log | head -5; exit 3; }
bash build/webos-oe/in-container.sh --arm build/webos-oe/build-daemon-arm.sh > build/webos-oe/out-arm/daemon-final.log 2>&1
bash build/webos-oe/make-device-bundle.sh > build/webos-oe/out-arm/bundle-final.log 2>&1 && md5sum build/webos-oe/device-bundle/libxul.so | cut -c1-8
