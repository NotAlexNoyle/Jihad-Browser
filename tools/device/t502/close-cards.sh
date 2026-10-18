n=0
for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 'net.riverstonerelay.jihad-browser"' | grep processid | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do
  luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1; n=$((n+1))
done
echo "closed $n browser cards"; sleep 3
luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | grep -o 'jihad-browser' | wc -l
