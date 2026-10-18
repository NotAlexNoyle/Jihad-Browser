echo "--- adapter.log since the last screenshot marker (all lines)"
awk '/--- marker screenshot/{buf=""; next} {buf=buf $0 "\n"} END{printf "%s", buf}' /var/palm/jihad/enyo/adapter.log | cut -c1-170 | head -n 40
echo "--- the 12 lines before the marker"
grep -a -B12 -- '--- marker screenshot' /var/palm/jihad/enyo/adapter.log | tail -n 13 | cut -c1-170
