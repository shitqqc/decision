# 2027赛季上位机裁判系统串口驱动

## 快速开始

```bash

cd path/to/workspace
colcon build --symlink-install --packages-select rm_referee rm_referee_msg

source install/setup.sh
ros2 launch rm_referee referee.launch.py robot_type:=robot_type serial_port:=/dev/ttyUSB0 baudrate:=115200
# 目前支持robot_type: infentry, sentry, radar

```

>大肥鱼帮忙写的，目前测了0x0201能收到数据。由于目前裁判系统只有上赛季的固件，现在接裁判系统跑的时候部分命令码应该是要修改的。

## TODO

1. 测试别的命令码
2. 测试向裁判系统发送数据
