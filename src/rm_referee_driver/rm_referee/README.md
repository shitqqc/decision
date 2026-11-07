# rm_referee

RoboMaster 2027 裁判系统标准链路（TTL 串口）ROS 2 驱动。协议依据工作空间根目录的
《RoboMaster 2027 机甲大师高校系列赛通信协议 V1.0.0（20261029）》。

功能包的核心是一个 CRTP 模板基类 `RefereeNode<Derived, CommandList<...>>`：**要接收哪些
命令由子类的 `CommandList` 在编译期决定**。基类负责串口、组帧、CRC、解析和分发，子类只需
声明命令列表并实现对应的 `on_command()`。

- ROS 2 Jazzy，C++17，`ament_cmake`
- 不依赖第三方串口库，使用 POSIX `termios` + `poll`
- 消息定义独立在 `rm_referee_msgs` 功能包，本包只保留驱动与模板基类
- `rm_referee_msgs` 提供 24 个消息类型，本包提供步兵/哨兵/雷达三个示例子类
- 17 个单元测试覆盖 CRC、组帧/解析、重同步、编解码与命令分发

---

## 1. 快速开始

```bash
cd ~/pioneer_nav/ros2_ws
source /opt/ros/jazzy/setup.bash
colcon build --packages-up-to rm_referee --symlink-install
source install/setup.bash

# 步兵；sentry / radar 同理
ros2 launch rm_referee referee.launch.py robot_type:=infantry port:=/dev/ttyUSB0
```

若串口权限不足：

```bash
sudo usermod -aG dialout "$USER"   # 重新登录后生效
# 或者临时： sudo chmod a+rw /dev/ttyUSB0
```

默认参数在 [config/referee.yaml](config/referee.yaml)。`robot_type` 只用于选择可执行文件，
三个角色共用节点名 `referee_node`，因此共用同一份 YAML 参数块。

| 参数 | 默认值 | 说明 |
|---|---|---|
| `port` | `/dev/ttyUSB0` | TTL 模块对应的串口设备 |
| `baudrate` | `115200` | 电源管理模块端口；图传链路为 921600，本包不涉及 |
| `read_timeout_ms` | `50` | 单次 `poll` 读超时；同时决定接收线程的退出粒度 |
| `reconnect_interval_ms` | `1000` | 打开失败/链路断开后的重试间隔，最小 10 ms |
| `max_payload_size` | `118` | 单帧 data 上限，协议标准链路最大值 |
| `publish_raw` | `false` | 是否把所有有效帧原样发布到 `~/raw_frame` |
| `frame_id` | `referee` | 所有消息 `Header.frame_id` |

---

## 2. 协议实现

### 2.1 帧格式

| 偏移 | 长度 | 字段 | 说明 |
|---|---|---|---|
| 0 | 1 | `SOF` | 固定 `0xA5` |
| 1 | 2 | `data_length` | 小端，**仅 data 长度**，不含头尾 |
| 3 | 1 | `seq` | 发送序号 |
| 4 | 1 | `CRC8` | 对偏移 0..3 共 4 字节 |
| 5 | 2 | `cmd_id` | 小端 |
| 7 | n | `data` | 命令负载 |
| 7+n | 2 | `CRC16` | 小端，对偏移 0..(6+n) 共 `7+n` 字节 |

- CRC8：反射多项式 `0x8C`，初值 `0xFF`
- CRC16：反射多项式 `0x8408`，初值 `0xFFFF`，无 xorout
- 8N1，无流控；标准链路单帧最长 `9 + 118 = 127` 字节

实现见 [include/rm_referee/crc.hpp](include/rm_referee/crc.hpp) 与
[include/rm_referee/frame.hpp](include/rm_referee/frame.hpp)。CRC 用位运算实现，不占查表
内存；解析器逐字节寻找 `0xA5`，头部 CRC8 或尾部 CRC16 失败时只丢弃 1 字节后重新同步，
因此串口噪声或半帧不会让解析器永久失步。

校验基准（协议中的 `0x0001` 示例帧，data 为 11 个 0 字节）：

```text
a5 0b 00 01 5c 01 00 00 00 00 00 00 00 00 00 00 00 00 66 cb
```

注意尾部的 `66 CB` 是小端存储，CRC16 数值为 `0xCB66`。

### 2.2 命令负载

所有 `cmd::` 结构都是 `#pragma pack(1)`，字段顺序与协议一致，
`static_assert(sizeof(...) == 协议长度)` 在编译期兜底。多字节字段按小端解释（裁判系统与
RoboMaster 控制板均为小端）。结构体放在
[include/rm_referee/commands.hpp](include/rm_referee/commands.hpp)，序列化在
[include/rm_referee/codec.hpp](include/rm_referee/codec.hpp)：

- 定长命令：`memcpy`，长度必须精确匹配
- `0x0301 robot_interaction`：6 字节头 + 最多 112 字节 `user_data`，长度 6..118 均可
- 收到长度不符的负载时只告警并丢弃该帧，不会越界读取

---

## 3. 模板基类

```cpp
template <typename... Commands>
struct CommandList {};

template <typename Derived, typename... Cmds>
class RefereeNode<Derived, CommandList<Cmds...>> : public rclcpp::Node
{
  // 串口、解析、CRC、raw_frame、connected、send()/send_raw() ...
};
```

`Derived` 是子类自身（CRTP），`CommandList<...>` 精确列出要解码的命令类型。基类：

1. 打开 `port`，以 `baudrate` 配置 8N1 raw 模式；
2. 在独立接收线程中读取字节并送入 `FrameParser`；
3. 对每个有效帧：可选发布 `~/raw_frame` → 调用 `Derived::on_frame()`（若存在）→
   调用帧回调 → 按 `CommandList` 逐个匹配 `cmd_id` 并解码；
4. 若子类实现了 `void on_command(const Cmd &)`，调用它；**命令可以只列入列表而不实现
   handler**（此时仅 `raw_frame` 可见，`publish_raw` 为 false 时等于忽略）。

命令列表中没有的命令不会被解码；解析器仍会消费它们并计入统计，不会阻塞后续帧。

### 3.1 自定义子类

```cpp
#include "rm_referee/referee_node.hpp"
#include "rm_referee/ros_convert.hpp"

using MyCommands = rm_referee::CommandList<
  rm_referee::cmd::GameStatus,
  rm_referee::cmd::RobotStatus,
  rm_referee::cmd::PowerHeat>;

class MyRefereeNode : public rm_referee::RefereeNode<MyRefereeNode, MyCommands>
{
public:
  using Base = rm_referee::RefereeNode<MyRefereeNode, MyCommands>;

  explicit MyRefereeNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Base("my_referee_node", options)
  {
    status_pub_ = create_publisher<rm_referee_msgs::msg::RobotStatus>("~/robot_status", 10);
    start();   // 基类不会自动启动接收线程
  }

  void on_command(const rm_referee::cmd::RobotStatus & value)
  {
    status_pub_->publish(rm_referee::convert::to_msg(value, make_header(now())));
  }

private:
  rclcpp::Publisher<rm_referee_msgs::msg::RobotStatus>::SharedPtr status_pub_;
};
```

要点：

- `on_command` 必须是 **public**，否则 CRTP 的编译期检测看不到它；
- 命令类型必须带 `static constexpr std::uint16_t kId`，编码/解码按 `sizeof` 自动工作；
- 子类构造函数末尾调用 `start()`；若节点可能在运行中被销毁，请在子类析构函数里调用
  `stop()`（三个示例子类都这样做），以保证接收线程先退出、再销毁子类的 publisher；
  基类析构函数也会兜底调用 `stop()`；
- handler 运行在接收线程，**不要在里面做阻塞操作**；需要发布时直接调用 publisher 即可；
- `send()` / `send_raw()` 内部有发送互斥锁，可从任意线程调用。

### 3.2 发送

```cpp
rm_referee::cmd::RobotInteraction msg;
msg.data_cmd_id = 0x0200;
msg.sender_id   = 7;
msg.receiver_id = 0x8080;
msg.user_data   = {0x01, 0x02};
node->send(msg);                              // 自动组帧、递增 seq
node->send_raw(0x0301, data, length);         // 自定义负载
```

`send()` 会检查 `codec::encode()` 的长度是否与声明的长度一致，`0x0301` 的 `user_data`
超过 112 字节会被拒绝。发送失败（例如串口未打开）返回 `false` 并限流告警，不会抛异常。

---

## 4. 已封装的命令

| cmd_id | data 长度 | 结构 / 消息 | 说明 |
|---|---|---|---|
| `0x0001` | 11 | `GameStatus` | 比赛类型/阶段、剩余时间、同步时间戳 |
| `0x0002` | 1 | `GameResult` | 胜方 |
| `0x0003` | 24 | `GameRobotHP` | 双方血量、前哨站、基地、控制度 |
| `0x0101` | 8 | `FieldEvent` | 场地事件 |
| `0x0104` | 3 | `RefereeWarning` | 裁判警告 |
| `0x0105` | 3 | `DartInfo` | 飞镖信息（结构可用，示例角色未启用） |
| `0x0201` | 17 | `RobotStatus` | 机器人 ID/等级、血量、热量上限、底盘功率、枪口速度、电源输出 |
| `0x0202` | 14 | `PowerHeat` | 缓冲能量、17/42 mm 枪管热量（前 8 B 保留） |
| `0x0203` | 12 | `RobotPosition` | 位置（米）与朝向（度，正北 0） |
| `0x0204` | 7 | `RobotBuff` | 回血/冷却/防御/易伤/攻击增益 |
| `0x0206` | 7 | `RobotHurt` | 装甲 ID、扣血原因、受击序号、命中位置 |
| `0x0207` | 7 | `Shoot` | 弹种、枪管编号、射频、初速 |
| `0x0208` | 8 | `ProjectileAllowance` | 允许发弹量、剩余金币、堡垒发弹量 |
| `0x0209` | 6 | `RfidStatus` | RFID 状态 |
| `0x020A` | 6 | `DartClientCmd` | 飞镖选手端指令（结构可用，示例角色未启用） |
| `0x020B` | 40 | `GroundRobotPosition` | **己方**重装/3 号/4 号坐标（m） |
| `0x020C` | 2 | `RadarMark` | 雷达标记进度 |
| `0x020D` | 14 | `SentryInfo` | 哨兵决策信息，已拆成位域访问器 |
| `0x020E` | 8 | `RadarInfo` | 雷达决策信息，已拆成位域访问器 |
| `0x0301` | 6..118 | `RobotInteraction` | 机器人间交互，6 B 头 + ≤112 B 数据 |
| `0x0303` | 12 | `MapCommand` | 小地图点击/目标 |
| `0x0305` | 48 | `MapRobotData` | 小地图双方机器人坐标（cm） |
| `0x0307` | 105 | `MapData` | 小地图路径（dm，49 点，结构可用，示例角色未启用） |
| `0x0308` | 34 | `CustomInfo` | 自定义消息（UTF-16，结构可用，示例角色未启用） |

图传链路（`0x0310`/`0x0311`）和雷达无线链路（`0x0A01`..`0x0A06`）不经过本包使用的 TTL
标准链路，未实现。

`0x020D` 的位域访问器包括 `exchanged_ammo()`、`can_buy_revive()`、`posture()`、
`attack_weak_remaining()` 等；`0x020E` 包括 `double_vulnerability_chances()`、
`encryption_level()`、`armor_fortification_active()` 等。原始三个 `u32/u16/u64` 字段也
保留，便于协议升级时自行解析。

---

## 5. 示例子类与话题

三个角色共用节点名 `referee_node`，因此话题都在 `/referee_node/` 下。

### 5.1 步兵 `InfantryRefereeNode`

命令：`GameStatus, GameResult, GameRobotHP, FieldEvent, RefereeWarning, RobotStatus,
PowerHeat, RobotPosition, RobotBuff, RobotHurt, Shoot, ProjectileAllowance, RfidStatus,
RobotInteraction, MapCommand`

发布：`~/game_status`、`~/game_result`、`~/game_robot_hp`、`~/field_event`、
`~/referee_warning`、`~/robot_status`、`~/power_heat`、`~/robot_position`、
`~/robot_buff`、`~/robot_hurt`、`~/shoot`、`~/projectile_allowance`、`~/rfid_status`、
`~/robot_interaction`、`~/map_command`

订阅：`~/tx/robot_interaction` → 编码为 `0x0301` 发出

### 5.2 哨兵 `SentryRefereeNode`

命令：`GameStatus, GameRobotHP, FieldEvent, RefereeWarning, RobotStatus, PowerHeat,
RobotPosition, ProjectileAllowance, SentryInfo, GroundRobotPosition, RadarMark,
RobotInteraction`

发布：`~/game_status`、`~/game_robot_hp`、`~/field_event`、`~/referee_warning`、
`~/robot_status`、`~/power_heat`、`~/robot_position`、`~/projectile_allowance`、
`~/sentry_info`、`~/ground_robot_position`、`~/radar_mark`、`~/robot_interaction`

订阅：`~/tx/robot_interaction`

### 5.3 雷达 `RadarRefereeNode`

命令：`GameStatus, GameRobotHP, FieldEvent, RobotStatus, PowerHeat, RadarMark,
RadarInfo, MapCommand, RobotInteraction`

发布：`~/game_status`、`~/game_robot_hp`、`~/field_event`、`~/robot_status`、
`~/power_heat`、`~/radar_mark`、`~/radar_info`、`~/map_command`、`~/robot_interaction`

订阅：

- `~/tx/robot_interaction` → `0x0301`
- `~/tx/map_robot_data` → `0x0305`（把双方坐标推给操作手/客户端）

### 5.4 所有角色通用

- `~/raw_frame`（`rm_referee_msgs/msg/RefereeFrame`，QoS 50）：仅 `publish_raw:=true` 时发布
- `~/connected`（`std_msgs/msg/Bool`，QoS 1 transient_local）：串口是否打开

`connected=true` 只表示 TTY 打开成功，**不表示裁判系统在线**。标准链路以接收为主，建议
上层用某条高频命令（如 `0x0001`/`0x0201`）的到达时间做真正的链路存活判断。

---

## 6. 设计说明

### 6.1 为什么用 CRTP + 编译期命令表

裁判系统的命令是固定 ID、固定布局的；把“我要哪些命令”做成编译期类型列表可以：

- 让每个角色只链接/实例化自己需要的解码路径，未订阅命令零开销；
- 在编译期检查 handler 与命令类型的匹配，避免运行时字符串/ID 查表；
- 保持零动态分配（除 `0x0301` 的变长 `user_data` 与帧缓冲）。

代价是命令表不能在运行时按参数切换。因此提供三个可执行文件，由 launch 的 `robot_type`
选择；新增角色时新建一个子类和对应 main 即可，而不是在运行时解释 YAML。

### 6.2 分层

```text
serial_port (POSIX termios/poll)
        ↓ bytes
FrameParser (SOF/长度/CRC8/CRC16/重同步)
        ↓ Frame
RefereeNode<Derived, CommandList<...>>
   ├─ ~/raw_frame
   ├─ Derived::on_frame() / frame callback
   └─ dispatch_one<Cmd>() → codec::decode → Derived::on_command()
        ↓
convert::to_msg → ROS publishers
```

### 6.3 线程模型

- 一个接收线程负责打开/重连串口、读取字节、解析与分发；
- ROS executor 负责订阅回调（`~/tx/*`）和发布；
- `send()` 用 `tx_mutex_` 串行化，`seq` 在锁内自增；
- `stop()` 先置 `running_=false`，再 `join` 接收线程；在接收线程内部调用 `stop()` 会告警
  并返回，避免自 join 死锁；
- 单元测试可用 `feed()` 在未启动接收线程时同步喂字节，走与串口完全相同的解析/分发路径。

### 6.4 扩展一条新命令

1. 在 `commands.hpp` 中按协议加 `#pragma pack(1)` 结构体、`kId` 和 `static_assert`；
2. 定长命令无需改 `codec.hpp`；变长命令需仿照 `RobotInteraction` 增加特化；
3. 若要发布为 ROS 消息：在 `rm_referee_msgs/msg/` 加 `.msg`，在
   `rm_referee_msgs/CMakeLists.txt` 的 `msg_files` 注册，在 `ros_convert.hpp` 加
   `to_msg()`；
4. 把新类型加入目标子类的 `CommandList`，实现 `on_command()`（可选）；
5. 在 `test/test_protocol.cpp` 加编解码/边界测试。

---

## 7. 测试

```bash
colcon test --packages-select rm_referee --event-handlers console_direct+
colcon test-result --verbose
```

覆盖：

- CRC8/CRC16 与协议示例帧逐字节一致（`a50b...66cb`）
- 单字节喂入、噪声前缀重同步、CRC8/CRC16 错误恢复、超长长度字段
- 定长/变长命令编解码与长度校验
- `SentryInfo`/`RobotHurt` 位域
- CRTP 命令分发、列表外命令忽略、错误长度负载拒绝、跨多次 `feed()` 的半帧

---

## 8. 0x0201 现场测试

`robot_status_test_node` 是一个只订阅 `0x0201` 的诊断节点。它使用
`CommandList<cmd::RobotStatus>`，因此不会解析或处理其他命令；每收到一帧就打印各字段，
并发布到 `~/robot_status`。

```bash
# 终端 1：默认 /dev/ttyUSB0、115200
ros2 run rm_referee robot_status_test_node

# 终端 2：查看发布的 ROS 话题
ros2 topic echo /robot_status_test_node/robot_status
```

参数可覆盖：`ros2 run rm_referee robot_status_test_node --ros-args -p port:=/dev/ttyUSB0`。

终端输出示例（实测裁判系统 17 字节 `0x0201`）：

```text
0x0201 robot_status: id=3 level=1 hp=100/0 cooling=40 heat_limit=240 chassis_power_limit=80
  bullet_speed_limit=30.00 outputs[gimbal=1 chassis=1 shooter=1]
```

该节点按 2027 协议的 17 字节 `RobotStatus` 解码；如果裁判系统发送旧版 13 字节负载，
基类会打印长度不匹配告警并丢弃该帧，不会误解析。

---

## 9. 已知限制

- 只实现**标准链路**（115200，电源管理模块 TTL 口）；图传链路和雷达无线链路不在范围内。
- 不做应用层重传/确认；裁判系统本身也不提供。
- 没有“裁判系统在线”心跳判断，只有串口打开状态（见 5.4）。
- 一个串口设备只能由一个进程打开；若工作空间里其他节点（例如 STM32 转发节点）也占用
  同一 TTL 口，需要先做设备/链路拆分。
- `0x0301` 的 `user_data` 语义（子命令 ID）由队伍自定义，本包只做透明收发。
- 坐标单位以协议为准：`0x0203`、`0x020B` 为米，`0x0305` 为厘米，`0x0307` 为分米；
  `0x0203` 的朝向为度（正北 0）；包内不做单位换算。
