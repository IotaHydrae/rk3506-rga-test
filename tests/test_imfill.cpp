/*
 * Test: imfill / imrectangle — color fill and rectangle drawing
 *
 * Tests:
 *   A. Full-buffer fill with solid red
 *   B. Partial rectangle fill with green
 *   C. Rectangle outline drawing
 */

#include "test_utils.h"

int main() {
    printf("=== test_imfill ===\n");

    const int W = 480, H = 200;

    TestBuffer buf;
    if (!buf.alloc(W, H, RK_FORMAT_RGBA_8888))
        return 77;

    RgaImport imp;
    if (!imp.import(buf.fd, (int)buf.size)) {
        buf.free();
        return g_test_failures;
    }
    rga_buffer_t img = buf.wrap(imp.handle);

    /* -------------------------------------------------------------- */
    /*  Test A: full-buffer fill — solid red                           */
    /* -------------------------------------------------------------- */
    /* RGA color format is 0xAABBGGRR (little-endian):                 */
    /*   0xFF0000FF = red, 0xFF00FF00 = green, 0xFFFF0000 = blue       */
    IM_STATUS st = imfill(img, {0, 0, W, H}, 0xFF0000FF);
    TEST_CHECK(st == IM_STATUS_SUCCESS,
               "imfill(full) returned %s", imStrError(st));
    if (st == IM_STATUS_SUCCESS) {
        TEST_PASS("imfill full-buffer red");
        check_rect_filled(buf.ptr, buf.stride_px(), W, H,
                          {0, 0, W, H}, 0xFF0000FF, "full-red");
    }

    /* -------------------------------------------------------------- */
    /*  Test B: partial rectangle fill — green sub-rect                */
    /* -------------------------------------------------------------- */
    {
        im_rect r = {10, 10, 100, 50};
        st = imfill(img, r, 0xFF00FF00);
        TEST_CHECK(st == IM_STATUS_SUCCESS,
                   "imfill(partial) returned %s", imStrError(st));
        if (st == IM_STATUS_SUCCESS) {
            TEST_PASS("imfill partial green rect");
            check_rect_filled(buf.ptr, buf.stride_px(), W, H,
                              r, 0xFF00FF00, "partial-green-inside");
            check_rect_not_filled(buf.ptr, buf.stride_px(),
                                  r, 0xFF00FF00, "partial-green-outside");
        }
    }

    /* -------------------------------------------------------------- */
    /*  Test C: rectangle outline                                      */
    /* -------------------------------------------------------------- */
    {
        im_rect r = {50, 50, 200, 100};
        /* 0xFFFF0000 = blue in 0xAABBGGRR format */
        st = imrectangle(img, r, 0xFFFF0000, 2);
        TEST_CHECK(st == IM_STATUS_SUCCESS,
                   "imrectangle returned %s", imStrError(st));
        if (st == IM_STATUS_SUCCESS) {
            TEST_PASS("imrectangle outline blue");
            /* top edge (thickness=2 → rows 50,51).
             * imrectangle writes BGRA; get_pixel_rgba reads p[0]=B as R, p[2]=R as B.
             * Outline color 0xFFFF0000 (AABBGGRR: blue) → BGRA mem [0xFF,0x00,0x00,0xFF]
             * → get_pixel_rgba returns R=0xFF, G=0x00, B=0x00. */
            check_pixel(buf.ptr, buf.stride_px(), 60, 50,
                        0xFF,0x00,0x00,0xFF, "rect-top-edge");
            check_pixel(buf.ptr, buf.stride_px(), 60, 51,
                        0xFF,0x00,0x00,0xFF, "rect-top-edge+1");
            /* interior — should NOT be the outline color (as read by get_pixel_rgba) */
            uint8_t pr, pg, pb, pa;
            get_pixel_rgba(buf.ptr, buf.stride_px(), 80, 80,
                           &pr, &pg, &pb, &pa);
            TEST_CHECK(!(pr == 0xFF && pg == 0x00 && pb == 0x00 && pa == 0xFF),
                       "rectangle interior should not be overwritten by outline");
            TEST_PASS("imrectangle interior not overwritten");
        }
    }

    imp.release();
    buf.free();

    printf("test_imfill: %d failures\n", g_test_failures);
    return g_test_failures;
}
