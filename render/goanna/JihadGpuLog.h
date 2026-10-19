/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#ifndef JIHAD_GPU_LOG_H
#define JIHAD_GPU_LOG_H

// T-512: the GPU path's diagnostics print only with JIHAD_GPU_LOG=1, or in a device round's
// JIHAD_GPU_TIMELINE, JIHAD_GPU_VERIFY or JIHAD_GPU_DUMP_EVERY; failures and fallbacks print anyway
// (plain fprintf). An expression, its switch read once at each place, so it stands anywhere a call
// does. The engine has the same gate (mozilla/gfx/JihadGpuLog.h, engine patch queue).

#include <cstdio>
#include <cstdlib>

#ifndef JIHAD_GPU_LOG
#define JIHAD_GPU_LOG(...) \
  ([]() { static const bool sJihadGpuLogOn = getenv("JIHAD_GPU_LOG") || getenv("JIHAD_GPU_TIMELINE") || \
                                             getenv("JIHAD_GPU_VERIFY") || getenv("JIHAD_GPU_DUMP_EVERY"); \
           return sJihadGpuLogOn; }() ? fprintf(stderr, __VA_ARGS__) : 0)
#endif

#endif // JIHAD_GPU_LOG_H
