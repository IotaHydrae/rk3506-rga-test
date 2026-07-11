/*
 * Test: imrotate — rotation 90°/180°/270°
 *
 * Uses an asymmetric 100×50 pattern with red square at top-left
 * and blue square at bottom-right. Verifies corner positions after
 * each rotation.
 */

#include "test_utils.h"

int main() {
    printf("=== test_imrotate ===\n");

    const int SW = 100, SH = 50;

    TestBuffer src;
    if (!src.alloc(SW, SH, RK_FORMAT_RGBA_8888))
        return 77;

    /* Fill asymmetric pattern:
     *   top-left  10×10 square: red
     *   bottom-right 10×10 square: blue
     *   rest: white */
    for (int y = 0; y < SH; y++) {
        for (int x = 0; x < SW; x++) {
            uint32_t color;
            if (x < 10 && y < 10)
                color = 0xFFFF0000;
            else if (x >= SW - 10 && y >= SH - 10)
                color = 0xFF0000FF;
            else
                color = 0xFFFFFFFF;
            uint8_t *p = src.ptr + (y * SW + x) * 4;
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
    /*  Test A: Rotate 90°  (100×50 → 50×100)                         */
    /* -------------------------------------------------------------- */
    {
        const int DW = SH, DH = SW;
        TestBuffer dst;
        if (dst.alloc(DW, DH, RK_FORMAT_RGBA_8888)) {
            RgaImport dst_imp;
            if (dst_imp.import(dst.fd, (int)dst.size)) {
                rga_buffer_t dst_img = dst.wrap(dst_imp.handle);
                IM_STATUS st = imrotate(src_img, dst_img,
                                        IM_HAL_TRANSFORM_ROT_90);
                TEST_CHECK(st == IM_STATUS_SUCCESS,
                           "imrotate 90 returned %s", imStrError(st));
                if (st == IM_STATUS_SUCCESS) {
                    TEST_PASS("imrotate 90° completed");
                    check_pixel(dst.ptr, DW, 45, 5,
                                0xFF,0x00,0x00,0xFF, "rot90-red");
                    check_pixel(dst.ptr, DW, 5, 95,
                                0x00,0x00,0xFF,0xFF, "rot90-blue");
                }
                dst_imp.release();
            }
            dst.free();
        }
    }

    /* -------------------------------------------------------------- */
    /*  Test B: Rotate 180°  (100×50 stays 100×50)                     */
    /* -------------------------------------------------------------- */
    {
        const int DW = SW, DH = SH;
        TestBuffer dst;
        if (dst.alloc(DW, DH, RK_FORMAT_RGBA_8888)) {
            RgaImport dst_imp;
            if (dst_imp.import(dst.fd, (int)dst.size)) {
                rga_buffer_t dst_img = dst.wrap(dst_imp.handle);
                IM_STATUS st = imrotate(src_img, dst_img,
                                        IM_HAL_TRANSFORM_ROT_180);
                TEST_CHECK(st == IM_STATUS_SUCCESS,
                           "imrotate 180 returned %s", imStrError(st));
                if (st == IM_STATUS_SUCCESS) {
                    TEST_PASS("imrotate 180° completed");
                    check_pixel(dst.ptr, DW, 95, 45,
                                0xFF,0x00,0x00,0xFF, "rot180-red");
                    check_pixel(dst.ptr, DW, 5, 5,
                                0x00,0x00,0xFF,0xFF, "rot180-blue");
                }
                dst_imp.release();
            }
            dst.free();
        }
    }

    /* -------------------------------------------------------------- */
    /*  Test C: Rotate 270°  (100×50 → 50×100)                        */
    /* -------------------------------------------------------------- */
    {
        const int DW = SH, DH = SW;
        TestBuffer dst;
        if (dst.alloc(DW, DH, RK_FORMAT_RGBA_8888)) {
            RgaImport dst_imp;
            if (dst_imp.import(dst.fd, (int)dst.size)) {
                rga_buffer_t dst_img = dst.wrap(dst_imp.handle);
                IM_STATUS st = imrotate(src_img, dst_img,
                                        IM_HAL_TRANSFORM_ROT_270);
                TEST_CHECK(st == IM_STATUS_SUCCESS,
                           "imrotate 270 returned %s", imStrError(st));
                if (st == IM_STATUS_SUCCESS) {
                    TEST_PASS("imrotate 270° completed");
                    check_pixel(dst.ptr, DW, 5, 95,
                                0xFF,0x00,0x00,0xFF, "rot270-red");
                    check_pixel(dst.ptr, DW, 45, 5,
                                0x00,0x00,0xFF,0xFF, "rot270-blue");
                }
                dst_imp.release();
            }
            dst.free();
        }
    }

    src_imp.release();
    src.free();

    printf("test_imrotate: %d failures\n", g_test_failures);
    return g_test_failures;
}
