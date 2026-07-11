/*
 * Test: imcopy — basic RGA buffer copy
 *
 * Allocates two DRM buffers, fills src with draw_rgba() stripes,
 * copies src→dst via imcopy, then validates pixel values at
 * known stripe positions.
 */

#include "test_utils.h"

int main() {
    printf("=== test_imcopy ===\n");

    /* ---- allocate buffers ---- */
    const int W = 480, H = 800;

    TestBuffer src, dst;
    if (!src.alloc(W, H, RK_FORMAT_RGBA_8888) ||
        !dst.alloc(W, H, RK_FORMAT_RGBA_8888)) {
        src.free();
        dst.free();
        return (g_test_failures == 0) ? 77 : g_test_failures;
    }

    /* ---- fill source with stripe pattern ---- */
    draw_rgba((char *)src.ptr, W, H);

    /* ---- import & wrap ---- */
    RgaImport src_import, dst_import;
    if (src_import.import(src.fd, (int)src.size) &&
        dst_import.import(dst.fd, (int)dst.size)) {

        rga_buffer_t src_img = src.wrap(src_import.handle);
        rga_buffer_t dst_img = dst.wrap(dst_import.handle);

        /* ---- validate params ---- */
        IM_STATUS st = imcheck(src_img, dst_img, {}, {});
        TEST_CHECK(st == IM_STATUS_NOERROR,
                   "imcheck returned %s (expected NOERROR)",
                   imStrError(st));

        /* ---- copy ---- */
        st = imcopy(src_img, dst_img);
        TEST_CHECK(st == IM_STATUS_SUCCESS,
                   "imcopy returned %s (expected SUCCESS)",
                   imStrError(st));
        if (st == IM_STATUS_SUCCESS) {
            TEST_PASS("imcopy completed successfully");

            /* ---- verify stripes ---- */
            /* draw_rgba() creates 4 vertical stripes:
             *   [0] red   x=0..119    [1] green x=120..239
             *   [2] blue  x=240..359  [3] white x=360..479 */
            struct { int x; uint8_t r,g,b,a; const char *name; } samples[] = {
                { 60,  0xFF,0x00,0x00,0xFF, "stripe-0-red"   },
                {180,  0x00,0xFF,0x00,0xFF, "stripe-1-green" },
                {300,  0x00,0x00,0xFF,0xFF, "stripe-2-blue"  },
                {420,  0xFF,0xFF,0xFF,0xFF, "stripe-3-white" },
            };

            for (auto &s : samples) {
                check_pixel(dst.ptr, dst.stride_px(),
                            s.x, 10,  s.r,s.g,s.b,s.a, s.name);
                check_pixel(dst.ptr, dst.stride_px(),
                            s.x, 400, s.r,s.g,s.b,s.a, s.name);
                check_pixel(dst.ptr, dst.stride_px(),
                            s.x, 790, s.r,s.g,s.b,s.a, s.name);
            }
        }
    }

    src_import.release();
    dst_import.release();
    src.free();
    dst.free();

    printf("test_imcopy: %d failures\n", g_test_failures);
    return g_test_failures;
}
