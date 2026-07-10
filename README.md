# Rockchip RK3506 RGA 测试项目

基于 [librga](https://github.com/rockchip-linux/librga) 官方库开发，演示通过 DRM/KMS 接口读取 framebuffer 并使用 RGA 硬件加速器完成图像拷贝。仅支持 RK3506 平台。

中文 | [English](README_EN.md)

## 编译

### 准备工具链

默认使用 Luckfox Buildroot 工具链，路径为：

```
/home/developer/luckfox/lyra/buildroot/output/rockchip_rk3506_luckfox/host
```

如果你的工具链路径不同，有两种方式指定：

```bash
# 方式一：环境变量（推荐）
export RGA_TOOLCHAIN_HOME=/path/to/your/toolchain
./build.sh

# 方式二：直接修改 toolchain_linux.cmake 中的 TOOLCHAIN_HOME 默认值
```

### 编译步骤

```bash
chmod +x build.sh
./build.sh
```

也可以分步执行：

```bash
./build.sh                                # 配置 + 编译
cmake --install build                     # 安装到 build/install（按需）
```

### 生成测试图像

```bash
ffmpeg -i assets/480x800.png -pix_fmt rgba -f rawvideo in0w480-h800-rgba8888.bin
```

## 运行

将可执行文件和图像数据拷贝到 RK3506 开发板：

```bash
# 拷贝图像
scp in0w480-h800-rgba8888.bin root@192.168.50.133:/data/

# 拷贝可执行文件
scp build/rga_drm_img_display root@192.168.50.133:/root/
```

在开发板上运行：

```bash
cd /root
./rga_drm_img_display
```

程序会尝试从 `/data` 读取图像文件进行拷贝。

## 项目结构

```
├── build.sh                   # 编译脚本
├── toolchain_linux.cmake     # 交叉编译工具链
├── CMakeLists.txt            # 顶层 CMake
├── examples/                 # 示例程序
│   ├── rga_drm_img_display.cpp
│   └── cpu_drm_img_display.cpp
├── utils/                    # 工具库（allocator、libdrm 封装）
├── include/                  # RGA API 头文件
├── libs/                     # 预编译 librga
├── assets/                   # 测试素材
└── build/                    # 构建输出（自动生成）
    ├── compile_commands.json # 编译数据库（供 clangd 使用）
    └── rga_drm_img_display
```

## 许可证

Apache License 2.0
