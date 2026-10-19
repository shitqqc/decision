#华南师范大学2027赛季决策开发仓库

包：`rm_decision_cpp`。**BTv4 多树**，原算法来源为 ****。

TODO:将读取位姿和计算在哪个区域功能删除并从判断条件中移除、补齐云台控制、补完defend全局模式、

---

### 运行

```bash
ros2 launch rm_decision_cpp run.launch.py namespace:=nav
ros2 launch rm_decision_cpp bt_monitor.launch.py
# 浏览器打开 http://127.0.0.1:8765/
```
