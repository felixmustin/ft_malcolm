#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <ctype.h>
#include <sys/cdefs.h>
# include <signal.h>


typedef struct s_params
{	
	in_addr_t target_ip;
	in_addr_t source_ip;
	unsigned char target_mac[6];
	unsigned char source_mac[6];
} t_params;

typedef struct arp_header
{
        unsigned char	Hardware[2] ;
	unsigned char	Protocol[2] ;
	unsigned char	HardwareAddressLen ;
	unsigned char	ProtocolAddressLen ;
	unsigned char	Operation[2] ;
	unsigned char	SenderHardwareAddr[6] ;
	unsigned char	SenderIpAddr[4] ;
	unsigned char	TargetHardwareAddr[6] ;
	unsigned char	TargetIPAddr[4] ;
} arp_header;

typedef struct s_data
{
	int sockfd;
	t_params params;
	struct ifaddrs *iflist;
	int waiting;
	int print_info;
} t_data;

void	print_buff(ssize_t buflen, unsigned char *buffer);
char	hextobyte(const char *hex);
void	mac_strbin(unsigned char *bin, const char *hex);
struct	ifaddrs *getinterface(const char *name);
void sig_handler(int sig);
int check_ip(char *source_ip, char *target_ip);
int check_mac(char *source_mac, char *target_mac);
