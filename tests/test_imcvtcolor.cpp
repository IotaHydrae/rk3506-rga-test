/*
 * Test: imcvtcolor — color space / format conversion
 *
 * Tests:
 *   A. RGBA8888 → BGRA8888  (R/B channel swap)
 *   B. RGBA8888 → YCbCr420SP (BT.601 full-range)
 */

#include <cmath>
#include "test_utils.h"

int main() {
    printf("=== test_imcvtcolor ===\n");

    /* -------------------------------------------------------------- */
    /*  Test A: RGBA → BGRA  (channel swap)                           */
    /* -------------------------------------------------------------- */
    {
        const int W = 64, H = 64;

        TestBuffer src;
        TestBuffer dst;
        if (!src.alloc(W, H, RK_FORMAT_RGBA_8888) ||
            !dst.alloc(W, H, RK_FORMAT_BGRA_8888)) {
            src.free();
            dst.free();
            /* if first alloc fails => return SKIP, else test failure */
            return (g_test_failures == 0) ? 77 : g_test_failures;
        }

        /* fill src with known pixels: R=x, G=y, B=128, A=255 */
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                uint8_t *p = src.ptr + (y * W + x) * 4;
                p[0] = (uint8_t)(x);
                p[1] = (uint8_t)(y);
                p[2] = (uint8_t)(128);
                p[3] = 0xFF;
            }
        }

        RgaImport src_imp, dst_imp;
        if (src_imp.import(src.fd, (int)src.size) &&
            dst_imp.import(dst.fd, (int)dst.size)) {
            rga_buffer_t src_img = src.wrap(src_imp.handle);
            rga_buffer_t dst_img = dst.wrap(dst_imp.handle);

            IM_STATUS st = imcvtcolor(src_img, dst_img,
                                      RK_FORMAT_RGBA_8888,
                                      RK_FORMAT_BGRA_8888);
            TEST_CHECK(st == IM_STATUS_SUCCESS,
                       "imcvtcolor RGBA→BGRA returned %s", imStrError(st));
            if (st == IM_STATUS_SUCCESS) {
                TEST_PASS("imcvtcolor RGBA→BGRA completed");
                for (int y = 0; y < H; y += 16) {
                    for (int x = 0; x < W; x += 16) {
                        uint8_t pr, pg, pb, pa;
                        get_pixel_rgba(dst.ptr, dst.stride_px(), x, y,
                                       &pr, &pg, &pb, &pa);
                        TEST_CHECK(pr == (uint8_t)(128) &&
                                   pg == (uint8_t)(y)   &&
                                   pb == (uint8_t)(x)   &&
                                   pa == 0xFF,
                                   "RGBA→BGRA pixel(%d,%d) mismatch", x, y);
                    }
                }
                TEST_PASS("imcvtcolor RGBA→BGRA channel swap verified");
            }
        }
        src_imp.release();
        dst_imp.release();
        src.free();
        dst.free();
    }

    /* -------------------------------------------------------------- */
    /*  Test B: RGBA → YCbCr420SP                                     */
    /* -------------------------------------------------------------- */
    {
        const int W = 64, H = 64;

        TestBuffer src;
        TestBuffer dst;
        if (!src.alloc(W, H, RK_FORMAT_RGBA_8888) ||
            !dst.alloc(W, H, RK_FORMAT_YCbCr_420_SP)) {
            src.free();
            dst.free();
            return (g_test_failures == 0) ? 77 : g_test_failures;
        }

        /* fill src with solid grey (128,128,128) */
        for (int i = 0; i < W * H; i++) {
            uint8_t *p = src.ptr + i * 4;
            p[0] = 128; p[1] = 128; p[2] = 128; p[3] = 255;
        }

        RgaImport src_imp, dst_imp;
        if (src_imp.import(src.fd, (int)src.size) &&
            dst_imp.import(dst.fd, (int)dst.size)) {
            rga_buffer_t src_img = src.wrap(src_imp.handle);
            rga_buffer_t dst_img = dst.wrap(dst_imp.handle);

            IM_STATUS st = imcvtcolor(src_img, dst_img,
                                      RK_FORMAT_RGBA_8888,
                                      RK_FORMAT_YCbCr_420_SP,
                                      IM_RGB_TO_YUV_BT601_FULL);
            TEST_CHECK(st == IM_STATUS_SUCCESS,
                       "imcvtcolor RGBA→YUV420 returned %s", imStrError(st));
            if (st == IM_STATUS_SUCCESS) {
                TEST_PASS("imcvtcolor RGBA→YUV420 completed");

                uint8_t *dst_y   = dst.ptr;
                uint8_t *dst_uv  = dst.ptr + W * H;
                int      expected_y = (int)(0.299 * 128 + 0.587 * 128 +
                                            0.114 * 128);

                for (int i = 0; i < 10; i++) {
                    int x = i * 6 + 2, y = i * 6 + 2;
                    uint8_t y_val = dst_y[y * W + x];
                    TEST_CHECK(std::abs((int)y_val - expected_y) <= 2,
                               "Y plane (%d,%d): expected ~%d got %d",
                               x, y, expected_y, y_val);
                }
                TEST_PASS("imcvtcolor Y plane within tolerance");

                uint8_t uv_u = dst_uv[0], uv_v = dst_uv[1];
                TEST_CHECK(std::abs((int)uv_u - 128) <= 2,
                           "Cb expected ~128 got %d", uv_u);
                TEST_CHECK(std::abs((int)uv_v - 128) <= 2,
                           "Cr expected ~128 got %d", uv_v);
                TEST_PASS("imcvtcolor UV plane at neutral grey");
            }
        }
        src_imp.release();
        dst_imp.release();
        src.free();
        dst.free();
    }

    printf("test_imcvtcolor: %d failures\n", g_test_failures);
    return g_test_failures;
}
