# bit 0：是否确认复活（1=确认复活）
bool resurrection_en
# bit 1：是否确认兑换立即复活（1=确认兑换）
bool buy_resurrection_en
# bit 2-12：将要兑换的发弹量值，单调递增
uint16 buy_projectile_allowance
# bit 13-16：远程兑换发弹量请求次数，单调递增且每次仅能+1
uint16 buy_projectile_times
# bit 17-20：远程兑换血量请求次数，单调递增且每次仅能+1
uint16 buy_hp_times
# bit 21-22：姿态指令 1=进攻 2=防御 3=移动
uint8 posture_cmd
# bit 23：是否确认使能量机关进入正在激活状态（1=确认）
bool energy_activation_confirm
# 启用超电
bool use_super_cap
# 是否小相机控制（独立 bool 字段，不在 sentry_cmd 位域内）
bool small_camera_control