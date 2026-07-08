# Rockchip RK3506 RGA Test Project

A demo based on the official [librga](https://github.com/rockchip-linux/librga) library, demonstrating framebuffer readback via DRM/KMS and hardware-accelerated image copy using the RGA engine. Only supports the RK3506 platform.

[中文](README.md) | English

## Build

```bash
# 1. Edit toolchain_linux.cmake to set your toolchain path according to your environment
# 2. Run the build script
chmod +x cmake-linux.sh
./cmake-linux.sh
```

## Run

Copy the executable to your RK3506 board and run from `/data`:

```bash
./rga_drm_img_display
```

The program loads an image from `/data` for copying.

## License

Apache License 2.0
