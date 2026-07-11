/*
 * Copyright (C) 2024 Rockchip Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __RGA_TESTS_TEST_UTILS_H__
#define __RGA_TESTS_TEST_UTILS_H__

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "drm_alloc.h"
#include "im2d.hpp"
#include "RgaUtils.h"
#include "utils.h"

/* ------------------------------------------------------------------ */
/*  test macros                                                       */
/* ------------------------------------------------------------------ */

static int g_test_failures = 0;

#define TEST_CHECK(condition, format, ...)                             \
    do {                                                               \
        if (!(condition)) {                                            \
            printf("  FAIL at %s:%d: " format "\n",                    \
                   __FILE__, __LINE__, ##__VA_ARGS__);                 \
            g_test_failures++;                                         \
        }                                                              \
    } while (0)

#define TEST_PASS(format, ...)                                         \
    printf("  PASS: " format "\n", ##__VA_ARGS__)

/* ------------------------------------------------------------------ */
/*  RAII DRM buffer wrapper                                           */
/* ------------------------------------------------------------------ */

struct TestBuffer {
    int      fd     = -1;
    int      handle = 0;
    size_t   size   = 0;
    uint8_t *ptr    = nullptr;
    int      width  = 0;
    int      height = 0;
    int      format = RK_FORMAT_RGBA_8888;

    bool alloc(int w, int h, int fmt, int flags = RGA_UTILS_ROCKCHIP_BO_CONTIG) {
        width  = w;
        height = h;
        format = fmt;
        int bpp = (int)(get_bpp_from_format(fmt) * 8);
        ptr     = (uint8_t *)drm_buf_alloc(w, h, bpp, &fd, &handle, &size, flags);
        if (!ptr) {
            printf("SKIP: %s (no /dev/dri/card0 or allocation failed)\n", __FILE__);
            return false;
        }
        return true;
    }

    void free() {
        if (ptr) {
            drm_buf_destroy(fd, handle, ptr, size);
            ptr    = nullptr;
            fd     = -1;
            handle = 0;
            size   = 0;
        }
    }

    /* wrap into an rga_buffer_t using an already-imported handle */
    rga_buffer_t wrap(rga_buffer_handle_t rga_handle) const {
        return wrapbuffer_handle(rga_handle, width, height, format);
    }

    /* stride in pixels (== width for contiguous buffers) */
    int stride_px() const { return width; }
};

/* ------------------------------------------------------------------ */
/*  RAII RGA import wrapper                                           */
/* ------------------------------------------------------------------ */

struct RgaImport {
    rga_buffer_handle_t handle = 0;

    bool import(int dma_fd, int buf_size) {
        handle = importbuffer_fd(dma_fd, buf_size);
        if (!handle) {
            printf("  FAIL: importbuffer_fd returned NULL\n");
            g_test_failures++;
            return false;
        }
        return true;
    }

    void release() {
        if (handle) {
            releasebuffer_handle(handle);
            handle = 0;
        }
    }
};

/* ------------------------------------------------------------------ */
/*  pixel helpers  (RGBA8888 buffers)                                  */
/* ------------------------------------------------------------------ */

inline void get_pixel_rgba(const uint8_t *buf, int stride_px,
                           int x, int y,
                           uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a) {
    const uint8_t *p = buf + (y * stride_px + x) * 4;
    *r = p[0]; *g = p[1]; *b = p[2]; *a = p[3];
}

inline uint32_t get_pixel_u32(const uint8_t *buf, int stride_px, int x, int y) {
    const uint8_t *p = buf + (y * stride_px + x) * 4;
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

inline bool check_pixel(const uint8_t *buf, int stride_px,
                        int x, int y,
                        uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                        const char *label) {
    uint8_t pr, pg, pb, pa;
    get_pixel_rgba(buf, stride_px, x, y, &pr, &pg, &pb, &pa);
    if (pr != r || pg != g || pb != b || pa != a) {
        printf("  FAIL at %s: pixel(%d,%d) expected %02x%02x%02x%02x "
               "got %02x%02x%02x%02x\n",
               label, x, y, r, g, b, a, pr, pg, pb, pa);
        g_test_failures++;
        return false;
    }
    return true;
}

inline bool check_rect_filled(const uint8_t *buf, int stride_px,
                              int width, int height,
                              im_rect rect, uint32_t color,
                              const char *label) {
    bool ok = true;
    /* imfill writes 0xAABBGGRR → BGRA memory [BB,GG,RR,AA].
     * get_pixel_rgba reads p[0] as R (=BB), p[2] as B (=RR).
     * So expected R = color's BB, expected B = color's RR. */
    uint8_t er = (color >> 16) & 0xFF;  /* BB → p[0] → read as R */
    uint8_t eg = (color >> 8)  & 0xFF;  /* GG → p[1] → read as G */
    uint8_t eb = (color >> 0)  & 0xFF;  /* RR → p[2] → read as B */
    uint8_t ea = (color >> 24) & 0xFF;  /* AA → p[3] → read as A */

    /* sample a grid of points inside the rect */
    int step_x = (rect.width  > 4) ? rect.width  / 4 : 1;
    int step_y = (rect.height > 4) ? rect.height / 4 : 1;
    for (int y = rect.y; y < rect.y + rect.height && ok; y += step_y) {
        for (int x = rect.x; x < rect.x + rect.width && ok; x += step_x) {
            if (!check_pixel(buf, stride_px, x, y, er, eg, eb, ea, label))
                ok = false;
        }
    }
    /* also check corners */
    if (ok && rect.width > 1 && rect.height > 1) {
        ok = check_pixel(buf, stride_px,
                         rect.x + rect.width  - 1,
                         rect.y + rect.height - 1,
                         er, eg, eb, ea, label);
    }
    return ok;
}

inline bool check_rect_not_filled(const uint8_t *buf, int stride_px,
                                  im_rect rect, uint32_t color,
                                  const char *label) {
    /* imfill writes 0xAABBGGRR → BGRA memory [BB,GG,RR,AA].
     * get_pixel_rgba reads p[0] as R (=BB), p[2] as B (=RR). */
    uint8_t er = (color >> 16) & 0xFF;  /* BB → p[0] → read as R */
    uint8_t eg = (color >> 8)  & 0xFF;  /* GG → p[1] → read as G */
    uint8_t eb = (color >> 0)  & 0xFF;  /* RR → p[2] → read as B */
    uint8_t ea = (color >> 24) & 0xFF;  /* AA → p[3] → read as A */

    uint8_t pr, pg, pb, pa;
    /* check one pixel outside each edge */
    if (rect.x > 0) {
        get_pixel_rgba(buf, stride_px, rect.x - 1, rect.y, &pr, &pg, &pb, &pa);
        if (pr == er && pg == eg && pb == eb && pa == ea) {
            printf("  FAIL at %s: pixel(%d,%d) unexpectedly matches fill color\n",
                   label, rect.x - 1, rect.y);
            g_test_failures++;
            return false;
        }
    }
    if (rect.y > 0) {
        get_pixel_rgba(buf, stride_px, rect.x, rect.y - 1, &pr, &pg, &pb, &pa);
        if (pr == er && pg == eg && pb == eb && pa == ea) {
            printf("  FAIL at %s: pixel(%d,%d) unexpectedly matches fill color\n",
                   label, rect.x, rect.y - 1);
            g_test_failures++;
            return false;
        }
    }
    return true;
}

#endif /* __RGA_TESTS_TEST_UTILS_H__ */
