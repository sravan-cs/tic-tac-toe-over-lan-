#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <bits/stdc++.h>
using namespace std;
#define PACKET_SIZE 64
#define PORT_NO 8888
#define TIMEOUT 1 
int ping_loop = 1;
string error_message="Request timed out or host unreacheable";

struct ping_packet{
    struct icmphdr header ;
    char message[PACKET_SIZE-sizeof(struct icmphdr)];
};

unsigned short checksum(void* b,int len){
    unsigned short * buff=(unsigned short *)b;
    unsigned int sum=0;
    unsigned short result;
    for(;len>1;len-=2){
        sum+=*buff++;
    }
    if(len==1){
        sum += *(unsigned char*)buff;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result= ~sum;
    return result;
}

bool Is_valid_IP(string ip)
{
	struct sockaddr_in temp;
	if(inet_pton(AF_INET,&ip[0],&temp))
	{
		return true;
	}
    else{
        return false;
    }
}

double SendPing(int socket_fd,struct sockaddr_in *addr){
    struct timespec start,end;
    clock_gettime(CLOCK_MONOTONIC,&start);
    struct ping_packet packet_to_send;
    memset(&packet_to_send,0,sizeof(packet_to_send));
    packet_to_send.header.type=ICMP_ECHO;
    memset(packet_to_send.message,0,sizeof(packet_to_send.message));
    packet_to_send.header.checksum = checksum((void*)&packet_to_send,sizeof(packet_to_send));
    struct sockaddr_in ad;
    double rtt;
    int send = sendto(socket_fd,&packet_to_send,sizeof(packet_to_send),0,(struct sockaddr*) addr,sizeof(*addr));
	if(send <= 0){cout<<error_message<<endl;cout<<"a"<<endl;return -1;}


	send = 0;
	fcntl(STDIN_FILENO,F_SETFL,	fcntl(STDIN_FILENO,F_GETFL,0) | O_NONBLOCK);
	while(true)
	{   
        int temp=sizeof(ad);
		send = recvfrom(socket_fd,&packet_to_send,sizeof(packet_to_send),0,(struct sockaddr*)&ad,(unsigned int*)&(temp));
        clock_gettime(CLOCK_MONOTONIC,&end);
        double temp_time = (end.tv_sec-start.tv_sec)*1000+(end.tv_nsec-start.tv_nsec)/1000000;
        if(temp_time>1e3){break;}
		if(send > 0){
	        rtt = temp_time;
            return rtt;
        }
		else if(send < 0){cout<<error_message<<endl;cout<<"b"<<endl;}
	}
    cout<<error_message<<endl;cout<<"c"<<endl;
    return -1;
}

int main(int argc,char ** argv)
{
	struct in_addr socket_addr;
    int fd;
	struct sockaddr_in ping_addr;
	char *ip_addr = argv[1];

	if(Is_valid_IP(ip_addr))
	{
        inet_aton(ip_addr,&socket_addr);
		ping_addr.sin_port = htons(PORT_NO);
		ping_addr.sin_family = AF_INET;
		ping_addr.sin_addr.s_addr = socket_addr.s_addr;
		fd = socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
        if(fd<0){cout<<"socket unavailable"<<endl;}
		double val = SendPing(fd,&ping_addr);
        if(val>0){
            cout << "Reply from " << ip_addr << ": RTT = " << val << " ms" << endl;
        }
		return 0;
	}
	else
	{
		cout << "Bad Hostname" << endl;
		return -1;
	}

	return 0;
}