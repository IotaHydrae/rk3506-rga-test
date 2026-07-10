# Rockchip RK3506 RGA Test Project

A demo based on the official [librga](https://github.com/rockchip-linux/librga) library, demonstrating framebuffer readback via DRM/KMS and hardware-accelerated image copy using the RGA engine. Only supports the RK3506 platform.

[中文](README.md) | English

## Build

### Toolchain Setup

The default toolchain path targets the Luckfox Buildroot SDK:

```
/home/developer/luckfox/lyra/buildroot/output/rockchip_rk3506_luckfox/host
```

If your toolchain is installed elsewhere, specify it via environment variable:

```bash
# Option 1: Environment variable (recommended)
export RGA_TOOLCHAIN_HOME=/path/to/your/toolchain
./build.sh

# Option 2: Edit the default fallback in toolchain_linux.cmake
```

### Build Steps

```bash
chmod +x build.sh
./build.sh
```

Or step by step:

```bash
./build.sh                                # Configure + build
cmake --install build                     # Install to build/install (optional)
```

### Generate Test Image

```bash
ffmpeg -i assets/480x800.png -pix_fmt rgba -f rawvideo in0w480-h800-rgba8888.bin
```

## Run

Copy the executable and image data to your RK3506 board:

```bash
# Copy image
scp in0w480-h800-rgba8888.bin root@192.168.50.133:/data/

# Copy executable
scp build/rga_drm_img_display root@192.168.50.133:/root/
```

Run on the board:

```bash
cd /root
./rga_drm_img_display
```

The program loads an image from `/data` for copying.

## Project Structure

```
├── build.sh                   # Build script
├── toolchain_linux.cmake     # Cross-compilation toolchain
├── CMakeLists.txt            # Top-level CMake
├── examples/                 # Example programs
│   ├── rga_drm_img_display.cpp
│   └── cpu_drm_img_display.cpp
├── utils/                    # Utility library (allocator, libdrm wrappers)
├── include/                  # RGA API headers
├── libs/                     # Prebuilt librga
├── assets/                   # Test assets
└── build/                    # Build output (auto-generated)
    ├── compile_commands.json # Compilation database (for clangd)
    └── rga_drm_img_display
```

## License

Apache License 2.0
