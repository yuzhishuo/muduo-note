#include <examples/ace/ttcp/common.h>

#include <boost/program_options.hpp>

#include <iostream>

#include <netdb.h>
#include <stdio.h>

namespace po = boost::program_options;

bool parseCommandLine(int argc, char* argv[], Options* opt)
{
  po::options_description desc("Allowed options");
  desc.add_options()
      ("help,h", "Help")
      ("port,p", po::value<uint16_t>(&opt->port)->default_value(5001), "TCP port")
      ("length,l", po::value<int>(&opt->length)->default_value(65536), "Buffer length")
      ("number,n", po::value<int>(&opt->number)->default_value(8192), "Number of buffers")
      ("trans,t",  po::value<std::string>(&opt->host), "Transmit")
      ("recv,r", "Receive")
      ("nodelay,D", "set TCP_NODELAY")
      ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  opt->transmit = vm.count("trans");
  opt->receive = vm.count("recv");
  opt->nodelay = vm.count("nodelay");
  if (vm.count("help"))
  {
    std::cout << desc << std::endl;
    return false;
  }

  if (opt->transmit == opt->receive)
  {
    printf("either -t or -r must be specified.\n");
    return false;
  }

  printf("port = %d\n", opt->port);
  if (opt->transmit)
  {
    printf("buffer length = %d\n", opt->length);
    printf("number of buffers = %d\n", opt->number);
  }
  else
  {
    printf("accepting...\n");
  }
  return true;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"

struct sockaddr_in resolveOrDie(const char* host, uint16_t port)
{
  // struct hostent
	// {
	// 	char *h_name;         //正式主机名
	// 	char **h_aliases;     //主机别名
	// 	int h_addrtype;       //主机IP地址类型：IPV4-AF_INET
	// 	int h_length;		      //主机IP地址字节长度，对于IPv4是四字节，即32位
	// 	char **h_addr_list;	  //主机的IP地址列表
	// };
	
	// #define h_addr h_addr_list[0]   //保存的是IP地址

  struct hostent* he = ::gethostbyname(host);
  if (!he)
  {
    perror("gethostbyname");
    exit(1);
  }
  assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
  return addr;
}

