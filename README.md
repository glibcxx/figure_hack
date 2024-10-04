# Figure Hack

一个有助于分析红石系统的小工具

## 安装

本插件基于bds 1.21.3.01 开发，不保证兼容以前的版本

1. 安装插件：

```shell
lip install github.com/glibcxx/Figure-Hack
```

2. 安装行为包和资源包：

从release页面下载`assets.mcaddon`，双击安装。

然后将`assets.mcaddon`内的BP文件夹复制到bds的`development_behavior_packs`文件夹内。

在存档的`world_behavior_packs.json`文件内，确保包含下面内容：

```json
[
	{
		"pack_id" : "495e0ad1-7fe1-40bc-a8b7-00b097a3a392",
		"version" : [ 1, 0, 0 ]
	}
]
```

## 使用

使用以下命令获得"魔杖"：

```
/mstick
```

手持魔杖按`Q`或者点击鼠标左键切换模式。

对着方块按鼠标右键使用对应功能。

对于可交互方块，我们强烈建议按住蹲再点鼠标右键。

## 功能

### 查询方块信息

```
/info basic [x y z]
```

查询方块的基本信息。x y z 坐标是可选项。对应魔杖的普通模式。

---

```
/info circuit [x y z]
```

查询红石元件的基本信息。x y z 坐标是可选项。对应魔杖的红石模式。

---

```
/info source [x y z]
```

查询红石元件的信号源。x y z 坐标是可选项。对应魔杖的信号源模式。

### 修改游戏速度

```
/tick (数字)/pause/resume/reset
```

- 数字：tps, 默认20
- pause：暂停游戏
- resume：恢复游戏
- reset：tps设为20

### 邻接表更新可视化

该功能需要将`config.json`文件的`"enable_microtick"`设为`true`才能启用。

使用前请备份存档。

使用魔杖的邻接表模式右键选中红石元件，当该元件发生邻接表更新时，会自动显示搜索元件的范围。

再次右键可以取消选中，切换模式可以一键取消选中所有。

更新速度受 tick 指令影响。
