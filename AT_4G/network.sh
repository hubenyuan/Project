#!/bin/bash

ppp_pid=$(pgrep -x ppp-4G)

echo "ppp-4G进程的PID为: $ppp_pid"

# 定义信号处理函数
handle_eth0_wlan0_status()
{
    # 判断网络状态
    eth0_status=$(ping -c 1 -W 1 -I eth0 baidu.com >/dev/null 2>&1 && echo "good" || echo "bad")
    wlan0_status=$(ping -c 1 -W 1 -I wlan0 baidu.com >/dev/null 2>&1 && echo "good" || echo "bad")
    ppp0_status=$(ping -c 1 -W 1 -I ppp0 baidu.com >/dev/null 2>&1 && echo "good" || echo "bad")

    # 根据网络状态发送信号给主程序
	if [ $eth0_status = "good" ]; then
		
		# 判断是否设置了默认网关
		eth0_gateway=$(ip route show default | grep -w wlan0 | awk '{print $3}')

		# 如果eth0_gateway为空
		if [ -z "$eth0_gateway" ]; then
			# 获取eth0的IP地址
			eth0_address=$(ip addr show eth0 | grep -w inet | awk '{print $2}' | awk -F '/' '{print $1}' | head -n 1)
		
		    # 添加默认路由
		    $(route add default gw $eth0_address)
		else
			echo "eth0_gateway already existed"
		fi

		# 判断ppp0网络
		if [ $ppp0_status = "good" ]; then
			echo "Close pppd dialing"
		$(kill -USR2 $ppp_pid)
	    else
			echo "without ppp0 network"
		fi
    else
		if [ $wlan0_status = "good" ]; then
			# 判断是否设置了默认网关
			wlan0_gateway=$(ip route show default | grep -w wlan0 | awk '{print $3}')

			if [ -z "$wlan0_gateway" ]; then
				# 获取wlan0的IP地址
				wlan0_address=$(ip addr show wlan0 | grep -w inet | awk '{print $2}' | awk -F '/' '{print $1}' | head -n 1)

				# 添加默认路由
				$(route add default gw $wlan0_address)
			else
				echo "wlan0_gateway already existed"
			fi

			# 判断ppp0网络
			if [ $ppp0_status = "good" ]; then
				echo "Close pppd dialing"
				$(kill -USR2 $ppp_pid)
			else
				echo "without ppp0 network"
			fi

		else
			if [ $ppp0_status = "good" ]; then
				# 判断是否设置了默认网关
				ppp0_gateway=$(ip route show default | grep -w wlan0 | awk '{print $3}')

				if [ -z "$wlan0_gateway" ]; then
					# 获取ppp0的IP地址
					ppp0_address=$(ip addr show wlan0 | grep -w inet | awk '{print $2}' | awk -F '/' '{print $1}' | head -n 1)

					# 添加默认路由
					$(route add default gw $ppp0_address)
				else
					echo "ppp0_gateway already existed"
				fi
			else
				echo "Starting pppd dialing"
				$(kill -USR1 $ppp_pid)
			fi
		fi
    fi
}

echo "Checking wlan0 and eth0 network status"
handle_eth0_wlan0_status

