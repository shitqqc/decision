# rm_referee_msgs

RoboMaster 2027 裁判系统标准链路（TTL）ROS 2 消息定义。消息字段、字节序和单位依据
《RoboMaster 2027 机甲大师高校系列赛通信协议 V1.0.0（20261029）》，由 `rm_referee`
驱动包的 `cmd::` 负载结构转换而来。

把接口从驱动中拆出后，其他功能包可以只依赖本包，不必引入串口、rclcpp 驱动或模板节点。

## 消息分组

| 分组 | 消息 |
|---|---|
| 原始帧 | `RefereeFrame` |
| 比赛与场地 | `GameStatus`、`GameResult`、`GameRobotHP`、`FieldEvent`、`RefereeWarning`、`DartInfo` |
| 机器人状态 | `RobotStatus`、`PowerHeat`、`RobotPosition`、`RobotBuff`、`RobotHurt`、`Shoot`、`ProjectileAllowance`、`RfidStatus`、`DartClientCmd`、`SentryInfo`、`RadarInfo`、`GroundRobotPosition`、`RadarMark` |
| 交互与小地图 | `RobotInteraction`、`MapCommand`、`MapRobotData`、`MapData` |

共 24 个消息，与协议表 1-5 的常规链路命令一一对应：
`0x0001`～`0x0003`、`0x0101`、`0x0104`、`0x0105`、`0x0201`～`0x020E`、`0x0301`、
`0x0303`、`0x0305`、`0x0307`、`0x0308`。

## 约定

- 所有带状态的消息都含 `std_msgs/Header`，`frame_id` 由驱动参数配置；
- 坐标单位以协议为准：`RobotPosition` 为 m/度，`GroundRobotPosition` 为 m，
  `MapRobotData` 为 cm，`MapData` 为 dm；
- `RobotInteraction.user_data` 与 `RefereeFrame.data` 是变长数组；
- `MapData.delta_x/delta_y` 为 `int8[49]`，与协议中的有符号差分一致。

## 构建

```bash
cd ~/pioneer_nav/ros2_ws
source /opt/ros/jazzy/setup.bash
colcon build --packages-up-to rm_referee --symlink-install
source install/setup.bash

ros2 interface show rm_referee_msgs/msg/GameStatus
```

驱动包 `rm_referee` 依赖本包；只使用消息时：

```bash
colcon build --packages-select rm_referee_msgs --symlink-install
```
