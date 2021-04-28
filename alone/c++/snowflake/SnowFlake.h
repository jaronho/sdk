/**********************************************************************
* Author:	jaron.ho
* Date:		2020-08-27
* Brief:	snow flake序列号生成器
**********************************************************************/
#ifndef _SNOW_FLAKE_H_
#define _SNOW_FLAKE_H_

class SnowFlake {
public:
    /*
     * Brief:   构造函数
     *
     * Param:   datacenterId - 数据中心ID
     *          workerId - 工作机器ID
     * Return:  无
     */
    SnowFlake(uint64_t datacenterId = 0, uint64_t workerId = 0);
    
    /*
     * Brief:   生成序列ID
     *
     * Param:   void
     * Return:  序列ID
     */
    uint64_t generate(void);
    
private:
    uint64_t m_flag; /* 符号位(1位),为0,通常不用 */
    uint64_t m_timestamp; /* 时间戳(41位),精确到毫秒,支持2^41/365/24/60/60/1000=69.7年 */
    /* 整个分布式系统内不会产生ID碰撞(由datacenterId和workerId作区分),支持1024个进程 */
    uint64_t m_datacenterId; /* 数据中心ID(5位) */
    uint64_t m_workerId; /* 工作机器ID(5位) */
    uint64_t m_sequence; /* 序列号(12位),每毫秒从0开始自增,支持4096个编号 */
};

/*
 * Brief:   生成序列ID
 *
 * Param:   void
 * Return:  序列ID
 */
static uint64_t generateSnowFlakeSeqId(void);

#endif  // _SNOW_FLAKE_H_
