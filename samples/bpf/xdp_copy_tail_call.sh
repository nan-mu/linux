#!/bin/sh
set -e

# 1. 配置参数
SRC="xdp_copy_tail_call.c"
OBJ="xdp_copy_tail_call.o"
IFACE="${1:-eth0}"        # 默认网卡名，可通过第一个参数指定
PARENT_SEC="parent"
CHILD_SEC="child"
PROG_ARRAY_MAP="prog_array"

# 2. 检查依赖
for bin in clang llc bpftool ip; do
    command -v $bin >/dev/null 2>&1 || { echo "缺少 $bin，请先安装！"; exit 1; }
done

# 3. 编译XDP程序
echo "[*] 编译 $SRC ..."
clang -O2 -g -target bpf -c "$SRC" -o "$OBJ"

# 4. 卸载已有XDP程序（如果有）
echo "[*] 卸载 $IFACE 上已有XDP程序（如果有）..."
ip link set dev "$IFACE" xdp off || true

# 5. 使用 bpftool 加载 parent 和 child
echo "[*] 加载 parent 和 child XDP程序 ..."
bpftool prog load "$OBJ" "/sys/fs/bpf/xdp_parent" type xdp \
    pinmaps /sys/fs/bpf/xdp_maps
bpftool prog load "$OBJ" "/sys/fs/bpf/xdp_child" type xdp \
    pinmaps /sys/fs/bpf/xdp_maps

# 6. 获取 prog_array map id
MAP_ID=$(bpftool map show | grep "$PROG_ARRAY_MAP" | awk '{print $1}' | sed 's/id://')
if [ -z "$MAP_ID" ]; then
    echo "未找到 $PROG_ARRAY_MAP map，请检查程序定义"
    exit 1
fi

# 7. 获取 child 程序 fd
CHILD_FD=$(bpftool prog show | grep xdp_child | awk '{print $1}' | sed 's/id://')
if [ -z "$CHILD_FD" ]; then
    echo "未找到 child 程序，请检查编译和加载"
    exit 1
fi

# 8. 将 child 程序插入 prog_array[0]
echo "[*] 将 child 加入 prog_array[0] ..."
bpftool map update id $MAP_ID key 0 0 0 0 value $CHILD_FD 0 0 0

# 9. 将 parent 挂载到网卡
echo "[*] 挂载 parent 到 $IFACE ..."
ip link set dev "$IFACE" xdp obj "$OBJ" sec "$PARENT_SEC"

echo "[*] 完成！你可以用 ping 测试 XDP 行为。"
