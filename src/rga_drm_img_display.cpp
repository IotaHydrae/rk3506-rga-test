#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "rga_drm_img_copy"

#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/stddef.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "RgaUtils.h"
#include "drm_alloc.h"
#include "im2d.hpp"
#include "utils.h"

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <drm_fourcc.h>

#define LOCAL_FILE_PATH "/data"

class DrmObject {
    public:
	int drm_buffer_fd;
	int drm_buffer_handle;
	size_t actual_size;
	uint8_t *drm_buf;
};

static int drm_to_rga_format(uint32_t fourcc)
{
	switch (fourcc) {
	case DRM_FORMAT_ARGB8888:
		return RK_FORMAT_BGRA_8888;
	case DRM_FORMAT_XRGB8888:
		return RK_FORMAT_BGRX_8888;
	case DRM_FORMAT_ABGR8888:
		return RK_FORMAT_RGBA_8888;
	case DRM_FORMAT_XBGR8888:
		return RK_FORMAT_RGBX_8888;
	case DRM_FORMAT_RGB565:
		return RK_FORMAT_RGB_565;
	case DRM_FORMAT_BGR565:
		return RK_FORMAT_BGR_565;
	default:
		printf("Unsupported DRM format: %08x\n", fourcc);
		return -1;
	}

	return 0;
}

int main()
{
	int drm_fd = open("/dev/dri/card0", O_RDWR);
	if (drm_fd < 0) {
		perror("open drm");
		return -1;
	}

	drmModeRes *res = drmModeGetResources(drm_fd);

	printf("crtcs      : %d\n", res->count_crtcs);
	printf("connectors : %d\n", res->count_connectors);
	printf("encoders   : %d\n", res->count_encoders);

	drmModeCrtc *crtc = drmModeGetCrtc(drm_fd, 71);

	printf("fb=%u\n", crtc->buffer_id);

	drmModeFB2 *fb2 = drmModeGetFB2(drm_fd, crtc->buffer_id);

	printf("width  = %u\n", fb2->width);
	printf("height = %u\n", fb2->height);
	printf("format = %08x\n", fb2->pixel_format);

	printf("handle = %u\n", fb2->handles[0]);
	printf("pitch  = %u\n", fb2->pitches[0]);
	printf("offset = %u\n", fb2->offsets[0]);

	int rga_fmt = drm_to_rga_format(fb2->pixel_format);

	int prime_fd;
	int ret = drmPrimeHandleToFD(drm_fd, fb2->handles[0], DRM_CLOEXEC,
				     &prime_fd);

	if (ret) {
		perror("drmPrimeHandleToFD");
		return -1;
	}

	printf("prime fd = %d\n", prime_fd);

	int src_width, src_height, src_format;
	int dst_width, dst_height, dst_format;
	long src_phy, dst_phy;
	int src_alloc_flags = 0, dst_alloc_flags = 0;

	int src_buf_size, dst_buf_size;
	DrmObject drm_src;
	DrmObject drm_dst;

	rga_buffer_t src_img, dst_img, fb_img;
	rga_buffer_handle_t src_handle, dst_handle, fb_handle;

	memset(&src_img, 0, sizeof(src_img));
	src_width = 480;
	src_height = 800;
	src_format = RK_FORMAT_RGBA_8888;

	src_buf_size = src_width * src_height * get_bpp_from_format(src_format);
	src_alloc_flags |= RGA_UTILS_ROCKCHIP_BO_CONTIG;

	/*
	* Allocate a physically continuous drm_buf, return dma_fd and
	* virtual address, and get the physical address.
	*/
	drm_src.drm_buf = (uint8_t *)drm_buf_alloc(
		src_width, src_height, get_bpp_from_format(src_format) * 8,
		&drm_src.drm_buffer_fd, &drm_src.drm_buffer_handle,
		&drm_src.actual_size, src_alloc_flags);

	src_phy = drm_buf_get_phy(drm_src.drm_buffer_handle);
	/* fill image data */
	if (0 != read_image_from_file(drm_src.drm_buf, LOCAL_FILE_PATH,
				      src_width, src_height, src_format, 0)) {
		printf("src image read err\n");
		draw_rgba((char *)drm_src.drm_buf, src_width, src_height);
	}

	src_handle = importbuffer_fd(drm_src.drm_buffer_fd, src_buf_size);
	fb_handle = importbuffer_fd(prime_fd, fb2->pitches[0] * fb2->height);

	src_img = wrapbuffer_handle(src_handle, src_width, src_height,
				    src_format);
	fb_img = wrapbuffer_handle(fb_handle, fb2->width, fb2->height, rga_fmt);

	ret = imcheck(src_img, fb_img, {}, {});
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s\n", __LINE__,
		       imStrError((IM_STATUS)ret));
		return -1;
	}

	ret = imcopy(src_img, fb_img);

release_buffer:
	if (src_handle)
		releasebuffer_handle(src_handle);
	if (fb_handle)
		releasebuffer_handle(fb_handle);

drm_buf_destroy:
	drm_buf_destroy(drm_src.drm_buffer_fd, drm_src.drm_buffer_handle,
			drm_src.drm_buf, drm_src.actual_size);

out:
	close(drm_fd);
	return 0;
}