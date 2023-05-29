#include "../includes/main.h"

extern t_data data;


void sig_handler(int sig)
{
  if (sig == SIGINT)
  {
	//if (iflist)
	//	freeifaddrs(g_project.iflist);
	close(data.sockfd);
	exit(1);
  }
}

int check_ip(char *source_ip, char *target_ip)
{
	char *ip_address;
	int ret = 0;

	for (int j = 1; j < 3; j++)
	{
		if (j == 1)
			ip_address = ft_strdup(source_ip);
		else
			ip_address = ft_strdup(target_ip);
		int num = 0;
		int count = 0;
		int len = strlen(ip_address);
		for (int i = 0; i < len; i++) {
 			if (ip_address[i] == '.') {
 				if (count >= 3 || num < 0 || num > 255)
					ret = 1;
				count++;
				num = 0;
			}
			else if (ip_address[i] >= '0' && ip_address[i] <= '9')
				num = num * 10 + (ip_address[i] - '0');
			else
				ret = 1;
		}
		if (count != 3 || num < 0 || num > 255)
 			ret = 1;
		free(ip_address);
		if (ret)
			return (j);
	}
	return (0);
}

int check_mac(char *source_mac, char *target_mac)
{
	char *mac_address;
	int ret = 0;
	int len;

	for (int i = 1; i < 3; i++) {
		if (i == 1)
			mac_address = ft_strdup(source_mac);
		else
			mac_address = ft_strdup(target_mac);
		len = strlen(mac_address); 
		if (len != 17)
			ret = 1;
		for (int i = 0; i < len; i++) {
			if (i % 3 == 2) {
				if (mac_address[i] != ':')
					ret = 1;
			} else {
				if ((mac_address[i] < '0' || mac_address[i] > '9') &&
					(mac_address[i] < 'A' || mac_address[i] > 'F') &&
					(mac_address[i] < 'a' || mac_address[i] > 'f')) {
					ret = 0;
				}
			}
		}
		free(mac_address);
		if (ret)
			return (i);
	}
	return (0);
}


void 	print_buff(ssize_t buflen, unsigned char *buffer)
{
	ssize_t i = 0;
	while (i < buflen)
		printf("%02x ", buffer[i++]);
}

char	hextobyte(const char *hex)
{
	/* Translate ASCII value into binary */
	char left, right;
	if (hex[0] < 'a') {
		left = hex[0] - 48;
	}
	else {
		left = hex[0] - 87;
	}
	if (hex[1] < 'a') {
	right = hex[1] - 48; 
	}
	else {
		right = hex[1] - 87;
	}

	/* Join the two values into a single byte */
	unsigned char result = ((left << 4) + right);
	return result;
}

void	mac_strbin(unsigned char *bin, const char *hex)
{
	ssize_t size = 0;
	char tmp[17] = {0};

	while (size <= 16)
	{
		tmp[size] = ft_tolower(hex[size]);
		++size;
	}
	size = 0;
	while (size < 16)
	{
		if (size % 3 == 0) /* Moving the pointer to hex sequences in the string */
			*bin++ = hextobyte(tmp + size);
		++size;
	}
}

struct ifaddrs *getinterface(const char *name)
{
	while (data.iflist)
	{
		if (strcmp(data.iflist->ifa_name, name) == 0)
			return (data.iflist);
		data.iflist = data.iflist->ifa_next;
	}
	return (NULL);
}

