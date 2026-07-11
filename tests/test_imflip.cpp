/*
 * Test: imflip — horizontal, vertical, and H+V flip
 *
 * Uses a 100×100 pattern: left half red, right half blue,
 * top row green. Verifies pixel positions after each flip mode.
 */

#include "test_utils.h"

int main() {
    printf("=== test_imflip ===\n");

    const int W = 100, H = 100;

    TestBuffer src;
    if (!src.alloc(W, H, RK_FORMAT_RGBA_8888))
        return 77;

    /* Fill pattern:
     *   left half  (x=0..49):  red
     *   right half (x=50..99): blue
     *   top row    (y=0):      green  (overrides background) */
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            uint32_t color;
            if (y == 0)
                color = 0xFF00FF00;
            else if (x < W / 2)
                color = 0xFFFF0000;
            else
                color = 0xFF0000FF;
            uint8_t *p = src.ptr + (y * W + x) * 4;
            p[0] = (color >> 16) & 0xFF;  /* R */
            p[1] = (color >> 8)  & 0xFF;  /* G */
            p[2] = (color >> 0)  & 0xFF;  /* B */
            p[3] = (color >> 24) & 0xFF;  /* A */
        }
    }

    RgaImport src_imp;
    if (!src_imp.import(src.fd, (int)src.size)) {
        src.free();
        return g_test_failures;
    }
    rga_buffer_t src_img = src.wrap(src_imp.handle);

    /* -------------------------------------------------------------- */
    /*  Test A: Horizontal flip                                        */
    /* -------------------------------------------------------------- */
    {
        TestBuffer dst;
        if (dst.alloc(W, H, RK_FORMAT_RGBA_8888)) {
            RgaImport dst_imp;
            if (dst_imp.import(dst.fd, (int)dst.size)) {
                rga_buffer_t dst_img = dst.wrap(dst_imp.handle);
                IM_STATUS st = imflip(src_img, dst_img,
                                      IM_HAL_TRANSFORM_FLIP_H);
                TEST_CHECK(st == IM_STATUS_SUCCESS,
                           "imflip H returned %s", imStrError(st));
                if (st == IM_STATUS_SUCCESS) {
                    TEST_PASS("imflip horizontal completed");
                    /* H-flip: (x,y) → (W-1-x, y) */
                    check_pixel(dst.ptr, W, 89, 50,
                                0xFF,0x00,0x00,0xFF, "flipH-red");
                    check_pixel(dst.ptr, W, 39, 50,
                                0x00,0x00,0xFF,0xFF, "flipH-blue");
                    check_pixel(dst.ptr, W, 89, 0,
                                0x00,0xFF,0x00,0xFF, "flipH-green");
                }
                dst_imp.release();
            }
            dst.free();
        }
    }

    /* -------------------------------------------------------------- */
    /*  Test B: Vertical flip                                          */
    /* -------------------------------------------------------------- */
    {
        TestBuffer dst;
        if (dst.alloc(W, H, RK_FORMAT_RGBA_8888)) {
            RgaImport dst_imp;
            if (dst_imp.import(dst.fd, (int)dst.size)) {
                rga_buffer_t dst_img = dst.wrap(dst_imp.handle);
                IM_STATUS st = imflip(src_img, dst_img,
                                      IM_HAL_TRANSFORM_FLIP_V);
                TEST_CHECK(st == IM_STATUS_SUCCESS,
                           "imflip V returned %s", imStrError(st));
                if (st == IM_STATUS_SUCCESS) {
                    TEST_PASS("imflip vertical completed");
                    /* V-flip: (x,y) → (x, H-1-y) */
                    check_pixel(dst.ptr, W, 10, 49,
                                0xFF,0x00,0x00,0xFF, "flipV-red");
                    check_pixel(dst.ptr, W, 60, 49,
                                0x00,0x00,0xFF,0xFF, "flipV-blue");
                    check_pixel(dst.ptr, W, 50, 99,
                                0x00,0xFF,0x00,0xFF, "flipV-green");
                }
                dst_imp.release();
            }
            dst.free();
        }
    }

    /* -------------------------------------------------------------- */
    /*  Test C: H + V flip                                             */
    /* -------------------------------------------------------------- */
    {
        TestBuffer dst;
        if (dst.alloc(W, H, RK_FORMAT_RGBA_8888)) {
            RgaImport dst_imp;
            if (dst_imp.import(dst.fd, (int)dst.size)) {
                rga_buffer_t dst_img = dst.wrap(dst_imp.handle);
                IM_STATUS st = imflip(src_img, dst_img,
                                      IM_HAL_TRANSFORM_FLIP_H_V);
                TEST_CHECK(st == IM_STATUS_SUCCESS,
                           "imflip H_V returned %s", imStrError(st));
                if (st == IM_STATUS_SUCCESS) {
                    TEST_PASS("imflip H+V completed");
                    /* H+V: (x,y) → (W-1-x, H-1-y) */
                    check_pixel(dst.ptr, W, 89, 49,
                                0xFF,0x00,0x00,0xFF, "flipHV-red");
                    check_pixel(dst.ptr, W, 39, 49,
                                0x00,0x00,0xFF,0xFF, "flipHV-blue");
                    check_pixel(dst.ptr, W, 50, 99,
                                0x00,0xFF,0x00,0xFF, "flipHV-green");
                }
                dst_imp.release();
            }
            dst.free();
        }
    }

    src_imp.release();
    src.free();

    printf("test_imflip: %d failures\n", g_test_failures);
    return g_test_failures;
}
