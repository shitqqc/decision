#!/usr/bin/env python3
"""Publish virtual GameInfo + odom for rm_decision_cpp offline testing."""

from __future__ import annotations

import argparse
import math

import rclpy
from geometry_msgs.msg import Quaternion
from nav_msgs.msg import Odometry
from rclpy.node import Node

from decision_interfaces.msg import GameInfo


SCENARIOS = (
    "normal",      # in match, healthy, patrol
    "low_ammo",     # ammo low -> retreat home
    "low_hp",       # hp low engaged -> defend / retreat
    "engaged",      # combat attack stance
    "attack_fort",  # big energy + late game -> attack tactical
    "manual",       # manual control + fort goal
    "dead",         # hp=0 revive
    "supply",       # in supply zone buy ammo
    "cycle",        # rotate scenarios every --period seconds
)


def yaw_to_quat(yaw: float) -> Quaternion:
    q = Quaternion()
    q.z = math.sin(yaw * 0.5)
    q.w = math.cos(yaw * 0.5)
    return q


class MockDecisionPub(Node):
    def __init__(self, scenario: str, period_s: float, ns: str) -> None:
        super().__init__("mock_decision_pub")
        self._scenario = scenario
        self._period_s = period_s
        self._t0 = self.get_clock().now()
        self._tick = 0

        game_topic = f"{ns}/game_info" if ns else "game_info"
        odom_topic = "/odom"
        self._game_pub = self.create_publisher(GameInfo, game_topic, 10)
        self._odom_pub = self.create_publisher(Odometry, odom_topic, 10)
        self.create_timer(0.1, self._on_timer)
        self.get_logger().info(
            f"mock pub: scenario={scenario} game={game_topic} odom={odom_topic}"
        )

    def _elapsed(self) -> float:
        return (self.get_clock().now() - self._t0).nanoseconds * 1e-9

    def _active_scenario(self) -> str:
        if self._scenario != "cycle":
            return self._scenario
        names = [s for s in SCENARIOS if s != "cycle"]
        idx = int(self._elapsed() / self._period_s) % len(names)
        return names[idx]

    def _fill_base(self, msg: GameInfo) -> None:
        msg.current_hp = 400
        msg.game_progress = 4  # in match
        msg.game_remaining_time = 350
        msg.remaining_energy = 50
        msg.ally_outpost_hp = 1500
        msg.ally_base_hp = 5000
        msg.hurt_hp_deduction_reason = 0
        msg.posture = 3
        msg.out_of_combat = True
        msg.redeemable_17mm = 200
        msg.bullet_remaining_17mm = 300
        msg.energy_activatable = False
        msg.coin_remaining = 400
        msg.enemy_outpost_hp = 1200
        msg.enemy_base_hp = 5000
        msg.current_heat = 20
        msg.heat_limit = 240
        msg.big_energy_status = 0
        msg.control_mode = 0
        msg.manual_goal_x = 0.0
        msg.manual_goal_y = 0.0
        msg.can_free_resurrect = True

    def _apply_scenario(self, name: str, msg: GameInfo) -> tuple[float, float, float]:
        """Return pose x,y,yaw for odom."""
        self._fill_base(msg)
        x, y, yaw = 5.0, 5.0, 0.0

        if name == "normal":
            msg.game_remaining_time = 380
            msg.out_of_combat = True
            x, y = 12.0, 5.0
        elif name == "low_ammo":
            msg.bullet_remaining_17mm = 30
            msg.game_remaining_time = 300
            x, y = 10.0, 6.0
        elif name == "low_hp":
            msg.current_hp = 35
            msg.out_of_combat = False
            msg.game_remaining_time = 280
            x, y = 14.0, 8.0
        elif name == "engaged":
            msg.current_hp = 280
            msg.out_of_combat = False
            msg.current_heat = 80
            msg.game_remaining_time = 260
            x, y = 15.0, 10.0
        elif name == "attack_fort":
            msg.big_energy_status = 1
            msg.energy_activatable = True
            msg.game_remaining_time = 90
            msg.out_of_combat = True
            x, y = 18.0, 7.0
        elif name == "manual":
            msg.control_mode = 1
            msg.manual_goal_x = 22.0
            msg.manual_goal_y = 7.5
            msg.game_remaining_time = 200
            x, y = 8.0, 4.0
        elif name == "dead":
            msg.current_hp = 0
            msg.can_free_resurrect = True
            msg.game_remaining_time = 250
            x, y = 6.0, 3.0
        elif name == "supply":
            msg.bullet_remaining_17mm = 80
            msg.coin_remaining = 250
            msg.game_remaining_time = 320
            # own_supply zone from zones.yaml: min_x 1.5 max_x 3.8 min_y 0 max_y 4.4
            x, y = 2.5, 2.0
        else:
            self.get_logger().warn(f"unknown scenario {name}, using normal")

        # slight motion so pose looks alive
        phase = self._elapsed() * 0.4
        x += 0.15 * math.sin(phase)
        y += 0.15 * math.cos(phase)
        yaw = phase * 0.2
        return x, y, yaw

    def _on_timer(self) -> None:
        name = self._active_scenario()
        game = GameInfo()
        x, y, yaw = self._apply_scenario(name, game)
        self._game_pub.publish(game)

        odom = Odometry()
        odom.header.stamp = self.get_clock().now().to_msg()
        odom.header.frame_id = "odom"
        odom.child_frame_id = "base_link"
        odom.pose.pose.position.x = x
        odom.pose.pose.position.y = y
        odom.pose.pose.orientation = yaw_to_quat(yaw)
        self._odom_pub.publish(odom)

        self._tick += 1
        if self._tick % 20 == 1:
            self.get_logger().info(
                f"[{name}] hp={game.current_hp} ammo={game.bullet_remaining_17mm} "
                f"t={game.game_remaining_time} pose=({x:.1f},{y:.1f}) "
                f"mode={game.control_mode} energy={game.big_energy_status}"
            )


def main() -> None:
    parser = argparse.ArgumentParser(description="Mock GameInfo + odom for decision BT")
    parser.add_argument(
        "--scenario",
        default="cycle",
        choices=list(SCENARIOS),
        help="fixed scenario or cycle through all",
    )
    parser.add_argument(
        "--period",
        type=float,
        default=8.0,
        help="seconds per scenario when --scenario=cycle",
    )
    parser.add_argument(
        "--ns",
        default="/nav",
        help="namespace for game_info (default /nav -> /nav/game_info)",
    )
    args, ros_args = parser.parse_known_args()

    ns = args.ns.rstrip("/")
    if ns == "/":
        ns = ""

    rclpy.init(args=ros_args)
    node = MockDecisionPub(args.scenario, args.period, ns)
    try:
        rclpy.spin(node)
    except (KeyboardInterrupt, Exception):
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    main()
