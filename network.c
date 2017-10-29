#include "network.h"

char * getIpAddress()
{
  int fd;
  struct ifreq ifr;

  fd = socket(AF_INET, SOCK_DGRAM, 0);

  /* IPv4のIPアドレスを取得したい */
  ifr.ifr_addr.sa_family = AF_INET;

  /* eth0のIPアドレスを取得したい */
  strncpy(ifr.ifr_name, INTERFACE, IFNAMSIZ-1);

  ioctl(fd, SIOCGIFADDR, &ifr);

  close(fd);

  /* 結果を表示 */
  printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

  return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}
