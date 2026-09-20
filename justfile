GENERATOR := "Ninja"
JOBS := `sysctl -n hw.logicalcpu`

build:
    cd build && ninja -j{{ JOBS }}

run:
    just config
    just build
    clear
    ./build/revolve

config:
    mkdir -p build
    cd build && cmake -G "{{ GENERATOR }}"  \
           -DCMAKE_C_COMPILER=/usr/bin/clang \
           -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
           -DCMAKE_C_COMPILER_LAUNCHER= \
           -DCMAKE_CXX_COMPILER_LAUNCHER= \
           -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
           ..

clangd:
    mkdir -p build
    cd build && cmake -G "{{ GENERATOR }}" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_C_COMPILER_LAUNCHER= \
        -DCMAKE_CXX_COMPILER_LAUNCHER= \
        ..
    ln -sf build/compile_commands.json compile_commands.json
