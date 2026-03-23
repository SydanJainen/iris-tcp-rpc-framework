import json


class JsonSerializer:

    def serialize(self, obj: dict) -> bytes:
        return json.dumps(obj, ensure_ascii=False).encode("utf-8")

    def deserialize(self, data: bytes) -> dict:
        return json.loads(data.decode("utf-8"))
