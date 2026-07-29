# Copyright (c) 2008-2014 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ---------------------------------------------------------------------------
# JIHAD BROWSER — vendored copy of build-webos/weboslayers.py.
#
# oe-env.sh installs this over build-webos/weboslayers.py before running mcf, so
# the 2013 "dylan"-era layer pins (bitbake 1.18, oe-core/meta-oe dylan, the legacy
# openwebos meta-webos correct for the HP TouchPad) live in THIS repo and are
# reproducible from a clean clone — not left as a hand-edit in a sibling checkout.
#
# TWO changes vs upstream build-webos@37540e5:
#   1. Machines adds 'tenderloin' (HP TouchPad) + 'opal' (TouchPad Go) so mcf accepts
#      `./mcf tenderloin`. The machine .conf files live in the Jihad layer
#      (build/webos-oe/conf/machine/{tenderloin,opal}.conf), which oe-env.sh wires
#      into BBLAYERS after mcf (so mcf's git layer-management never touches it).
#   2. Nothing else — the upstream layer pins are kept byte-identical.
# ---------------------------------------------------------------------------
#
# webos_layers = [
# ('layer-name', priority, 'URL', 'submission', 'working-dir'),
# ...
# ]

Distribution = "webos"

# Supported MACHINE-s. qemux86/qemuarm kept for sanity builds; tenderloin + opal are
# the Jihad targets (machine confs supplied by the Jihad layer, wired post-mcf).
Machines = ['qemux86', 'qemuarm', 'tenderloin', 'opal']

# github.com/openembedded repositories are read-only mirrors of the authoritative
# repositories on git.openembedded.org
webos_layers = [
('bitbake',               -1, 'git://github.com/openembedded/bitbake.git',              'branch=1.18,commit=0f7b6a0', ''),
('meta',                   5, 'git://github.com/openembedded/openembedded-core.git',              'branch=dylan,commit=bf2d538', ''),
('meta-oe',                6, 'git://github.com/openembedded/meta-openembedded.git',              'branch=dylan,commit=70ebe86', ''),
('meta-networking',        6, 'git://github.com/openembedded/meta-openembedded.git',              '', ''),

('meta-webos-backports',   9, 'git://github.com/openwebos/meta-webos-backports.git',    'commit=ed80399', ''),
('meta-webos',            10, 'git://github.com/openwebos/meta-webos.git',              'commit=f43220d', ''),
]
