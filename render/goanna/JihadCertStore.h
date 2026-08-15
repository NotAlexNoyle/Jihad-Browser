/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — the webOS platform certificate store (browser-services R5).
 *
 * Upstream's BrowserServer did not call a Luna service for this; it LINKED
 * libPmCertificateMgr (ref-BrowserServer/Makefile:20-23, USE_CERT_MGR=1 in the
 * shipping recipe). We reach the same C library through dlopen instead, for two
 * reasons: no header for it exists in this workspace, and the daemon must never
 * link OpenSSL (the device carries 0.9.8, our sysroot 1.0.0, and -lssl would
 * bake a SONAME the TouchPad does not have). Everything here is therefore
 * path- and serial-based; no X509* ever crosses the boundary.
 *
 * On desktop the library does not exist, so every entry point below degrades to
 * "Goanna-only trust" after one log line. That degrade path IS the desktop
 * behaviour and is what the desktop harness exercises.
 */
#ifndef JIHAD_CERT_STORE_H
#define JIHAD_CERT_STORE_H

#include <stddef.h>
#include <string>

namespace jihad {
namespace certstore {

// Which arm of the SSL-confirm dialog the user chose. The isis card has always
// offered three answers ("0" reject, "1" trust always, "2" trust once) and the
// two trust arms are different calls on BOTH sides of the fence: Goanna gets a
// permanent vs temporary validity override, the platform store gets
// CertAddAuthorizedCert vs CertAddTrustedCert.
enum Trust {
  kTrustSession   = 0,   // "trust once": swept at page teardown and at next startup
  kTrustPermanent = 1    // "trust always": survives a daemon restart
};

// True once libPmCertificateMgr.so has loaded AND every entry point the write
// path calls has resolved. The first call performs the dlopen. Always false on
// desktop, and false on any device that does not ship the library.
bool Available();

// Remove SESSION certificates a PREVIOUS daemon left behind, and delete stale
// PEM scratch files. Upstream's own comment (BrowserPage.cpp:1904-1910) flags
// the leak this closes: a daemon that dies without destructing turns every
// "trust once" into a permanent trust, and nothing upstream ever swept at
// startup. Call once before any Install.
void InitAndSweepStale();

// Install `pemPath` into the platform store under `trust`, then trust it.
// `owner` is an opaque token (the page) that scopes SweepSession; it is compared
// and never dereferenced. Returns false whenever the store is unavailable or the
// library refuses; the caller must treat that as "Goanna-only trust" and MUST
// NOT fail the dialog flow over it.
bool Install(const char* pemPath, Trust trust, const void* owner);

// Remove every session serial installed for `owner` and forget them.
void SweepSession(const void* owner);

// Armour `der` as a PEM both the card and the store can read, and write it out.
// Returns the file path, or "" when there is nowhere writable. The path is what
// msgSSLConfirm2 carries as certFile and what CertInstallKeyPackage is given.
std::string WriteCertPem(const void* der, size_t derLen);

// Delete a PEM written by WriteCertPem (no-op for an empty path).
void ForgetCertPem(const std::string& path);

// What is wrong with the certificate, as the engine can describe it. `haveFlags`
// false means nothing but the nsresult is known.
struct Problem {
  bool haveFlags;        // nsISSLStatus was captured, so the rest mean something
  bool untrusted;        // isUntrusted: unknown/distrusted issuer, self-signed chain
  bool domainMismatch;   // isDomainMismatch: right certificate, wrong host
  bool notValidNow;      // isNotValidAtThisTime: outside the validity window
  bool expired;          // ...and past notAfter (rather than before notBefore)
  bool selfSigned;       // nsIX509Cert::isSelfSigned
  Problem() : haveFlags(false), untrusted(false), domainMismatch(false),
              notValidNow(false), expired(false), selfSigned(false) {}
};

// Translate the failing nsresult the engine reports into the small ordinal space
// the card's SSL message table matches on (app/source/Browser.js:289-303, shared
// with the stock Mojo webview). Returns a generic "certificate is invalid"
// ordinal for anything unrecognised, never a raw nsresult.
int PalmSslErrorOrdinal(int nsresultStatus);

// Same, but preferring what the engine actually observed about the certificate.
// The document-stop nsresult for a rejected handshake is a transport-level code
// that says nothing about the certificate, so the flags are the better source
// whenever they exist; the nsresult map is the fallback.
int PalmSslErrorOrdinalFor(const Problem& problem, int nsresultStatus);

}  // namespace certstore
}  // namespace jihad

#endif  // JIHAD_CERT_STORE_H
