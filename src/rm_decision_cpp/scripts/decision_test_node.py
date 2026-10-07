#!/usr/bin/env python3
import json
import math
from copy import deepcopy
from pathlib import Path

import rclpy
import yaml
from decision_interfaces.msg import GameInfo
from nav_msgs.msg import Odometry
from rclpy.node import Node
from std_msgs.msg import String
from std_srvs.srv import SetBool

class ScenarioStore:
    def __init__(self, path):
        self.path = Path(path).expanduser().resolve()

    def read(self):
        return yaml.safe_load(self.path.read_text(encoding="utf-8"))

    def write(self, document):
        self.path.write_text(yaml.safe_dump(document, sort_keys=False), encoding="utf-8")

    def compose(self, document):
        current = document["current"]
        data = deepcopy(document["base"])
        for layer in (document["scenarios"][current["scenario"]], current.get("overrides", {})):
            for group, fields in layer.items():
                data[group].update(fields)
        return data


class DecisionTestNode(Node):
    def __init__(self):
        super().__init__("decision_test")
        self.declare_parameter("data_file", "")
        self.store = ScenarioStore(self.get_parameter("data_file").value)
        document = self.store.read()
        document["current"] = {"scenario": "normal", "overrides": {}}
        self.store.write(document)
        self.web_enabled = False
        self.error = ""
        self.file_mtime_ns = self.store.path.stat().st_mtime_ns
        self.game_pub = self.create_publisher(GameInfo, "/nav/game_info", 10)
        self.odom_pub = self.create_publisher(Odometry, "/odom", 10)
        self.state_pub = self.create_publisher(String, "/nav/decision_test/state", 10)
        self.load()
        self.create_service(SetBool, "/nav/decision_test/web_enabled", self.on_web_enabled)
        self.create_timer(0.1, self.tick)

    def load(self):
        document = self.store.read()
        data = self.store.compose(document)
        game = GameInfo()
        
        field_types = game.get_fields_and_field_types()
        for field, value in data["game_info"].items():
            type_name = field_types.get(field)
            if type_name == "boolean":
                value = bool(value)
            elif type_name.startswith("float"):
                value = float(value)
            elif type_name.startswith(("uint", "int")):
                value = int(value)
            setattr(game, field, value)
            
        pose = data["pose"]
        odom = Odometry()
        odom.header.frame_id = "odom"
        odom.child_frame_id = "base_link"
        odom.pose.pose.position.x = float(pose["x"])
        odom.pose.pose.position.y = float(pose["y"])
        odom.pose.pose.orientation.z = math.sin(float(pose["yaw"]) / 2)
        odom.pose.pose.orientation.w = math.cos(float(pose["yaw"]) / 2)
        
        self.document, self.data = document, data
        self.game, self.odom = game, odom
        self.error = ""
        self.file_mtime_ns = self.store.path.stat().st_mtime_ns
        self.publish()

    def on_web_enabled(self, request, response):
        self.web_enabled = request.data
        response.success = True
        self.publish_state()
        return response

    def check_file(self):
        try: 
            # 获取文件修改时间
            mtime_ns = self.store.path.stat().st_mtime_ns 
        except Exception as exc: 
            self.error = str(exc)
            self.get_logger().error(self.error)
            self.publish_state()
            return
        # 文件没有变化
        if mtime_ns == self.file_mtime_ns: 
            return
        # 文件发生变化，重新加载 
        self.file_mtime_ns = mtime_ns 
        try:
            self.load()
        except Exception as exc:
            self.error = str(exc)
            self.get_logger().error(self.error)
            self.publish_state()

    def publish_state(self):
        self.state_pub.publish(String(data=json.dumps({
            "data_file": str(self.store.path), "scenario": self.document["current"]["scenario"],
            "scenarios": list(self.document["scenarios"]), "data": self.data,
            "web_enabled": self.web_enabled, "error": self.error,
        })))

    def publish(self):
        self.odom.header.stamp = self.get_clock().now().to_msg()
        self.game_pub.publish(self.game)
        self.odom_pub.publish(self.odom)
        self.publish_state()

    def tick(self):
        self.check_file()
        self.publish()


def main():
    rclpy.init()
    node = None
    try:
        node = DecisionTestNode()
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        if node is not None:
            node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    main()
