#!/bin/sh

# please provide raylib/ with
# emmake make -Craylib/src PLATFORM=PLATFORM_WEB BUILD_MODE=RELEASE
# no modification to source is required

em++ \
    -pipe -std=c++20 -O2 -flto=auto -Wall -Wextra -Wpedantic \
    --embed-file safety-first@ \
    --shell-file raylib/src/shell.html \
    -s INITIAL_MEMORY=64MB \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s STACK_SIZE=4MB \
    -s MAX_WEBGL_VERSION=2 \
    -s MIN_WEBGL_VERSION=2 \
    -s ASYNCIFY=1 \
    -s EXPORTED_RUNTIME_METHODS='["requestFullscreen"]' \
    -s USE_WEBGL2=1 \
    -s USE_GLFW=3 \
    -s ASSERTIONS=0 \
    -s SAFE_HEAP=0 \
    -s STACK_OVERFLOW_CHECK=0 \
    -s EMIT_EMSCRIPTEN_METADATA=0 \
    -s FORCE_FILESYSTEM=0 \
    -Iraylib/src \
    -D_FORTIFY_SOURCE=3 \
    -DPLATFORM_WEB \
    -o torment-nexus-i.html \
    main.cpp \
    raylib/src/libraylib.web.a
