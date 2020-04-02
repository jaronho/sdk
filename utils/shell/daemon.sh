#!/bin/sh

# 客户端路径
CLIENT_PATH=/opt/kanzi
# 客户端进程名
PROCESS_CLIENT=theme1.exe
# 服务端路径
SERVER_PATH=/opt
# 服务端进程名
PROCESS_SERVER=m11_server
# 当前文件目录
FILEPATH=$(cd "$(dirname "$0")"; pwd)
# 延迟时间(秒数,可以为小数)
SLEEP_TIME=1
# 循环最大次数
LOOP_MAX_COUNT=2
# 循环当前次数
LOOP_COUNT=1
# 循环次数步进
LOOP_STEP=0
# 进入循环体(当前次数大于最大次数时退出循环)
while [ $LOOP_COUNT -le $LOOP_MAX_COUNT ]
do
    ########################################
    # 打印当前时间
    # NOWTIME=$(date +%Y-%m-%d)' '$(date +%H:%M:%S)
    # echo [$NOWTIME]
    ########################################
    # 监听客户端进程
    PID_CLIENT=$(pidof $PROCESS_CLIENT)
    if [ ! $PID_CLIENT ]; then
        if [ ! -f "$CLIENT_PATH/$PROCESS_CLIENT" ]; then
            echo ========== $CLIENT_PATH/$PROCESS_CLIENT = FILE NOT EXIST
        else
            echo ========== $CLIENT_PATH/$PROCESS_CLIENT = RESTART
            cd $CLIENT_PATH
            ./$PROCESS_CLIENT &
            cd $FILEPATH
        fi
    fi
    # 监听服务端进程
    PID_SERVER=$(pidof $PROCESS_SERVER)
    if [ ! $PID_SERVER ]; then
        if [ ! -f "$SERVER_PATH/$PROCESS_SERVER" ]; then
            echo ========== $SERVER_PATH/$PROCESS_SERVER = FILE NOT EXIST
        else
            echo ========== $SERVER_PATH/$PROCESS_SERVER = RESTART
            cd $SERVER_PATH
            ./$PROCESS_SERVER &
            cd $FILEPATH
        fi
    fi
    ########################################
    # 计数及延迟
    LOOP_COUNT=$(($LOOP_COUNT + $LOOP_STEP))
    sleep $SLEEP_TIME
    ########################################
done
