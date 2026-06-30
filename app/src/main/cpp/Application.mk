# Application.mk - NDK 应用级配置
APP_STL := c++_static          # 静态链接 C++ 运行时，避免外部依赖
APP_CPPFLAGS := -std=c++17      # 使用 C++17 标准
APP_SHORT_COMMANDS := true      # 启用短命令，避免 Windows 命令行长度限制