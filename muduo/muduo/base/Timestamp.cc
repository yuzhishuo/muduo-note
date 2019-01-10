#include <muduo/base/Timestamp.h>

#include <sys/time.h>
#include <stdio.h>

// c++使用PRID64，需要两步：

//     包含头文件：<inttypes.h>
//     定义宏：__STDC_FORMAT_MACROS，可以通过编译时加-D__STDC_FORMAT_MACROS，或者在包含文件之前定义这个宏。

#ifndef __STDC_FORMAT_MACROS // !!!
#define __STDC_FORMAT_MACROS // !!!
#endif

#include <inttypes.h>         // !!!

#include <boost/static_assert.hpp>

using namespace muduo;

BOOST_STATIC_ASSERT(sizeof(Timestamp) == sizeof(int64_t));

string Timestamp::toString() const
{
  char buf[32] = {0};
  int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
  int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
  // int64_t用来表示64位整数，在32位系统中是long long int，在64位系统中是long int，所以打印int64_t的格式化方法是：
  // 源文件中 sceonds,microseconds 定义为 int64_t
  snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
  // snprintf 将可变个参数(...)按照format格式化成字符串，然后将其复制到str中
  return buf;
}

//  你看默认的时间戳中定义的是毫秒级的
string Timestamp::toFormattedString(bool showMicroseconds) const
{
  char buf[32] = {0};
  time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
  
  struct tm tm_time;
  // struct tm *gmtime(const time_t *timep)；

  // struct tm *gmtime_r(const time_t *timep, struct tm *result);

  // 这两个函数意思一样，将timep这个秒数转换成以UTC时区为标准的年月日时分秒时间。gmtime_r是线程安全的，
  gmtime_r(&seconds, &tm_time);

  if (showMicroseconds)
  {
    int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);
  }
  else
  {
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  }
  return buf;
}

//  timeval
//  {
//    time_t tv_sec; //秒 [long int]
//    suseconds_t tv_usec; //微秒 [long int]
//  };

Timestamp Timestamp::now()
{
	// 定时函数，用于让程序等待一段时间或安排计划任务：
	// • sleep • alarm • usleep • nanosleep • clock_nanosleep 
	// • getitimer / setitimer 
	// • timer_create / timer_settime / timer_gettime / timer_delete 
	// • timerfd_create / timerfd_gettime / timerfd_settime
	// 作者的取舍原因如下：
	// • （计时）只使用 gettimeofday 来获取当前时间。 
	// • （定时）只使用 timerfd_* 系列函数来处理定时。
	//	gettimeofday 入选原因：（这也是 muduo::Timestampclass 的主要设计考虑）
	//	1. time 的精度太低，ftime 已被废弃，clock_gettime 精度最高，但是它系统调用 的开销比 gettimeofday 大。
	//  2. 在 x86 - 64 平台上，gettimeofday 不是系统调用，而是在用户态实现的（搜 vsyscall），没有上下文切换和陷入内核的开销。 
	//  3. gettimeofday 的分辨率(resolution) 是 1 微秒，足以满足日常计时的需要。
	//  muduo::Timestamp 用一个 int64_t 来表示从 Epoch 到现在的微秒数，其范围 可达上下 30 万年。
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

// struct tm {
//   int tm_sec; /* 秒 – 取值区间为[0,59] */
//   int tm_min; /* 分 - 取值区间为[0,59] */
//   int tm_hour; /* 时 - 取值区间为[0,23] */
//   int tm_mday; /* 一个月中的日期 - 取值区间为[1,31] */
//   int tm_mon; /* 月份（从一月开始，0代表一月） - 取值区间为[0,11] */
//   int tm_year; /* 年份，其值等于实际年份减去1900 */
//   int tm_wday;  /* 星期 – 取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推 */ 
//   int tm_yday; /* 从每年的1月1日开始的天数 – 取值区间为[0,365]，其中0代表1月1日，1代表1月2日，以此类推 */
//   int tm_isdst; /* 夏令时标识符，实行夏令时的时候，tm_isdst为正。不实行夏令时的时候，tm_isdst为0；不了解情况时，tm_isdst()为负。*/
//   long int tm_gmtoff; /*指定了日期变更线东面时区中UTC东部时区正秒数或UTC西部时区的负秒数*/
//   const char *tm_zone; /*当前时区的名字(与环境变量TZ有关)*/
// };