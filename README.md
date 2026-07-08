# Rockchip RK3506 RGA 测试项目

基于 [librga](https://github.com/rockchip-linux/librga) 官方库开发，演示通过 DRM/KMS 接口读取 framebuffer 并使用 RGA 硬件加速器完成图像拷贝。仅支持 RK3506 平台。

中文 | [English](README_EN.md)

## 编译

```bash
# 1. 根据你的环境修改 toolchain_linux.cmake 中的工具链路径
# 2. 执行编译脚本
chmod +x cmake-linux.sh
./cmake-linux.sh
```

将图片转换为 RGBA8888 格式
```
ffmpeg -i assets/480x800.png -pix_fmt rgba -f rawvideo in0w480-h800-rgba8888.bin
```

## 运行

```bash
scp in0w480-h800-rgba8888.bin root@192.168.50.133:/data/
```

将可执行文件拷贝到 RK3506 开发板，放置到 `/root` 目录下运行：

```bash
./rga_drm_img_display
```

程序会尝试从 `/data` 读取图像文件进行拷贝

## 许可证

Apache License 2.0
