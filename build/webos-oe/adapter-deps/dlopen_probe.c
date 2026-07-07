/* Isolated on-device load test for the cross-built BrowserAdapter NPAPI plugin.
 * dlopen(RTLD_NOW) forces immediate resolution of every symbol, so any missing
 * lib or undefined reference surfaces here WITHOUT loading the plugin into
 * LunaSysMgr (no risk to the running UI). Then we call the pure NPAPI entry
 * points to prove the plugin's own code links. Built as plain C: no libstdc++. */
#include <dlfcn.h>
#include <stdio.h>

typedef const char *(*mime_fn)(void);

int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : "./BrowserAdapter.so";
    printf("[probe] dlopen(RTLD_NOW) %s\n", path);
    void *h = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!h) {
        printf("[probe] FAIL dlopen: %s\n", dlerror());
        return 1;
    }
    printf("[probe] OK loaded, no undefined symbols\n");

    void *init = dlsym(h, "NP_Initialize");
    void *shut = dlsym(h, "NP_Shutdown");
    mime_fn mime = (mime_fn)dlsym(h, "NP_GetMIMEDescription");
    printf("[probe] NP_Initialize=%p NP_Shutdown=%p NP_GetMIMEDescription=%p\n",
           init, shut, (void *)mime);
    if (mime) {
        const char *m = mime();
        printf("[probe] MIME='%s'\n", m ? m : "(null)");
    }
    dlclose(h);
    printf("[probe] PASS\n");
    return 0;
}
