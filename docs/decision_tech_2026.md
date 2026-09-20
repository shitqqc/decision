# 2026 赛季决策框架目标

包：`rm_decision_cpp`。**BTv4 多树**，算法来源为 **navi_minco_bit 全量树**（resource / tactical / nav+recovery / stance / gimbal）。下行仍为 `SentryCmd` + `/nav/use_spin` + Nav2 `NavigateToPose`。

**不做（缺基础设施时节点 stub FAILURE）**：云台硬件 egress、隧道 cmd_vel / 换图 LoadMap、雷达 `enemies_info` 闭环。

---

## 基础功能

### 1. 多树编排与只读可视化

Tick：`resource → tactical → nav → stance → gimbal`。

监控默认用 **BT Monitor（Web）**：决策每 tick 发 `bt_snapshot`（五树节点状态 + 黑板 JSON），浏览器以方框节点 + 箭头连线只读展示当前路径与数值。**不做改树**。

节点实现按 bit 目录拆分：`bt/action/*`、`bt/condition/*`，在 `BtEngine::registerNodes_` 内注册（含 bit 原名别名），不单开 `register_nodes.cpp`。

### 节点移植状态（相对 navi_minco_bit）

| 类别 | 状态 |
|------|------|
| 资源/战术/导航设点/基础姿态陀螺/Nav2 | 已接 BbKey + SentryCmd / use_spin |
| 远程兑换弹药/血量 | 写 `buy_projectile_times` / `buy_hp_times` |
| 前哨/高地/英雄守卫等纯黑板条件 | 已实现（依赖 GameInfo 字段） |
| 强化姿态 4–6 | 可写黑板；egress 钳制到 posture 1–3 |
| 云台 Track/SetGimbal* | 只写黑板；**无硬件 egress** |
| 隧道/过洞/换图/楼梯下降 | **stub FAILURE**，待 LoadMap / cmd_vel / 队友 ingress |

```bash
ros2 launch rm_decision_cpp run.launch.py namespace:=nav
ros2 launch rm_decision_cpp bt_monitor.launch.py
# 浏览器打开 http://127.0.0.1:8765/
```

话题默认：`/nav/bt_snapshot`（`std_msgs/String` JSON）。

### 2. 裁判与位姿入口

- `IRefereeIngress` ← 扩展后的 `GameInfo`
- `IPoseIngress` ← 里程计，写 `current_pose_*`，供区域判定

### 3. 区域与航点

`config/zones.yaml`（RMUC 默认）：`nav_points` + `own_supply` / `own_outpost` / `enemy_fort` 等。

### 4. 导航 / 姿态 / 资源

与 bit 树一致：人工点、低血低弹回 HOME、attack→敌堡、时间窗前哨与巡逻；姿态交战/热量/防御/过洞；陀螺与敌堡超电；复活与补给区买弹。

---

## 输入 / 输出契约

| 方向 | 话题 / 动作 | 说明 |
|------|-------------|------|
| 入 | `game_info` | 扩展 `GameInfo` |
| 入 | `odom`（可配） | 位姿 |
| 出 | `sentry_cmd` | 黑板驱动 |
| 出 | `/nav/use_spin` | 陀螺 |
| 出 | `navigate_to_pose` | Nav2 |
| 出 | `bt_snapshot` | 只读监控 JSON |

---

## 技术方案

```text
GameInfo + Odom → Blackboard + ZoneMap
  → resource / tactical / nav(+recovery) / stance / gimbal
  → SentryCmd + use_spin + NavigateToPose
  → bt_snapshot → Web BT Monitor
```
