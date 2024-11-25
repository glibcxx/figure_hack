# Figure Hack

一个有助于分析红石系统的小工具，但将不限于此。

## 安装

本插件基于 bds 1.21.3.01 开发，不保证兼容以前的版本

1. 安装插件：

```shell
lip install github.com/glibcxx/figure_hack
```

2. 安装行为包和资源包：

从 release 页面下载`assets.mcaddon`，双击安装。

然后将`assets.mcaddon`内的 BP 文件夹复制到 bds 的`development_behavior_packs`文件夹内。

在存档的`world_behavior_packs.json`文件内，确保包含下面内容：

```json
[
    {
        "pack_id": "495e0ad1-7fe1-40bc-a8b7-00b097a3a392",
        "version": [1, 0, 0]
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

### 查询实体信息

```
/info actor
```

查询玩家指向的实体信息。目前给出坐标、速度、runtimeId 这三个信息。

### 修改游戏速度

```
/tick (数字)/pause/resume/reset
```

-   数字：tps, 默认 20
-   pause：暂停游戏
-   resume：恢复游戏
-   reset：tps 设为 20

可以在`config.json`中关闭，以防止同功能冲突

### 邻接表更新可视化

该功能需要将`config.json`文件的`"enable_microtick"`设为`true`才能启用。

使用前请备份存档。

使用魔杖的邻接表模式右键选中红石元件，当该元件发生邻接表更新时，会自动显示搜索元件的范围。

再次右键可以取消选中，切换模式可以一键取消选中所有。

更新速度受 tick 指令影响。

### 生成任意下落的方块

```
/falling 方块命名空间id/方块数字Id [x y z]
```

坐标缺省时，为指令执行者坐标。命名空间 id 可以为原版指令方块列表中不包含的 id，比如`moving_block`

### 获取任意方块物品

```
/blockitem 方块命名空间id/方块数字Id
```

执行后会强行给予玩家方块对应的物品。命名空间 id 可以为原版指令方块列表中不包含的 id，比如`moving_block`
