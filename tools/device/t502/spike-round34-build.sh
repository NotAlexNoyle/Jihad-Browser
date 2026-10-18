set -e
cd /home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/third_party/uxp
git add -A gfx widget
git -c user.name="$(git -C .. config user.name)" -c user.email="$(git -C .. config user.email)" commit -q --amend --no-edit
cd /home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser
bash build/webos-oe/in-container.sh --arm build/webos-oe/build-goanna-arm.sh build > build/webos-oe/out-arm/libxul-gpu-spike34.log 2>&1 || { grep -n ' error: ' build/webos-oe/out-arm/libxul-gpu-spike34.log | head -5; exit 3; }
bash build/webos-oe/make-device-bundle.sh > build/webos-oe/out-arm/bundle-gpu-spike34.log 2>&1 && md5sum build/webos-oe/device-bundle/libxul.so | cut -c1-8
