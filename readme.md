#华南师范大学2027赛季决策开发仓库

包：`rm_decision_cpp`。**BTv4 多树**，原算法来源为 **北理导航开源**。

TODO:修改CheckRemoteExchangeCooldown实现、补上裁判系统串口、将导航移除决策工作空间、修改云台控制、根据规则重写全局模式、雷达决策闭环

---

### 运行

```bash
ros2 launch rm_decision_cpp run.launch.py namespace:=nav
ros2 launch rm_decision_cpp bt_monitor.launch.py
# 浏览器打开 http://127.0.0.1:8765/
```

### 测试

```bash
# 测试数据节点
ros2 launch rm_decision_cpp decision_test.launch.py

# 完整测试栈
ros2 launch rm_decision_cpp bt_test.launch.py
ros2 launch rm_decision_cpp bt_test.launch.py tree_xml_file:=test_tree.xml
```