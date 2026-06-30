# LVGL9AndroidDemo

一个在 Android 平台上运行 [LVGL 9.5.0](https://github.com/lvgl/lvgl) 的示例工程。项目通过 `SurfaceView` 承载 Native 渲染结果，由 JNI 启动 C++ 层的 LVGL 主循环，并内置 LVGL 官方 Demo（Widgets、Music、Benchmark）与一个完整的中国象棋人机对弈示例。

## 功能概览

- 内置 LVGL 官方 Demo：`Widgets`、`Music`、`Benchmark`。
- 自定义 LVGL 应用：`Chinese Chess` 象棋人机对弈（含 AI 搜索引擎、开局库、悔棋）。
- Java 层负责 Demo 列表展示、屏幕方向控制、显示区域比例约束和触摸事件转发。
- C++ 层负责 LVGL 初始化、渲染刷新（`ANativeWindow`）、触摸输入读取和独立渲染线程管理。
- 支持多分辨率适配（`480×320`、`320×480` 等），通过 `ConstraintLayout` 比例约束自适应。

## 项目结构

```text
LVGL9AndroidDemo/
├── app/
│   ├── build.gradle                          # 应用构建脚本（AGP 8.9.1, compileSdk 36）
│   ├── proguard-rules.pro                    # ProGuard 规则
│   ├── demokey.jks                           # 调试签名文件
│   └── src/main/
│       ├── AndroidManifest.xml               # 清单文件
│       ├── java/com/hzy/lvgl/demo/
│       │   ├── LVApp.java                    # JNI 包装类，管理 Native LVGL 实例生命周期
│       │   ├── LVGLView.java                 # SurfaceView 子类，转发触摸事件 & 管理渲染生命周期
│       │   ├── MainApp.java                  # Application，初始化工具库
│       │   ├── model/
│       │   │   └── DemoEntry.java            # Demo 列表数据模型
│       │   └── ui/
│       │       ├── SplashActivity.java       # 启动页（跳转至 MainActivity）
│       │       ├── MainActivity.java         # 主页，展示 Demo 列表
│       │       ├── PartScreenActivity.java   # Demo 展示页，承载 LVGLView
│       │       └── adapter/
│       │           └── DemoAdapter.java      # RecyclerView 适配器
│       ├── cpp/
│       │   ├── Android.mk                    # ndk-build 构建脚本
│       │   ├── Application.mk                # ABI & STL 配置
│       │   ├── LVApi.cpp                     # JNI 入口，Java native 方法实现
│       │   ├── LVApp.cpp / .h                # LVGL 主循环、显示刷新、触摸桥接
│       │   ├── AppList.h                     # Demo 名称 → 启动函数映射表
│       │   ├── LVHelper.h                    # 日志宏定义（NDEBUG 下禁用）
│       │   ├── chess/                        # 中国象棋 Demo
│       │   │   ├── LvChess.cpp / .h          # 象棋 UI 逻辑（棋盘绘制、棋子动画、事件处理）
│       │   │   ├── XQWL.h                    # 象棋搜索引擎（Alpha-Beta 搜索、置换表、开局库）
│       │   │   ├── BookData.h                # 开局库数据
│       │   │   ├── teng_18.c                 # 自定义中文字体（18px 腾祥爱情体）
│       │   │   └── imgs/                     # 棋盘 & 棋子图片资源（C 数组格式）
│       │   └── lvgl-9.5.0/                   # LVGL 9.5.0 官方源码及官方 Demo
│       └── res/                              # Android 资源（布局、图标、主题、字符串）
├── build.gradle                              # 根构建脚本
├── settings.gradle                           # 项目设置
└── gradle.properties                         # Gradle 全局属性
```

## 架构说明

### 整体数据流

```text
┌─────────────────────────────────────────────────────┐
│                    Android (Java)                     │
│                                                       │
│  MainActivity ──→ DemoEntry列表 ──→ PartScreenActivity │
│                                         │              │
│                                    LVGLView            │
│                                  (SurfaceView)         │
│                              ┌──────────┼──────────┐  │
│                              │ Surface  │ Touch    │  │
│                              │ Callback │ Event    │  │
│                              └────┬─────┴────┬─────┘  │
│                                   │           │        │
│                              LVApp.java (JNI 包装)     │
│                                   │           │        │
└───────────────────────────────────┼───────────┼────────┘
                                    │ JNI       │
┌───────────────────────────────────┼───────────┼────────┐
│                   Android (C++ / Native)       │        │
│                                   │           │        │
│                              LVApi.cpp        │        │
│                                   │           │        │
│                              LVApp.cpp        │        │
│                          ┌────────┴────────┐  │        │
│                          │  渲染线程        │  │        │
│                          │  (std::thread)   │  │        │
│                          │                  │  │        │
│                          │  lv_init()       │  │        │
│                          │  lv_display      │  │        │
│                          │  lv_indev        │  │        │
│                          │  lv_timer_handler│  │        │
│                          │       ↕          │  │        │
│                          │  ANativeWindow   │◄┘        │
│                          │  (RGB_565)       │          │
│                          └──────────────────┘          │
│                                  │                     │
│                          AppList.h 映射                  │
│                          ┌───────┼───────┐              │
│                          │       │       │              │
│                     widgets  music   chess              │
│                     (官方Demo)      (自定义)              │
└─────────────────────────────────────────────────────────┘
```

### 关键设计

1. **渲染线程隔离**：`LVApp` 在独立的 `std::thread` 中运行 LVGL 主循环（`lv_timer_handler`），避免阻塞 UI 线程。LVGL 非线程安全，所有 LVGL API 调用均在此线程内完成。

2. **Surface 生命周期**：`SurfaceView` 的 `surfaceChanged` 回调触发 `LVApp.start()`，`surfaceDestroyed` 触发 `LVApp.stop()`（暂停渲染但保留实例）。`onDetachedFromWindow` 时才真正销毁 Native 实例。

3. **触摸坐标映射**：Java 层传递原始屏幕坐标，C++ 层在 `LVApp::onTouch()` 中按 `app_width / screen_width` 比例换算为 LVGL 逻辑坐标。

4. **显示缓冲**：使用 `LV_DISPLAY_RENDER_MODE_PARTIAL` 模式，分配 `10 * width * height` 字节的绘制缓冲区。刷新回调中将局部区域数据拷贝到 `ANativeWindow` 的全屏缓冲。

## 构建环境

| 项目 | 版本/要求 |
|------|-----------|
| Android Studio | Hedgehog 或更高 |
| Gradle | 8.11.1 |
| Android Gradle Plugin | 8.9.1 |
| compileSdk | 36 |
| minSdk | 21 |
| targetSdk | 36 |
| Android NDK | r25c+（ndk-build） |
| C++ 标准 | C++17 |
| 支持 ABI | `armeabi-v7a`、`arm64-v8a` |

## 构建方式

仓库中的 `gradlew` 可能没有可执行权限，可以任选一种方式构建：

```bash
chmod +x ./gradlew
./gradlew assembleDebug
```

或：

```bash
sh ./gradlew assembleDebug
```

构建产物会生成在：

```text
app/build/outputs/apk/debug/
```

## 添加新的 LVGL Demo

1. 在 `app/src/main/cpp` 下添加新的 LVGL 启动函数，例如 `void lv_demo_xxx_start();`。
2. 在 `AppList.h` 中加入名称映射：

```cpp
{"xxx", lv_demo_xxx_start},
```

3. 在 `MainActivity.initData()` 中添加入口：

```java
mDemoList.add(new DemoEntry("Demo Xxx", "xxx", 480, 320, ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE));
```

其中第二个参数需要和 `AppList.h` 中的 key 保持一致。

## 中国象棋 Demo 说明

象棋 Demo 基于 LVGL 9 实现，包含完整的棋盘绘制、棋子动画和 AI 引擎。

### AI 引擎特性

- **搜索算法**：Alpha-Beta 剪枝 + PVS（Principal Variation Search）
- **优化技术**：置换表（Zobrist Hash）、杀手走法启发、历史表启发、空步裁剪、MVV/LVA 排序
- **开局库**：内置 600 条开局数据，支持局面镜像搜索
- **搜索深度**：最大 4 层（`LIMIT_DEPTH`），时间限制 1 秒
- **重复检测**：支持长将判负、长打判负、自然限着判和

### 棋盘坐标系

棋盘使用 256 格一维数组表示（16×16 布局），有效格子范围 `FILE_LEFT(3)` ~ `FILE_RIGHT(11)` × `RANK_TOP(3)` ~ `RANK_BOTTOM(12)`。棋子编号规则：红方 8~14，黑方 16~22。

## 注意事项

- LVGL 不是线程安全库，当前工程只在 Native 渲染线程中调用 LVGL 主循环。
- `LVGLView` 会把 Android 触摸坐标按显示比例换算为 LVGL 坐标。
- `lvgl-9.5.0` 是第三方源码，日常业务修改建议集中在 `LVApp.*`、`LVApi.cpp`、`AppList.h` 和自定义 Demo 目录。
- 当前 Demo 使用内置调试签名配置（`demokey.jks`），不建议直接用于正式发布。
- Release 构建已启用混淆和资源压缩，ProGuard 规则配置在 `proguard-rules.pro`。

## 优化建议

以下是对项目的优化建议，已实现的标注 ✅，待实现的标注 📋。

### 高优先级

1. ✅ **渲染缓冲内存优化**：已将绘制缓冲从 `10 * width * height`（约 1.5MB）减小为 `width * height * 2`（约 300KB），内存占用降低约 80%。

2. ✅ **`lv_flush_callback` 双重拷贝消除**：已移除 `surface_buf` 中间缓冲，刷新回调直接锁定 `ANativeWindow` 并将局部区域数据写入帧缓冲，省去全屏拷贝开销。

3. 📋 **触摸事件多指支持**：当前 `onTouchEvent` 仅处理单指触摸（`ACTION_DOWN/MOVE/UP`），可扩展支持多指以适配更复杂的交互场景。

4. ✅ **`usleep(1000)` 替换为自适应休眠**：已使用 `lv_timer_handler()` 的返回值动态调整休眠时长（1~33ms），减少不必要的 CPU 唤醒，降低功耗。

### 中优先级

5. ✅ **Java 层使用 `Intent` extras 常量**：已在 `PartScreenActivity` 中定义 `EXTRA_APP`、`EXTRA_TITLE` 等常量，`MainActivity` 统一引用，消除字符串拼写错误风险。

6. ✅ **`DemoEntry` 字段访问修饰符**：已为所有字段添加 `final` 修饰符，确保对象不可变，提高线程安全性。

7. ✅ **ProGuard 配置**：已启用 Release 混淆（`minifyEnabled true` + `shrinkResources true`），并配置了 JNI native 方法、ViewBinding 等相关 keep 规则。

8. 📋 **`SplashActivity` 优化**：当前直接跳转，可考虑使用 `SplashScreen` API（Android 12+）或添加品牌展示。

9. ✅ **象棋 AI 搜索深度可配置**：已将 `LIMIT_DEPTH` 改为运行时可变量，并在 `lv_chess_start()` 中调用 `AutoSetSearchDepth()` 根据设备 CPU 核心数自动设置（<4 核=3 层，4~7 核=4 层，8+ 核=5 层）。

### 低优先级

10. 📋 **C++ 代码现代化**：`XQWL.h` 中的引擎代码保留了较多 C 风格写法（`static` 全局变量、裸指针），可逐步迁移为 RAII 风格和智能指针管理。

11. 📋 **日志系统增强**：`LVHelper.h` 在 `NDEBUG` 下完全禁用日志，可改为运行时级别控制。

12. 📋 **图片资源格式**：棋子图片以 C 数组形式编译进 so 库，体积较大。可考虑改用 LVGL 文件系统从 assets 加载，减小 so 体积。

13. 📋 **`Android.mk` 迁移至 CMake**：ndk-build 已逐步被 CMake 取代，建议迁移以获得更好的 IDE 集成和调试体验。

## 参考资料

- LVGL 官方仓库：https://github.com/lvgl/lvgl
- LVGL 文档：https://docs.lvgl.io/
- Android NDK NativeWindow 文档：https://developer.android.com/ndk/reference/group/native-activity
- 象棋引擎算法参考：[象棋小巫师](https://github.com/xqbase/xqwizard)
