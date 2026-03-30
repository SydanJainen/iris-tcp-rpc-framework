# Stage 1: Build
FROM gcc:13 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
        cmake make git ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

COPY server/CMakeLists.txt ./
COPY server/src/ src/
COPY server/tests/ tests/

RUN cmake -B out -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++ -static" \
    && cmake --build out --target iris_server -j"$(nproc)"

# Stage 2: Runtime
FROM debian:bookworm-slim

WORKDIR /app

COPY --from=builder /build/out/iris_server .
COPY api_spec.json .
COPY server/data/ server/data/

EXPOSE 5555

ENTRYPOINT ["./iris_server"]
