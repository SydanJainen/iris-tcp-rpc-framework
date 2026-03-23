import pytest

from src.container import Container


class TestContainerRegisterResolve:
    def test_resolve_returns_registered_value(self) -> None:
        c = Container()
        c.register("greeting", lambda: "hello")
        assert c.resolve("greeting") == "hello"

    def test_resolve_returns_complex_object(self) -> None:
        c = Container()
        c.register("config", lambda: {"host": "localhost", "port": 5555})
        cfg = c.resolve("config")
        assert cfg["host"] == "localhost"
        assert cfg["port"] == 5555


class TestContainerSingleton:
    def test_singleton_returns_same_instance(self) -> None:
        c = Container()
        c.register("config", lambda: {"host": "localhost"}, singleton=True)

        first = c.resolve("config")
        second = c.resolve("config")
        assert first is second

    def test_singleton_factory_called_once(self) -> None:
        call_count = 0

        def factory():
            nonlocal call_count
            call_count += 1
            return {"count": call_count}

        c = Container()
        c.register("counter", factory, singleton=True)

        c.resolve("counter")
        c.resolve("counter")
        c.resolve("counter")

        assert call_count == 1


class TestContainerFactory:

    def test_factory_creates_new_instance_each_time(self) -> None:
        c = Container()
        c.register("obj", lambda: {"data": "fresh"})

        first = c.resolve("obj")
        second = c.resolve("obj")
        assert first is not second

    def test_factory_calls_callable_each_time(self) -> None:
        call_count = 0

        def factory():
            nonlocal call_count
            call_count += 1
            return call_count

        c = Container()
        c.register("seq", factory)

        assert c.resolve("seq") == 1
        assert c.resolve("seq") == 2
        assert c.resolve("seq") == 3


class TestContainerError:

    def test_resolve_unknown_raises_key_error(self) -> None:
        c = Container()
        with pytest.raises(KeyError, match="nonexistent"):
            c.resolve("nonexistent")


class TestContainerHas:

    def test_has_returns_true_for_registered(self) -> None:
        c = Container()
        c.register("x", lambda: 42)
        assert c.has("x") is True

    def test_has_returns_false_for_unregistered(self) -> None:
        c = Container()
        assert c.has("missing") is False


class TestContainerMixed:

    def test_singleton_and_factory_coexist(self) -> None:
        c = Container()
        c.register("db", lambda: {"conn": "pg://db"}, singleton=True)

        seq = 0

        def request_factory():
            nonlocal seq
            seq += 1
            return {"req_id": seq}

        c.register("request", request_factory)

        db1 = c.resolve("db")
        db2 = c.resolve("db")
        assert db1 is db2

        r1 = c.resolve("request")
        r2 = c.resolve("request")
        assert r1["req_id"] == 1
        assert r2["req_id"] == 2

    def test_re_register_clears_singleton_cache(self) -> None:
        c = Container()
        c.register("val", lambda: "first", singleton=True)
        assert c.resolve("val") == "first"

        c.register("val", lambda: "second", singleton=True)
        assert c.resolve("val") == "second"
