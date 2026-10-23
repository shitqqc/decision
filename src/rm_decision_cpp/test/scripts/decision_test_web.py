import json
import threading
import time

from std_msgs.msg import String
from std_srvs.srv import SetBool

from decision_test_node import ScenarioStore

class TestWebBridge:
    def __init__(self, node):
        self.lock = threading.Lock()
        self.operation_lock = threading.Lock()
        self.latest = {}
        self.received = 0.0
        self.subscription = node.create_subscription(
            String, "/nav/decision_test/state", self.on_state, 10)
        self.enable_client = node.create_client(SetBool, "/nav/decision_test/web_enabled")

    def on_state(self, message): 
        with self.lock:
            self.latest = json.loads(message.data)
            self.received = time.monotonic()

    def state(self): 
        with self.lock:
            return {**self.latest, "online": time.monotonic() - self.received < 2.0}

    # 调用测试节点服务
    def call(self, client, request): 
        if not client.wait_for_service(timeout_sec=1.0):
            raise RuntimeError("Test node is offline")
        done = threading.Event()
        future = client.call_async(request)
        future.add_done_callback(lambda result: done.set())
        if not done.wait(3.0):
            raise RuntimeError("Test node request timed out")
        response = future.result()
        if not response.success:
            raise RuntimeError(response.message)

    # 串行处理 Web 端测试操作，避免多个操作同时修改测试数据
    def operate(self, action, payload):
        with self.operation_lock:
            state = self.state()
            if not state["online"]:
                raise RuntimeError("Test node is offline")
            if action == "enabled":
                self.call(self.enable_client, SetBool.Request(data=payload["enabled"]))
                return
            if not state.get("web_enabled"):
                raise RuntimeError("Web editing is disabled")

            store = ScenarioStore(state["data_file"])
            document = store.read()
            current = document["current"]

            if action == "scenario":
                current["scenario"] = payload["scenario"]
                current["overrides"] = {}
            elif action == "apply":
                overrides = current.setdefault("overrides", {})
                for group, fields in payload["changes"].items():
                    overrides.setdefault(group, {}).update(fields)
            elif action == "reset":
                current["overrides"] = {}
            else:
                raise RuntimeError("Unknown test operation")

            store.write(document)
