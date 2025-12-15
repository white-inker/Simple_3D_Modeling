# Simple 3D Modeling

基于 GLFW + OpenGL 的轻量级示例程序，覆盖课程要求的基本功能：基本体素建模、OBJ 导入导出、材质纹理编辑、几何变换、光照编辑、漫游观察与屏幕截图。

## 构建运行

```bash
cmake -S . -B build
cmake --build build
./build/Simple3DModeler
```

首次构建会使用 `FetchContent` 下载 GLFW 与 GLM 源码。Linux 需要 OpenGL 与 X11 相关开发包，macOS 直接使用系统 OpenGL。

## 操作指南

- **基本体元**：1 立方体、2 球体、3 圆柱、4 圆锥、5 棱柱、6 棱台。
- **OBJ 导入导出**：`Shift+L` 尝试加载 `scene.obj`，`Shift+O` 导出当前模型为 `export.obj`。
- **材质与纹理**：`M/N` 调整漫反射，`H/B` 调整高光，`<`/`>` 调整高光粗糙度，`P` 切换棋盘格纹理。
- **几何变换**：方向键/`PgUp`/`PgDn` 平移，`R/T/Y` 旋转，`+/-` 缩放，`F` 视图回位。
- **光照编辑**：`I/K` 上下、`J/L` 左右、`U/O` 前后移动光源，`[`/`]` 改变强度。
- **漫游**：鼠标左键轨道旋转，右键平移，滚轮缩放。
- **截屏**：`V` 保存当前画面为 `screenshot.png`。

## 局限说明

示例使用固定管线 OpenGL 以保持简洁，并侧重功能演示而非现代渲染效果。若需要更先进的着色、阴影等可在现有框架上拓展。
