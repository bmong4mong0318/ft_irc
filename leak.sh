#!/bin/bash
RED='\033[0;31m'
NC='\033[0m'
NAME="ircserv" # 여기 이름
TTY="/dev/"$(ps u | grep -E '[.]/'${NAME} | awk 'END {print $7}')
while :
do
	PID=$(ps u | grep -E '[.]/'${NAME} | awk '{print $2}' | xargs echo )
	IFS=' ' read -r -a pids <<< "${PID}"
	for pid in "${pids[@]}"
	do
		echo -e "${RED}PID : ${pid}${NC}   TTY : ${TTY}"
		leaks ${pid} | grep Process
	done
	echo "============================================================"
	sleep 2 # 여기 인터벌
done