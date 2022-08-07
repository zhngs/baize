#!/bin/bash
SCRIPT_DIR=$(cd `dirname $0`; pwd)
PROJECT_DIR=`cd $SCRIPT_DIR/..; pwd`
THIRD_PARTY_DIR=`cd $PROJECT_DIR/third_party; pwd`
FLAME_GRAPH_DIR=`cd $THIRD_PARTY_DIR/FlameGraph; pwd`
SVG_DIR="$PROJECT_DIR/build/svg"

# 使用perf测写相关程序的时候执行如下命令
# perf record -F 99 -g 执行相关程序

perf script > out.perf
$FLAME_GRAPH_DIR/stackcollapse-perf.pl out.perf > out.folded
$FLAME_GRAPH_DIR/flamegraph.pl out.folded > out.svg

rm perf.data
rm out.perf
rm out.folded

mkdir -p $SVG_DIR
mv out.svg $SVG_DIR