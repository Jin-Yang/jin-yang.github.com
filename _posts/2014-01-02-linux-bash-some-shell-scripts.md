---
title: Bash 常用脚本
layout: post
comments: true
language: chinese
category: [program,linux,misc]
keywords: bash
description: 简单整理下常用的脚步。
---

简单整理下常用的脚步。

<!-- more -->

<!--
# 1111# */5 * * * * root /usr/bin/flock -xn /var/run/check_resource.sh.lock -c 'xxxx' 2>&1 >/dev/null

LOGLVL=0
if [[ $# -gt 1 && "x$1" == "xlog" ]]; then
        LOGLVL=1
fi

AGENTS=(
)
AGENTS_LIMIT_MEM=(409600 409600 409600)
AGENTS_LIMIT_CPU=(80 80 80)

AGENTS_NUM=${#AGENTS[@]}
if [[ ${AGENTS_NUM} != ${#AGENTS_LIMIT_CPU[@]} || ${AGENTS_NUM} != ${#AGENTS_LIMIT_MEM[@]} ]]; then
        exit 1
fi

LAST_UPTIME=""
LAST_PROTIME=""
HERTZ=`getconf CLK_TCK 2>&1`
PAGESIZE=`getconf PAGESIZE 2>&1`
CPUUSAGE=0   # %
MEMUSAGE=0   # KB

function log_info()
{
        if [[ ${LOGLVL} -le 0 ]]; then
                return
        fi
        DATE_N=`date "+%Y-%m-%d %H:%M:%S"`
        echo "${DATE_N} [INFO] $@"
}

function integer_valid_check() {
        # shouldn't fail, or quit immediately.
        for var in "$@"; do
                if [[ "x${var}" == "x" || ! "${var}" =~ ^[0-9]*$ ]]; then
                        exit 1
                fi
        done
}
# Fetch the process ID.
# NOTE: In some scenarios, multi process would get, but only the process whoes parent
#       process ID is 1 will be valid for us.
function get_process_id() {
        if [[ "x${1}" == "x" ]]; then
                return 1
        fi

        pids=`timeout -k 5 -s INT 3 pidof "${1}" 2>&1`
        if [[ "x${pids}" == "x" ]]; then
                return 1
        fi

        for pid in ${pids}; do
                if [[ "x${pid}" == "x" || ! "${pid}" =~ ^[0-9]*$ ]]; then
                        continue
                fi

                ppid=`awk '{ print $4 }' "/proc/${pid}/stat" 2>&1`
                if [[ "x${ppid}" == "x" || "${ppid}" != "1" ]]; then
                        continue
                fi

                actexe=`readlink "/proc/${pid}/exe"`
                if [[ "${actexe}" != "${1}" ]]; then
                        continue
                fi
                echo "${pid}"
                return 0
        done

        return 1
}

function first_get_process_info() {
        bootime=`awk '{ print $1 }' /proc/uptime 2>&1`
        if [[ "x${bootime}" == "x" || ! "${bootime}" =~ ^[0-9.]*$ ]]; then
                exit 1
        fi
        LAST_UPTIME=${bootime}

        LAST_PROTIME=`awk '{ print($14+$15) }' "/proc/${1}/stat" 2>&1`
        integer_valid_check "${LAST_PROTIME}"
}
# get the cpu usage and memory for a specified process.
# NOTE: check this through top command instead of ps.
#       Ex. top -d 1 -p <PID>
function get_usage() {
        bootime=`awk '{ print $1 }' /proc/uptime 2>&1`
        if [[ "x${bootime}" == "x" || ! "${bootime}" =~ ^[0-9.]*$ ]]; then
                exit 1
        fi

        USAGE=`awk -v bootime="${bootime}" -v lbootime="${LAST_UPTIME}"                            \
                -v lptime="${LAST_PROTIME}" -v hz="${HERTZ}" -v pagesize="${PAGESIZE}"             \
                'BEGIN{ percent=0;memusage=0;protime=0 }; {                                        \
                        tmp = (bootime - lbootime) * hz;                                           \
                        ptime= $14 + $15;                                                          \
                        memusage = $24 * pagesize / 1024;                                          \
                        if (tmp <=0)                                                               \
                                percent = 0;                                                       \
                        else                                                                       \
                                percent = int((ptime - lptime) * 100 / tmp)                        \
                }                                                                                  \
                END{ printf("CPUUSAGE=%d;MEMUSAGE=%d;LAST_PROTIME=%d", percent, memusage, ptime)}' \
                "/proc/${1}/stat" 2>&1`

        eval ${USAGE} 2>&1 >/dev/null
        integer_valid_check "${CPUUSAGE}" "${MEMUSAGE}" "${LAST_PROTIME}"
        LAST_UPTIME=${bootime}
}
integer_valid_check "${HERTZ}" "${PAGESIZE}"

for ((i = 0; i < ${AGENTS_NUM}; i++)); do
        CPUUSAGE=0
        MEMUSAGE=0

        agent=${AGENTS[${i}]}
        limcpu=${AGENTS_LIMIT_CPU[${i}]}
        limmem=${AGENTS_LIMIT_MEM[${i}]}
        integer_valid_check "${limcpu}" "${limmem}"

        # STEP1: Fetch the process ID for CloudAgent.
        log_info "check resource for agent ${agent}"
        pid=$(get_process_id ${agent})
        if [[ "x${pid}" == "x" || ! "${pid}" =~ ^[0-9]*$ ]]; then
                log_info "process not exist or invalid PID, check next"
                continue
        fi
        log_info "got a valid PID ${pid}"

        # STEP2: Prepare the data.
        first_get_process_info "${pid}"
        sleep 3

        # STEP3: And fire it.
        for j in {1..3}; do
                get_usage "${pid}"
                # NOTE: 90% 200M -> 60% 500M -> 30% 500M also send signal.
                log_info "current resource for PID ${pid}, CPU ${CPUUSAGE}% MEM ${MEMUSAGE}KB"
                if [[ "${CPUUSAGE}" -lt "${limcpu}" && "${MEMUSAGE}" -lt "${limmem}" ]]; then
                        break
                fi
                log_info "resource overflow ${j} times, limits CPU ${limcpu}% MEM ${limmem}KB"
                if [[ ${j} -ge 3 ]]; then
                        actexe=`readlink "/proc/${pid}/exe"`
                        if [[ "x${actexe}" != "x${agent}" ]]; then
                                break
                        fi
                        log_info "send SIGKILL to process ${pid}"
                        kill -9 "${pid}"
                        continue
                fi
                sleep 3
        done
done
-->

{% highlight text %}
{% endhighlight %}
