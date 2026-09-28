# rm_decision_cpp

2027 赛季哨兵决策：**BehaviorTree.CPP v4 多树**，算法来源 **navi_minco_bit 全量树**。下行：`SentryCmd` + `/nav/use_spin` + Nav2。

只读可视化用 Web BT Monitor（不做改树）。

节点按 bit 拆在 `bt/action/`、`bt/condition/`；在 `BtEngine::registerNodes_` 内注册（与 bit `SentryBTManager::registerNodes` 同风格，不单开注册文件）。

说明：[readme.md](../../readme.md)

## 架构

```text
application/     DecisionApp + BtEngine（含节点注册）
interface/       IRefereeIngress / IPoseIngress / ICommandEgress
infrastructure/  GameInfo / Odom / SentryCmd / BtSnapshotPublisher
domain/          blackboard keys / ZoneMap / zones.yaml
bt/action/       bit 移植动作节点
bt/condition/    bit 移植条件节点
tree/            resource / tactical / nav(+recovery) / stance / gimbal
web/             BT Monitor 静态页
```

## 运行

```bash
ros2 launch rm_decision_cpp run.launch.py namespace:=nav
ros2 launch rm_decision_cpp bt_monitor.launch.py
# http://127.0.0.1:8765/
```

## 虚拟数据测试

无真机时发布假 `game_info` + `/odom`：

```bash
# 默认 8s 轮换：normal / low_ammo / low_hp / engaged / attack_fort / manual / dead / supply
ros2 launch rm_decision_cpp mock_decision.launch.py

# 固定某一场景
ros2 launch rm_decision_cpp mock_decision.launch.py scenario:=attack_fort
```
