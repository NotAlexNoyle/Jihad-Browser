/* Generated for Jihad adapter cross-build (device glibc has syslog/vsyslog/vfprintf). */
#define HAVE_SYSLOG_H 1
#ifndef PJSON_LOG_STDOUT
#define HAVE_VSYSLOG 1
#endif
#define HAVE_VFPRINTF 1
#if HAVE_SYSLOG_H
#include <syslog.h>
#endif
