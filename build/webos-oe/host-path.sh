# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Sourced by every script here that runs novacom or a palm-* tool. It puts the repository's own
# copies ahead of anything installed on the host (cavekit-device-build.md R11):
#
#   tools/host/bin/                        novacom and novacomd, built by tools/host/build-novacom.sh
#   third_party/webos-sdk-redux/Current/bin/  palm-package, palm-install, palm-launch, palm-log …
#                                          (webOS Archive's SDK, a submodule; Java runs them)
#
# A directory that does not exist yet is skipped, so an installed SDK or novacom still works.
# Both bash and sh source this; a script in another directory sets JIHAD_REPO first.
_jihad_repo="${JIHAD_REPO:-$(cd "$(dirname "$0")/../.." && pwd)}"
for _jihad_dir in "$_jihad_repo/third_party/webos-sdk-redux/Current/bin" "$_jihad_repo/tools/host/bin"; do
	if [ -d "$_jihad_dir" ]; then
		case ":$PATH:" in
			*":$_jihad_dir:"*) ;;
			*) PATH="$_jihad_dir:$PATH" ;;
		esac
	fi
done
export PATH
unset _jihad_repo _jihad_dir
