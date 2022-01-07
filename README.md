### 编译安装 (Compile & Install)
0. 安装依赖：
```
boost >= v1.75
openssl >= 1.0
```
1. 安装 xmake 构建工具 (Install `xmake` build utility)
``` bash
bash <(curl -fsSL https://xmake.io/shget.text) # curl
bash <(wget https://xmake.io/shget.text -O -)  # wget
```
2. 配置构建 (Setup and build)
``` bash
xmake --mode=debug/release/releasedbg --vendor-boost=/path/to/boost --vendor-openssl=/path/to/openssl
```

3. 安装 (Install)
``` bash
xmake install -o /target/path/to/install
```

