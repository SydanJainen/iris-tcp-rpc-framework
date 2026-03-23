"""Unit tests for the API spec generator."""

import json
import os
import sys
import tempfile

import pytest

# Add the tools directory to the path so we can import generate_spec
sys.path.insert(
    0,
    os.path.join(os.path.dirname(__file__), "..", "..", "tools"),
)

from generate_spec import generate_spec, parse_api_header


@pytest.fixture
def sample_api_header(tmp_path) -> str:
    """Create a temporary api.h with sample declarations."""
    content = """\
#ifndef IRIS_API_H
#define IRIS_API_H

#include <string>

#define API_EXPORT

API_EXPORT int add(int a, int b);
API_EXPORT std::string reverse(std::string s);
API_EXPORT double multiply(double a, double b);
API_EXPORT int fibonacci(int n);
API_EXPORT std::string to_base(int number, int base);
API_EXPORT std::string tfidf(std::string term);

// Non-exported helper
void helper();

#endif
"""
    path = tmp_path / "api.h"
    path.write_text(content, encoding="utf-8")
    return str(path)


@pytest.fixture
def header_with_unsupported_type(tmp_path) -> str:
    """Create a header with an unsupported type."""
    content = """\
#define API_EXPORT

API_EXPORT int add(int a, int b);
API_EXPORT std::vector<int> get_values(int count);
API_EXPORT double multiply(double a, double b);
"""
    path = tmp_path / "api_unsupported.h"
    path.write_text(content, encoding="utf-8")
    return str(path)


# ---------- parse sample api.h ----------


def test_parse_extracts_all_functions(sample_api_header: str) -> None:
    functions = parse_api_header(sample_api_header)
    names = [f["name"] for f in functions]
    assert names == ["add", "reverse", "multiply", "fibonacci", "to_base", "tfidf"]


def test_parse_extracts_correct_args(sample_api_header: str) -> None:
    functions = parse_api_header(sample_api_header)
    add_func = functions[0]
    assert add_func["name"] == "add"
    assert add_func["args"] == [
        {"name": "a", "type": "int"},
        {"name": "b", "type": "int"},
    ]
    assert add_func["returns"] == "int"


def test_parse_extracts_string_return(sample_api_header: str) -> None:
    functions = parse_api_header(sample_api_header)
    reverse_func = functions[1]
    assert reverse_func["returns"] == "string"
    assert reverse_func["args"] == [{"name": "s", "type": "string"}]


def test_parse_extracts_double(sample_api_header: str) -> None:
    functions = parse_api_header(sample_api_header)
    multiply_func = functions[2]
    assert multiply_func["returns"] == "double"
    assert multiply_func["args"] == [
        {"name": "a", "type": "double"},
        {"name": "b", "type": "double"},
    ]


def test_parse_ignores_non_exported(sample_api_header: str) -> None:
    functions = parse_api_header(sample_api_header)
    names = [f["name"] for f in functions]
    assert "helper" not in names


# ---------- output JSON structure ----------


def test_spec_structure(sample_api_header: str) -> None:
    functions = parse_api_header(sample_api_header)
    spec = generate_spec(functions)

    assert "version" in spec
    assert spec["version"] == "1.0"
    assert "generated_at" in spec
    assert "functions" in spec
    assert isinstance(spec["functions"], list)
    assert len(spec["functions"]) == 6


def test_spec_json_serializable(sample_api_header: str) -> None:
    functions = parse_api_header(sample_api_header)
    spec = generate_spec(functions)
    # Must not raise
    json_str = json.dumps(spec, indent=4)
    parsed = json.loads(json_str)
    assert parsed["version"] == "1.0"
    assert len(parsed["functions"]) == 6


# ---------- unsupported type warning ----------


def test_unsupported_type_skipped(
    header_with_unsupported_type: str, capsys: pytest.CaptureFixture
) -> None:
    functions = parse_api_header(header_with_unsupported_type)
    names = [f["name"] for f in functions]

    # get_values should be skipped due to unsupported return type
    assert "get_values" not in names
    # add and multiply should still be present
    assert "add" in names
    assert "multiply" in names

    # Check that a warning was printed to stderr
    captured = capsys.readouterr()
    assert "WARNING" in captured.err
    assert "get_values" in captured.err


# ---------- whitespace handling ----------


def test_parse_handles_extra_whitespace(tmp_path) -> None:
    """The parser should handle various whitespace patterns."""
    content = """\
#define API_EXPORT

  API_EXPORT   int   add( int  a ,  int  b ) ;
API_EXPORT    double   multiply(  double a,double b  );
"""
    path = tmp_path / "api_ws.h"
    path.write_text(content, encoding="utf-8")

    functions = parse_api_header(str(path))
    names = [f["name"] for f in functions]
    assert "add" in names
    assert "multiply" in names
