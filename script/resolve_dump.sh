#!/bin/bash

# scripts/resolve.sh
# 用法: ./resolve.sh [crash.log] [binary_name]

LOG_FILE="${1:-crash.log}"
BINARY="${2:-crash_demo}"

if [ ! -f "$LOG_FILE" ]; then
    echo "❌ Crash log not found: $LOG_FILE"
    exit 1
fi

if [ ! -f "$BINARY" ]; then
    echo "❌ Binary not found: $BINARY"
    exit 1
fi

echo "🔍 解析崩溃堆栈 ($LOG_FILE) -> 源码行号"
echo "========================================"

grep "\[0x" "$LOG_FILE" | while read line; do
    if [[ "$line" == *"$BINARY"* ]]; then
        # 提取符号名和偏移量（在括号内的部分）
        symbol_part=$(echo "$line" | grep -o '([^)]*)' | head -n1 | tr -d '()')
        if [ -n "$symbol_part" ]; then
            # 提取符号名（去掉+offset部分）
            symbol=$(echo "$symbol_part" | sed 's/\+.*//')
            # 提取偏移量
            offset=$(echo "$symbol_part" | grep -o '\+[0-9a-fA-Fx]*' | head -n1)
            
            echo ">>> 符号: $symbol$offset"
            echo "    原始: $line"
            echo -n "    解析: "
            addr2line -e "$BINARY" -f -C -i "$symbol"
            echo ""
        fi
    fi
done

echo "✅ 解析完成！"
