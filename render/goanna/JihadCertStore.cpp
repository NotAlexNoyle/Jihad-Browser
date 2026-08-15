/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — webOS platform certificate store. See JihadCertStore.h.
 *
 * Deliberately free of NSS, XPCOM and OpenSSL headers: the only things that
 * cross the libPmCertificateMgr boundary are a file path and an int serial.
 */
#include "JihadCertStore.h"
#include "JihadRuntimePaths.h"      // the ONE runtime-state dir (T-057 / R8)

#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <vector>

namespace jihad {
namespace certstore {

namespace {

// The nine entry points upstream used, typedef'd from its call sites
// (ref-BrowserServer/Src/BrowserPage.cpp) because no header for this library
// exists in the workspace. All return CertReturnCode_t, with CERT_OK == 0.
typedef int (*fn_CertInitCertMgr)(const char* configFile);
typedef int (*fn_CertGetDatabaseInfo)(int type, int* out);
typedef int (*fn_CertGetDatabaseStrValue)(int index, int property, char* buf, int len);
typedef int (*fn_makePathToCert)(int serial, char* buf, int len);
typedef int (*fn_CertPemToX509)(const char* path, void** x509);
typedef int (*fn_CertInstallKeyPackage)(const char* pkgPath, const char* destPath,
                                        char* passPhrase, int* serial);
typedef int (*fn_CertAddAuthorizedCert)(int serial);
typedef int (*fn_CertAddTrustedCert)(int serial);
typedef int (*fn_CertRemoveCertificate)(int serial);

const int kCertOk = 0;

struct Api {
  void* lib;
  fn_CertInitCertMgr          InitCertMgr;
  fn_CertGetDatabaseInfo      GetDatabaseInfo;        // read direction, not called yet
  fn_CertGetDatabaseStrValue  GetDatabaseStrValue;    // read direction, not called yet
  fn_makePathToCert           MakePathToCert;         // read direction, not called yet
  fn_CertPemToX509            PemToX509;              // needs OpenSSL types, never called
  fn_CertInstallKeyPackage    InstallKeyPackage;
  fn_CertAddAuthorizedCert    AddAuthorizedCert;
  fn_CertAddTrustedCert       AddTrustedCert;
  fn_CertRemoveCertificate    RemoveCertificate;
  bool tried;
  bool ready;
};
Api gApi = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
             nullptr, nullptr, nullptr, nullptr, false, false };

struct SessionCert { int serial; const void* owner; };
std::vector<SessionCert> gSession;

// ── the library ────────────────────────────────────────────────────────────
// One log line on every degrade path, never more: an absent store is the NORMAL
// state on desktop and possibly the normal state on HP's 3.0.5 (the recipe that
// provides this library is Open-webOS era), so it must not be noise.
bool loadApi() {
  if (gApi.tried) return gApi.ready;
  gApi.tried = true;

  // No SONAME guessing: this is the name the recipe installs, and the same
  // dlopen shape JihadLunaService uses for liblunaservice.so.
  gApi.lib = dlopen("libPmCertificateMgr.so", RTLD_NOW);
  if (!gApi.lib) {
    const char* e = dlerror();
    fprintf(stderr, "[jihad-bs] cert: libPmCertificateMgr.so unavailable (%s) — Goanna-only trust\n",
            e ? e : "?");
    return false;
  }
  gApi.InitCertMgr         = (fn_CertInitCertMgr)        dlsym(gApi.lib, "CertInitCertMgr");
  gApi.GetDatabaseInfo     = (fn_CertGetDatabaseInfo)    dlsym(gApi.lib, "CertGetDatabaseInfo");
  gApi.GetDatabaseStrValue = (fn_CertGetDatabaseStrValue)dlsym(gApi.lib, "CertGetDatabaseStrValue");
  gApi.MakePathToCert      = (fn_makePathToCert)         dlsym(gApi.lib, "makePathToCert");
  gApi.PemToX509           = (fn_CertPemToX509)          dlsym(gApi.lib, "CertPemToX509");
  gApi.InstallKeyPackage   = (fn_CertInstallKeyPackage)  dlsym(gApi.lib, "CertInstallKeyPackage");
  gApi.AddAuthorizedCert   = (fn_CertAddAuthorizedCert)  dlsym(gApi.lib, "CertAddAuthorizedCert");
  gApi.AddTrustedCert      = (fn_CertAddTrustedCert)     dlsym(gApi.lib, "CertAddTrustedCert");
  gApi.RemoveCertificate   = (fn_CertRemoveCertificate)  dlsym(gApi.lib, "CertRemoveCertificate");

  // Only the five the WRITE path calls are required. The other four are resolved
  // so the one log line below doubles as the device evidence for whether the
  // read direction (platform store -> NSS import) is even reachable here.
  if (!gApi.InitCertMgr || !gApi.InstallKeyPackage || !gApi.AddAuthorizedCert ||
      !gApi.AddTrustedCert || !gApi.RemoveCertificate) {
    fprintf(stderr, "[jihad-bs] cert: libPmCertificateMgr.so is missing entry points "
                    "(init=%d install=%d auth=%d trust=%d remove=%d dbinfo=%d dbstr=%d path=%d pem=%d) "
                    "— Goanna-only trust\n",
            (int)!!gApi.InitCertMgr, (int)!!gApi.InstallKeyPackage, (int)!!gApi.AddAuthorizedCert,
            (int)!!gApi.AddTrustedCert, (int)!!gApi.RemoveCertificate, (int)!!gApi.GetDatabaseInfo,
            (int)!!gApi.GetDatabaseStrValue, (int)!!gApi.MakePathToCert, (int)!!gApi.PemToX509);
    return false;
  }

  // Once, guarded, exactly where upstream did it (BrowserServer.cpp:161). A
  // non-zero return is reported and not treated as fatal: some builds answer
  // that way for "already initialised", and every later call reports its own rc.
  int rc = gApi.InitCertMgr("/etc/ssl/openssl.cnf");
  fprintf(stderr, "[jihad-bs] cert: platform store available (CertInitCertMgr rc=%d, "
                  "read-direction syms dbinfo=%d dbstr=%d path=%d)\n",
          rc, (int)!!gApi.GetDatabaseInfo, (int)!!gApi.GetDatabaseStrValue,
          (int)!!gApi.MakePathToCert);
  gApi.ready = true;
  return true;
}

// ── where the PEM goes ─────────────────────────────────────────────────────
// Two candidates, in order, and the choice is deliberate:
//
//   1. /var/ssl/jihad/<variant> — /var/ssl IS the platform store root, and
//      upstream's Main.cpp:274-281 chmod/chowns it to root.luna at startup. It
//      is preferred so our scratch PEMs sit with the store they feed. Whether
//      OUR daemon's group covers it is UNVERIFIED (device check), which is why
//      we take our own 0755 subdirectory rather than writing beside the openssl
//      hash symlinks, and why we never require it.
//   2. <runtime state dir>/certs — the daemon's own per-variant state, always
//      writable. This is what keeps the certFile field NON-EMPTY (and so keeps
//      the card's "View Certificate" button alive) when /var/ssl is not ours.
//
// Per-variant either way: three variant daemons can run at once, and the startup
// sweep deletes stale PEMs by directory, so they must not share one.
// Desktop has no /var/ssl, so it deterministically lands on (2).
const std::string& certDir() {
  static const std::string dir = []() -> std::string {
    std::string out;
    const std::string variant = jihad::RuntimeVariant();
    struct stat st;
    if (stat("/var/ssl", &st) == 0 && S_ISDIR(st.st_mode) &&
        jihad::RuntimeTryDir("/var/ssl/jihad/" + variant, 0755, out)) {
      fprintf(stderr, "[jihad-bs] cert: certificate files -> %s (platform store root)\n", out.c_str());
      return out;
    }
    const std::string& state = jihad::RuntimeStateDir();
    if (!state.empty() && jihad::RuntimeTryDir(state + "/certs", 0755, out)) {
      fprintf(stderr, "[jihad-bs] cert: certificate files -> %s (/var/ssl not writable by us)\n",
              out.c_str());
      return out;
    }
    fprintf(stderr, "[jihad-bs] cert: no writable directory for certificate files — "
                    "the card's View Certificate button will stay disabled\n");
    return std::string();
  }();
  return dir;
}

// The serials of session certs live on disk as well as in memory, because the
// leak this closes is exactly the case where the in-memory list dies with us.
std::string sessionListPath() { return jihad::RuntimeStatePath("cert-session-serials"); }

void persistSessionList() {
  const std::string path = sessionListPath();
  if (path.empty()) return;
  if (gSession.empty()) { unlink(path.c_str()); return; }
  FILE* f = fopen(path.c_str(), "wb");
  if (!f) return;
  for (size_t i = 0; i < gSession.size(); ++i) fprintf(f, "%d\n", gSession[i].serial);
  fflush(f);
  fsync(fileno(f));   // the point of the file is surviving a daemon that dies badly
  fclose(f);
}

const char kB64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string armourPem(const unsigned char* d, size_t n) {
  std::string body;
  body.reserve(((n + 2) / 3) * 4 + n / 48 + 8);
  size_t col = 0;
  for (size_t i = 0; i < n; i += 3) {
    const unsigned v = ((unsigned)d[i] << 16) |
                       ((i + 1 < n) ? ((unsigned)d[i + 1] << 8) : 0u) |
                       ((i + 2 < n) ? (unsigned)d[i + 2] : 0u);
    body += kB64[(v >> 18) & 0x3f];
    body += kB64[(v >> 12) & 0x3f];
    body += (i + 1 < n) ? kB64[(v >> 6) & 0x3f] : '=';
    body += (i + 2 < n) ? kB64[v & 0x3f] : '=';
    col += 4;
    if (col >= 64) { body += '\n'; col = 0; }
  }
  if (col) body += '\n';
  return "-----BEGIN CERTIFICATE-----\n" + body + "-----END CERTIFICATE-----\n";
}

}  // namespace

bool Available() { return loadApi(); }

std::string WriteCertPem(const void* der, size_t derLen) {
  if (!der || !derLen) return std::string();
  const std::string& dir = certDir();
  if (dir.empty()) return std::string();

  static unsigned sSeq = 0;
  char leaf[64];
  snprintf(leaf, sizeof leaf, "/jihad-cert-%ld-%u.pem", (long)getpid(), ++sSeq);
  const std::string path = dir + leaf;
  const std::string pem = armourPem(static_cast<const unsigned char*>(der), derLen);

  FILE* f = fopen(path.c_str(), "wb");
  if (!f) {
    fprintf(stderr, "[jihad-bs] cert: cannot write %s (%s)\n", path.c_str(), strerror(errno));
    return std::string();
  }
  const size_t wrote = fwrite(pem.data(), 1, pem.size(), f);
  const int closed = fclose(f);
  if (wrote != pem.size() || closed != 0) {
    // A truncated PEM is worse than none: getcertificatedetails would answer
    // returnValue=false and the card would silently keep the button disabled.
    fprintf(stderr, "[jihad-bs] cert: short write %zu/%zu on %s — dropping it\n",
            wrote, pem.size(), path.c_str());
    unlink(path.c_str());
    return std::string();
  }
  // com.palm.certificatemanager reads this file from ANOTHER process to answer
  // the card's getcertificatedetails, so it has to be world-readable.
  chmod(path.c_str(), 0644);
  fprintf(stderr, "[jihad-bs] cert: wrote %zu-byte DER as PEM -> %s\n", derLen, path.c_str());
  return path;
}

void ForgetCertPem(const std::string& path) {
  if (!path.empty()) unlink(path.c_str());
}

bool Install(const char* pemPath, Trust trust, const void* owner) {
  if (!pemPath || !*pemPath) return false;
  if (!loadApi()) return false;

  int serial = 0;
  char passPhrase[128];
  memset(passPhrase, 0, sizeof passPhrase);
  int rc = gApi.InstallKeyPackage(pemPath, nullptr, passPhrase, &serial);
  if (rc != kCertOk) {
    fprintf(stderr, "[jihad-bs] cert: CertInstallKeyPackage(%s) rc=%d — Goanna-only trust\n",
            pemPath, rc);
    return false;
  }
  // CertAddAuthorizedCert, NOT CertAddTrustedCert, is the permanent arm:
  // upstream's note (BrowserPage.cpp:1986) says it is the one that creates the
  // correct openssl hash symlinks. Do not swap them.
  rc = (trust == kTrustPermanent) ? gApi.AddAuthorizedCert(serial)
                                  : gApi.AddTrustedCert(serial);
  if (rc != kCertOk) {
    fprintf(stderr, "[jihad-bs] cert: %s(serial %d) rc=%d — removing the half-installed cert\n",
            (trust == kTrustPermanent) ? "CertAddAuthorizedCert" : "CertAddTrustedCert",
            serial, rc);
    // Upstream left this behind. An installed-but-untrusted cert is dead weight
    // in the store and, for the session arm, one nobody would ever sweep.
    int rrc = gApi.RemoveCertificate(serial);
    if (rrc != kCertOk)
      fprintf(stderr, "[jihad-bs] cert: CertRemoveCertificate(%d) rc=%d\n", serial, rrc);
    return false;
  }
  if (trust == kTrustSession) {
    SessionCert s; s.serial = serial; s.owner = owner;
    gSession.push_back(s);
    persistSessionList();
  }
  fprintf(stderr, "[jihad-bs] cert: platform store now trusts serial %d (%s)\n", serial,
          (trust == kTrustPermanent) ? "permanent" : "session");
  return true;
}

void SweepSession(const void* owner) {
  if (gSession.empty()) return;
  std::vector<SessionCert> keep;
  for (size_t i = 0; i < gSession.size(); ++i) {
    if (gSession[i].owner != owner) { keep.push_back(gSession[i]); continue; }
    if (!loadApi()) { keep.push_back(gSession[i]); continue; }
    int rc = gApi.RemoveCertificate(gSession[i].serial);
    fprintf(stderr, "[jihad-bs] cert: sweep session serial %d rc=%d\n", gSession[i].serial, rc);
    if (rc != kCertOk) keep.push_back(gSession[i]);
  }
  gSession.swap(keep);
  persistSessionList();
}

void InitAndSweepStale() {
  static bool sDone = false;
  if (sDone) return;
  sDone = true;

  // 1. Session serials a previous daemon died holding. Without this sweep every
  //    "trust once" that outlived a crash is permanent, which is upstream's own
  //    recorded hole (BrowserPage.cpp:1904-1910) and the reason this runs at
  //    startup and not only at teardown.
  const std::string listPath = sessionListPath();
  if (!listPath.empty()) {
    FILE* f = fopen(listPath.c_str(), "rb");
    if (f) {
      std::vector<int> stale;
      char line[64];
      while (fgets(line, sizeof line, f)) {
        int serial = atoi(line);
        if (serial != 0) stale.push_back(serial);
      }
      fclose(f);
      if (!stale.empty()) {
        if (loadApi()) {
          for (size_t i = 0; i < stale.size(); ++i) {
            int rc = gApi.RemoveCertificate(stale[i]);
            fprintf(stderr, "[jihad-bs] cert: startup sweep of leftover session serial %d rc=%d\n",
                    stale[i], rc);
          }
          unlink(listPath.c_str());
        } else {
          // Keep the list: the certs are still in the store, and a later boot
          // with the library present is the only thing that can clear them.
          fprintf(stderr, "[jihad-bs] cert: %zu leftover session serial(s) in %s and no platform "
                          "store to remove them from — left for a later run\n",
                  stale.size(), listPath.c_str());
        }
      } else {
        unlink(listPath.c_str());
      }
    }
  }

  // 2. Scratch PEMs. These are unlinked as soon as a dialog resolves, so anything
  //    still here belongs to a daemon that died mid-prompt. The directory is
  //    per-variant, so this cannot touch another variant's live prompt; two
  //    daemons of the SAME variant (a hand-launched one plus upstart) can race
  //    here, and the worst case is one dead View Certificate button.
  const std::string& dir = certDir();
  if (dir.empty()) return;
  DIR* d = opendir(dir.c_str());
  if (!d) return;
  int removed = 0;
  while (struct dirent* e = readdir(d)) {
    if (strncmp(e->d_name, "jihad-cert-", 11) != 0) continue;
    if (unlink((dir + "/" + e->d_name).c_str()) == 0) ++removed;
  }
  closedir(d);
  if (removed)
    fprintf(stderr, "[jihad-bs] cert: startup sweep removed %d stale certificate file(s)\n", removed);
}

// ── nsresult -> the card's ordinal space ───────────────────────────────────
//
// The card matches Palm's small ordinals, which are OpenSSL X509_V_ERR_* codes:
//   0        "The security certificate ... is expired"
//   2-4      "... didn't send a security certificate"
//   5-9      "... could not be read completely"
//   10-17    "... has some invalid information"
//   18-23    "... has questionable signatures"
//   24-29    "... is invalid"
//   30,31,50 "... has inconsistent information in it"
// Anything outside those matches NOTHING, leaves `msg` undefined, and the dialog
// opens with an empty body — which is what passing a raw nsresult (0x805a2fe3)
// did on every prompt.
//
// The ordinal's whole job is to SELECT ONE OF THOSE SENTENCES, so where the
// OpenSSL code and the card's wording disagree, the wording wins and the choice
// is noted. The two places that happens are marked below.
int PalmSslErrorOrdinal(int nsresultStatus) {
  const unsigned s = (unsigned)nsresultStatus;

  // Generic "is invalid" — honest for an unknown failure, and never a raw nsresult.
  const int kGeneric = 27;   // X509_V_ERR_CERT_UNTRUSTED

  // Not a security-module nsresult: nothing to decode.
  if (((s >> 16) & 0x7fffu) != (21u + 0x45u)) return kGeneric;

  // psm::GetXPCOMFromNSSError stores -1 * the NSPR error code in the low 16 bits.
  const int nspr = -(int)(s & 0xffffu);

  // NSS bases, spelled out so this file needs no NSS headers:
  //   SEC_ERROR_BASE -0x2000 (-8192), SSL_ERROR_BASE -0x3000 (-12288),
  //   mozilla::pkix ERROR_BASE -0x4000 (-16384).
  switch (nspr) {
    // Validity period.
    case -8181:   // SEC_ERROR_EXPIRED_CERTIFICATE
    case -8162:   // SEC_ERROR_EXPIRED_ISSUER_CERTIFICATE
      // WORDING WINS: OpenSSL's expiry code is 10, whose card sentence is the
      // vague "has some invalid information"; ordinal 0 says "is expired".
      return 0;
    case -16379:  // MOZILLA_PKIX_ERROR_NOT_YET_VALID_CERTIFICATE
    case -16378:  // MOZILLA_PKIX_ERROR_NOT_YET_VALID_ISSUER_CERTIFICATE
    case -8184:   // SEC_ERROR_INVALID_TIME
      // WORDING WINS: OpenSSL's "not yet valid" is 9, whose sentence is "could
      // not be read completely" and is simply untrue here. 10 says "has some
      // invalid information", which a clock-skewed certificate does.
      return 10;

    // No certificate at all.
    case -12285:  // SSL_ERROR_NO_CERTIFICATE
      return 3;   // X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE range: "didn't send a certificate"

    // Unreadable / unverifiable encoding.
    case -8183:   // SEC_ERROR_BAD_DER
      return 6;   // X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY
    case -8182:   // SEC_ERROR_BAD_SIGNATURE
    case -16377:  // MOZILLA_PKIX_ERROR_SIGNATURE_ALGORITHM_MISMATCH
    case -8016:   // SEC_ERROR_CERT_SIGNATURE_ALGORITHM_DISABLED
      return 7;   // X509_V_ERR_CERT_SIGNATURE_FAILURE

    // Issuer not known / self-signed: "has questionable signatures".
    case -16370:  // MOZILLA_PKIX_ERROR_SELF_SIGNED_CERT
      return 18;  // X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT
    case -8179:   // SEC_ERROR_UNKNOWN_ISSUER
      return 20;  // X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY
    case -8180:   // SEC_ERROR_REVOKED_CERTIFICATE
      return 23;  // X509_V_ERR_CERT_REVOKED

    // Explicitly distrusted, or not usable for this purpose: "is invalid".
    case -8156:   // SEC_ERROR_CA_CERT_INVALID
    case -16383:  // MOZILLA_PKIX_ERROR_CA_CERT_USED_AS_END_ENTITY
    case -16381:  // MOZILLA_PKIX_ERROR_V1_CERT_USED_AS_CA
      return 24;  // X509_V_ERR_INVALID_CA
    case -8102:   // SEC_ERROR_INADEQUATE_KEY_USAGE
    case -8101:   // SEC_ERROR_INADEQUATE_CERT_TYPE
    case -16382:  // MOZILLA_PKIX_ERROR_INADEQUATE_KEY_SIZE
      return 26;  // X509_V_ERR_INVALID_PURPOSE
    case -8172:   // SEC_ERROR_UNTRUSTED_ISSUER
      return 27;  // X509_V_ERR_CERT_UNTRUSTED
    case -8171:   // SEC_ERROR_UNTRUSTED_CERT
      return 28;  // X509_V_ERR_CERT_REJECTED

    // The name does not match the site: "has inconsistent information in it".
    case -12276:  // SSL_ERROR_BAD_CERT_DOMAIN
    case -8080:   // SEC_ERROR_CERT_NOT_IN_NAME_SPACE
    case -8100:   // SEC_ERROR_CERT_ADDR_MISMATCH
      // OpenSSL 0.9.8 has no hostname-mismatch code at all (X509_V_ERR_HOSTNAME_MISMATCH
      // is a 1.0.2 addition), so 50 = X509_V_ERR_APPLICATION_VERIFICATION is the
      // slot upstream's own table leaves for a check openssl did not make.
      return 50;

    default:
      return kGeneric;
  }
}

int PalmSslErrorOrdinalFor(const Problem& p, int nsresultStatus) {
  if (!p.haveFlags) return PalmSslErrorOrdinal(nsresultStatus);
  // Ordered by what the user most needs told. "Untrusted" outranks a bad clock
  // or a wrong name because it is the one that means the site's identity was
  // never established at all; a certificate can and often does trip several.
  if (p.untrusted) return p.selfSigned ? 18   // X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT
                                       : 20;  // X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY
  if (p.notValidNow) return p.expired ? 0     // the card's one explicit "is expired"
                                      : 10;   // "has some invalid information"
  if (p.domainMismatch) return 50;            // "has inconsistent information in it"
  return PalmSslErrorOrdinal(nsresultStatus);
}

}  // namespace certstore
}  // namespace jihad
