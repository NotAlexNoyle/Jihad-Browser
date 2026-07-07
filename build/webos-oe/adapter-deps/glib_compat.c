/* glib compat shim for the Jihad adapter.
 * The pbnjson C layer is compiled against jessie glib 2.42 headers whose g_new0()
 * macro expands to g_malloc0_n() (added in glib 2.24). The TouchPad ships glib
 * 2.16.6, which lacks it. Provide it here (hidden visibility, internal to the .so)
 * so the plugin loads on-device. Same overflow-checked semantics as glib's own. */
#include <glib.h>

__attribute__((visibility("hidden")))
gpointer g_malloc0_n(gsize n_blocks, gsize n_block_bytes)
{
	if (n_blocks != 0 && n_block_bytes > G_MAXSIZE / n_blocks)
		g_error("g_malloc0_n: overflow");
	return g_malloc0(n_blocks * n_block_bytes);
}

/* Device glib 2.16 provides g_atomic_int_inc as a header macro over the extern glib
 * function; jessie headers + gcc-4.3.3 emit an extern reference the device glib does
 * not export. But the device DOES export the underlying atomic primitive
 * g_atomic_int_exchange_and_add (atomically adds and returns the previous value), so
 * implement inc as "add 1" on top of it. We declare it ourselves and #undef the header
 * macro so this compiles as a plain function call — no assembly. */
extern gint g_atomic_int_exchange_and_add(volatile gint *atomic, gint val);
#undef g_atomic_int_inc
__attribute__((visibility("hidden")))
void g_atomic_int_inc(volatile gint *atomic)
{
	g_atomic_int_exchange_and_add(atomic, 1);
}
