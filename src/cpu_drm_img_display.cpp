#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "cpu_drm_img_copy"

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

#include <drm.h>
#include <drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

static void rgba_to_bgra(uint32_t *buf, int width, int height)
{
	int pixels = width * height;

	for (int i = 0; i < pixels; i++) {
		uint32_t c = buf[i];

		buf[i] = (c & 0xFF00FF00) | ((c & 0x00FF0000) >> 16) |
			 ((c & 0x000000FF) << 16);
	}
}

int main()
{
	int ret = 0;
	int drm_fd = -1;
	drmModeRes *res = NULL;
	drmModeCrtc *crtc = NULL;
	drmModeFB2 *fb2 = NULL;
	void *fb_mem = MAP_FAILED;
	struct drm_mode_map_dumb map = {};

	const char *src_file_path = "/data/in0w480-h800-rgba8888.bin";
#define SRC_FILE_SIZE 480 * 800 * 4
	uint8_t src_file_buf[SRC_FILE_SIZE];
	FILE *fp = NULL;

	drm_fd = open("/dev/dri/card0", O_RDWR);
	if (drm_fd < 0) {
		perror("open drm");
		return -1;
	}

	res = drmModeGetResources(drm_fd);
	if (!res) {
		fprintf(stderr, "drmModeGetResources failed\n");
		ret = -1;
		goto cleanup;
	}

	printf("crtcs      : %d\n", res->count_crtcs);
	printf("connectors : %d\n", res->count_connectors);
	printf("encoders   : %d\n", res->count_encoders);

	crtc = drmModeGetCrtc(drm_fd, 71);
	if (!crtc) {
		fprintf(stderr, "drmModeGetCrtc failed\n");
		ret = -1;
		goto cleanup;
	}

	printf("fb=%u\n", crtc->buffer_id);

	fb2 = drmModeGetFB2(drm_fd, crtc->buffer_id);
	if (!fb2) {
		fprintf(stderr, "drmModeGetFB2 failed\n");
		ret = -1;
		goto cleanup;
	}

	printf("width  = %u\n", fb2->width);
	printf("height = %u\n", fb2->height);
	printf("format = %08x\n", fb2->pixel_format);
	printf("DRM_FORMAT_XRGB8888 = %08x\n", DRM_FORMAT_XRGB8888);

	printf("handle = %u\n", fb2->handles[0]);
	printf("pitch  = %u\n", fb2->pitches[0]);
	printf("offset = %u\n", fb2->offsets[0]);

	map.handle = fb2->handles[0];

	ret = ioctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map);
	if (ret) {
		perror("DRM_IOCTL_MODE_MAP_DUMB");
		goto cleanup;
	}

	printf("map.offset = %lld\n", map.offset);

	fb_mem = mmap(NULL, fb2->pitches[0] * fb2->height,
		      PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd, map.offset);

	if (fb_mem == MAP_FAILED) {
		perror("mmap");
		ret = -1;
		goto cleanup;
	}

	fp = fopen(src_file_path, "rb");
	if (!fp) {
		fprintf(stderr, "Could not open %s\n", src_file_path);
		ret = -EINVAL;
		goto cleanup;
	}

	if (fread(src_file_buf, SRC_FILE_SIZE, 1, fp) != 1) {
		fprintf(stderr, "Could not read %s\n", src_file_path);
		ret = -EIO;
		goto cleanup;
	}

	/* format convert : ARGB8888 -> BGRA8888 */
	rgba_to_bgra((uint32_t *)src_file_buf, 480, 800);

	/* copy row by row to account for framebuffer pitch */
	{
		uint8_t *src = src_file_buf;
		uint8_t *dst = (uint8_t *)fb_mem;
		uint32_t row_bytes = 480 * 4;

		for (int row = 0; row < 800; row++) {
			memcpy(dst, src, row_bytes);
			src += row_bytes;
			dst += fb2->pitches[0];
		}
	}

cleanup:
	if (fb_mem != MAP_FAILED)
		munmap(fb_mem, fb2->pitches[0] * fb2->height);
	if (fp)
		fclose(fp);
	if (fb2)
		drmModeFreeFB2(fb2);
	if (crtc)
		drmModeFreeCrtc(crtc);
	if (res)
		drmModeFreeResources(res);
	if (drm_fd >= 0)
		close(drm_fd);

	return ret;
}
