# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.3.1] - 2025-1-18

### What's new

-   适配 bds 1.21.50.
-   新的选中方块高亮提示，需要安装新的配套资源包
-   重构邻接表更新可视化相关代码
-   新指令: `/chunksourceinfo`，用于捕获游戏内 chunksource 的相关信息。

### Bug fix

-   对 blockitem 指令的参数进行调整，以修复参数不能匹配正确的重载的问题

## [0.3.0] - 2024-11-25

### What's new

新增两个指令：

-   生成任意 fallingBlock `/falling`
-   获取任意方块物品 `/blockitem`

### Bug fix

-   修复使用`info`指令时丢失的方块选中提示。

## [0.2.1] - 2024-11-23

### What's new

新增了两个指令功能：

-   获取计划刻信息的指令 `/pdtick`
-   获取实体坐标和速度的指令 `/info actor`

## [0.1.0] - 2024-10-04

figure hack 的第一个 release

[0.3.1]: https://github.com/glibcxx/figure_hack/compare/v0.3.0...v0.3.1
[0.3.0]: https://github.com/glibcxx/figure_hack/compare/v0.2.1...v0.3.0
[0.2.1]: https://github.com/glibcxx/figure_hack/compare/v0.1.0...v0.2.1
[0.1.0]: https://github.com/glibcxx/figure_hack/releases/tag/v0.1.0
