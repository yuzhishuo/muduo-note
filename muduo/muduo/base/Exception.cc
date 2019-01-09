// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

// invailed


#include <muduo/base/Exception.h>

//#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>

using namespace muduo;

Exception::Exception(const char* msg)
  : message_(msg)
{
  // 用于栈追踪的初始化
  fillStackTrace();
}

Exception::Exception(const string& msg)
  : message_(msg)
{
  // 用于栈追踪的初始化
  fillStackTrace();
}

Exception::~Exception() throw ()
{
}

const char* Exception::what() const throw()
{
  return message_.c_str();
}

const char* Exception::stackTrace() const throw()
{
  return stack_.c_str();
}

void Exception::fillStackTrace()
{
  const int len = 200;
  void* buffer[len];
  // #include <execinfo.h>
  // 用于获取程序中的当前函数的回溯信息，即一系列的函数关系
  int nptrs = ::backtrace(buffer, len);
  //  backtrace_symbols将从backtrace函数获取的信息转化为一个字符串数组. 
  //  参数buffer应该是从backtrace函数获取的数组指针,size是该数组中的元素个数(backtrace的返回值)，
  //  函数返回值是一个指向字符串数组的指针,它的大小同buffer相同.
  //  每个字符串包含了一个相对于buffer中对应元素的可打印信息.它包括函数名，函数的偏移地址,和实际的返回地址
  char** strings = ::backtrace_symbols(buffer, nptrs);
  if (strings)
  {
    for (int i = 0; i < nptrs; ++i)
    {
      // TODO demangle funcion name with abi::__cxa_demangle
      stack_.append(strings[i]);
      stack_.push_back('\n');
    }
    free(strings);
  }
}

