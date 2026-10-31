# 云台控制指令
# enable: 云台控制开关。true = 决策接管云台，按 yaw/pitch 转动；false = 交还控制权，yaw/pitch 不生效。
bool enable
# yaw/pitch: 目标云台角，仅在 enable = true 时有效。
float32 yaw
float32 pitch
