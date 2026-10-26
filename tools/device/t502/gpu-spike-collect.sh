echo "=== LOG BEGIN"; cat /var/palm/jihad/enyo/daemon-gpu.log; echo "=== LOG END"

rm -f /tmp/frame.raw /tmp/frames.raw /tmp/frame.raw-* /tmp/st-pan.png /tmp/st-gpu.png   # T-527: the dumps once fetched (/tmp is 40 MB)
