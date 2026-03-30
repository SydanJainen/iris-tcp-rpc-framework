"""Unit tests for the REPL module."""

import pytest

from src.repl import coerce_arg, coerce_args, parse_input, find_function_spec, BUILTIN_COMMANDS


SAMPLE_SPEC = {
    "version": "1.0",
    "functions": [
        {
            "name": "add",
            "args": [
                {"name": "a", "type": "int"},
                {"name": "b", "type": "int"},
            ],
            "returns": "int",
        },
        {
            "name": "multiply",
            "args": [
                {"name": "a", "type": "double"},
                {"name": "b", "type": "double"},
            ],
            "returns": "double",
        },
        {
            "name": "reverse",
            "args": [{"name": "s", "type": "string"}],
            "returns": "string",
        },
    ],
}


class TestParseInput:

    def test_simple_command(self) -> None:
        cmd, args = parse_input("add 1 2")
        assert cmd == "add"
        assert args == ["1", "2"]

    def test_command_only(self) -> None:
        cmd, args = parse_input("help")
        assert cmd == "help"
        assert args == []

    def test_empty_input(self) -> None:
        cmd, args = parse_input("")
        assert cmd == ""
        assert args == []

    def test_whitespace_only(self) -> None:
        cmd, args = parse_input("   ")
        assert cmd == ""
        assert args == []

    def test_leading_trailing_spaces(self) -> None:
        cmd, args = parse_input("  add  10  20  ")
        assert cmd == "add"
        assert args == ["10", "20"]


class TestCoercion:

    def test_int_coercion(self) -> None:
        assert coerce_arg("42", "int") == 42
        assert isinstance(coerce_arg("42", "int"), int)

    def test_double_coercion(self) -> None:
        assert coerce_arg("3.14", "double") == 3.14
        assert isinstance(coerce_arg("3.14", "double"), float)

    def test_string_passthrough(self) -> None:
        assert coerce_arg("hello", "string") == "hello"
        assert isinstance(coerce_arg("hello", "string"), str)

    def test_int_string_stays_int(self) -> None:
        """An integer-like string with type 'int' should become int."""
        assert coerce_arg("7", "int") == 7

    def test_coerce_args_with_spec(self) -> None:
        spec_args = [
            {"name": "a", "type": "int"},
            {"name": "b", "type": "int"},
        ]
        result = coerce_args(["10", "20"], spec_args)
        assert result == [10, 20]
        assert all(isinstance(v, int) for v in result)

    def test_coerce_args_mixed_types(self) -> None:
        spec_args = [
            {"name": "a", "type": "double"},
            {"name": "s", "type": "string"},
        ]
        result = coerce_args(["3.14", "hello"], spec_args)
        assert result == [3.14, "hello"]

    def test_invalid_int_raises_value_error(self) -> None:
        with pytest.raises(ValueError):
            coerce_arg("not_a_number", "int")


class TestBuiltinCommands:

    def test_help_is_builtin(self) -> None:
        assert "help" in BUILTIN_COMMANDS

    def test_quit_is_builtin(self) -> None:
        assert "quit" in BUILTIN_COMMANDS

    def test_exit_is_builtin(self) -> None:
        assert "exit" in BUILTIN_COMMANDS

    def test_history_is_builtin(self) -> None:
        assert "history" in BUILTIN_COMMANDS

    def test_metrics_is_builtin(self) -> None:
        assert "metrics" in BUILTIN_COMMANDS

    def test_add_is_not_builtin(self) -> None:
        assert "add" not in BUILTIN_COMMANDS


class TestFindFunctionSpec:

    def test_finds_existing_function(self) -> None:
        fn = find_function_spec(SAMPLE_SPEC, "add")
        assert fn is not None
        assert fn["name"] == "add"

    def test_returns_none_for_unknown(self) -> None:
        fn = find_function_spec(SAMPLE_SPEC, "nonexistent")
        assert fn is None
