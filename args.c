#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include "args.h"
#include "options.h"

void usage(char *msg, int exit_status)
{
    fprintf(exit_status == 0 ? stdout : stderr,
	    "%s", USAGE_TXT);
    
    if (msg) {
	fprintf(exit_status == 0 ? stdout : stderr,
		"\n%s\n", msg);
    }
    
    exit(exit_status);
}
 char *
str_ip1(uint32_t ip)
{
    struct in_addr addr;
    memcpy(&addr, &ip, sizeof(ip));
    return inet_ntoa(addr);
}
char our_ip[20] = {0};
//获得本机的ip列表

int MyifconfigTest( address_pool *pool)
{
	int i = 0;
	struct ifreq * ifr = NULL;
	struct ifconf ifc;
	//初始化ifconf
	unsigned char buf[1024] = {0};
	ifc.ifc_len = 1024;
	ifc.ifc_buf = buf;

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0){
	    perror("create socket failed:");
	    return -1;
	}
	
	if (ioctl(sock, SIOCGIFCONF, (char *) &ifc) < 0){
	    perror("ioctl-conf:");
		close(sock);
	    return -1;
	}	
	//printf("ifc.ifc_len %d\n",ifc.ifc_len);
	//printf("We have %d network card\n",ifc.ifc_len/sizeof(struct ifreq));
	ifr = (struct ifreq*)buf;  
	for(i = 0;i < ifc.ifc_len/sizeof(struct ifreq);i++){
        if(strcmp(ifr->ifr_name,pool->device)){
            ifr++;
            continue;
        }
	//网卡名
		printf("\n%s", ifr->ifr_name);
	//网卡本地IP地址
		//Get
		if(ioctl(sock, SIOCGIFADDR, ifr)>=0){
			printf("	IP: %s\n",inet_ntoa(((struct sockaddr_in*)&(ifr->ifr_addr))->sin_addr));
            strncpy(our_ip,inet_ntoa(((struct sockaddr_in*)&(ifr->ifr_addr))->sin_addr),strlen(inet_ntoa(((struct sockaddr_in*)&(ifr->ifr_addr))->sin_addr)));
			// printf("	IP + 1: %s\n",inet_ntoa(((struct sockaddr_in*)&(ifr->ifr_addr))->sin_addr));
// ((struct sockaddr_in*)&(ifr->ifr_addr))->sin_addr.s_addr = ((struct sockaddr_in*)&(ifr->ifr_addr))->sin_addr.s_addr + 1024*10;
			// printf("	IP + 10: %s\n",inet_ntoa(htonl(ntohl((struct sockaddr_in*)&(ifr->ifr_addr))->sin_addr.s_addr) + 30));
		}else{
			perror("	ioctl SIOCGIFADDR error");
		}
		ifr++;	  
	}
    printf("our ip is %s",our_ip);
	close(sock);


	uint32_t *current;

	if (parse_ip(our_ip, (void **)&current) != 4)
		usage("error: invalid last ip in address pool.", 1);

	printf("\n	IP current: %s\n",str_ip1(*current));

	pool->indexes.first   = htonl(ntohl(*current) + 1);
	pool->indexes.last    = htonl(ntohl(*current) + 50);
	pool->indexes.current = pool->indexes.first;

	printf("	IP first: %s\n",str_ip1(pool->indexes.first));
	printf("	IP last: %s\n",str_ip1(pool->indexes.last));
	printf("	IP current: %s\n",str_ip1(pool->indexes.current));

}

void parse_args(int argc, char *argv[], address_pool *pool)
{
    int c;

    opterr = 0;
	// MyifconfigTest();	
    while ((c = getopt (argc, argv, "a:d:o:p:s:")) != -1)
	switch (c) {

	case 'a': // parse IP address pool
	    {
		char *opt    = strdup(optarg);
		char *sfirst = opt;
		char *slast  = strchr(opt, ',');
	    
		if (slast == NULL)
		    usage("error: comma not present in option -a.", 1);
		*slast = '\0';
		slast++;
	    
		uint32_t *first, *last;
		
		if (parse_ip(sfirst, (void **)&first) != 4)
		    usage("error: invalid first ip in address pool.", 1);
		
		if (parse_ip(slast, (void **)&last) != 4)
		    usage("error: invalid last ip in address pool.", 1);
			
		pool->indexes.first   = *first;
		pool->indexes.last    = *last;
		pool->indexes.current = *first;
		
		free(first);
		free(last);
		free(opt);
		
		break;
	    }

	case 'd': // network device to use
	    {
		strncpy(pool->device, optarg, sizeof(pool->device));
		MyifconfigTest(pool);
		break;
	    }
	    
	case 'o': // parse dhcp option
	    {
		uint8_t id;

		char *opt   = strdup(optarg);
		char *name  = opt;
		char *value = strchr(opt, ',');
		
		if (value == NULL)
		    usage("error: comma not present in option -o.", 1);
		*value = '\0';
		value++;
		
		dhcp_option *option = calloc(1, sizeof(*option));
		
		if((id = parse_option(option, name, value)) == 0) {
		    char msg[128];
		    snprintf(msg, sizeof(msg),
			     "error: invalid dhcp option specified: %s,%s",
			     name, value);
		    usage(msg, 1);
		}
		
		append_option(&pool->options, option);

		if(option->id == IP_ADDRESS_LEASE_TIME)
		    pool->lease_time = ntohl(*((uint32_t *)option->data));

		free(option);
		free(opt);
		break;
	    }

	case 'p': // parse pending time
	    {
		time_t *t;

		if(parse_long(optarg, (void **)&t) != 4)
		    usage("error: invalid pending time.", 1);

		pool->pending_time = *t;
		free(t);
		break;
	    }

	case 's': // static binding
	    {
		char *opt = strdup(optarg);
		char *shw  = opt;
		char *sip  = strchr(opt, ',');
		
		if (sip == NULL)
		    usage("error: comma not present in option -s.", 1);
		*sip = '\0';
		    sip++;
		
		uint32_t *ip;
		uint8_t  *hw;
		
		if (parse_mac(shw, (void **)&hw) != 6)
		    usage("error: invalid mac address in static binding.", 1);
		
		if (parse_ip(sip, (void **)&ip) != 4)
		    usage("error: invalid ip in static binding.", 1);
		
		add_binding(&pool->bindings, *ip, hw, 6, 1);
		
		free(ip);
		free(hw);
		free(opt);
	    }
	    
	case '?':
	    usage(NULL, 1);

	default:
	    usage(NULL, 1);
	}

    if(optind >= argc)
	usage("error: server address not provided.", 1);

    uint32_t *ip;

    if (parse_ip(our_ip, (void **)&ip) != 4)
	usage("error: invalid server address.", 1);

    pool->server_id = *ip;
    
    free(ip);
}
