/*
 * TinyGL Dithering API Extension
 *
 * Custom extensions for controlling ordered dithering at runtime
 */

#ifndef _tgl_gl_dither_h_
#define _tgl_gl_dither_h_

#ifdef __cplusplus
extern "C" {
#endif

/* Custom TinyGL extension: Enable/disable dithering at runtime */
void glSetDithering(GLboolean enabled);

/* Custom TinyGL extension: Get current dithering state */
GLboolean glGetDithering(void);

#ifdef __cplusplus
}
#endif

#endif /* _tgl_gl_dither_h_ */