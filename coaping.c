#include "coaping.h"

void sig_hndlr(){
    stop = 1;
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    signal(SIGINT, sig_hndlr);
    int opt;
    
    if(argc==1) print_help();
    
    while ((opt = getopt(argc, argv, "p:n:?")) != -1) {
	switch(opt) {
	    case 'p':
		port = atoi(optarg);
		break;
	    case 'n':
		ntimes = atoi(optarg);
		break;
	    case '?':
		print_help();
	}
	    
    }
    address = argv[argc-1];

    if(port==0){
	port = DEFAULT_CLIENT_PORT;
    }

    resolve();
    uint16_t startid = rand();
    if(ntimes==0){
	while(1){
	    ping(startid);
	    if(stop==1) show_resume();
	    if(startid==0xFFFF) startid = 0;
	    startid++;	    
	}
    }else{
	int n = 0;
	for(n=0; n!=ntimes; n++){
	   ping(startid);
	   if(stop==1) show_resume();
	   startid++; 
	}
    }

    show_resume();
}
void show_resume(){
    /*
--- 163.117.94.180 ping statistics ---
6 packets transmitted, 0 received, +3 errors, 100% packet loss, time 5032ms
     */
     printf("\n--- %s ping statistics ---\n",inet_ntoa(server_ip));
     unsigned int total = nfail+nsuccess;
     double percentage = ((double)nfail/(double)total)*100.0;
     
     //printf("%d packets transmitted, %d received, %d errors, %.2lf%% packet loss\n",total,nsuccess,nfail,percentage);
     printf("%d packets transmitted, %d received, %.2lf%% packet loss\n",total,nsuccess,percentage);
     
     // rtt min/avg/max/mdev = 38.218/38.218/38.218/0.000 ms
    printf("rtt min/avg/max = %.3lf/%.3lf/%.3lf ms\n",min_time,total_time/(double)total,max_time);
     exit(0);
}
void print_help(){
    printf("pingcoap [-?] [-p port] [-n count] destination\n");
    printf("Options:\n");
    printf("   -p port: Targeted port. Default is %d\n",DEFAULT_CLIENT_PORT);
    printf("   -n times: Number of pings you want to send. It's infinite by default.\n");
    printf("   -?: Showes this help message.\n");
    exit(1);
}

void resolve(){
    server_address = gethostbyname(address);

    if(server_address){
	    bcopy(*server_address->h_addr_list++, (char *) &server_ip, sizeof(server_ip));
	    printf("PING %s (%s)\tport: %d \n", server_address->h_name,inet_ntoa(server_ip),port);
    }else{
	    printf("Server not found.\n");
	    exit(-1);
    }
}

int ping(uint16_t id){
    int sockfd;
    uint32_t recvt;
    int recv_bytes;
    struct timespec requestStart, requestEnd;
    struct sockaddr_in raddr;
    socklen_t fromlen = sizeof(raddr);
    char *response_type =  "???";
    char *id_mismatch = "(ID MISMATCH)";
    char host[1024];
    
    //Now we create the socket
    struct sockaddr_in client_socket;
    bzero(&client_socket, sizeof(client_socket)); // Clean the structure JIC
    client_socket.sin_family = AF_INET;
    client_socket.sin_addr.s_addr = inet_addr(inet_ntoa(server_ip));
    client_socket.sin_port = htons(port);

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = TIMEOUT_MICROS;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
	perror("Error");
    }    
    int result = connect(sockfd, (struct sockaddr *)&client_socket, sizeof(struct sockaddr));
    if(result < 0) {
	    perror("Error");
	    nfail++;
	    sleep(SLEEP_SEC);
	    close(sockfd);
	    return 0;
    }

    // Create and fill datagram
    coap_dtg_t datagram;
    bzero(&datagram,sizeof(datagram));
    datagram.vtl = 0b01000000;
    datagram.id = htons(id);
    
    clock_gettime(CLOCK_REALTIME, &requestStart);	    // Calculate time taken by a request

    send(sockfd, &datagram,sizeof(datagram), 0);
    recv_bytes = recvfrom(sockfd, &recvt, BUFF_LEN, 0, (struct sockaddr *) &raddr, &fromlen); 			//size_t send(int sockfd, const void *buf, size_t len, int flags);
    clock_gettime(CLOCK_REALTIME, &requestEnd);

    double accum = ( requestEnd.tv_sec - requestStart.tv_sec ) + ( requestEnd.tv_nsec - requestStart.tv_nsec )/ BILLION;
    total_time = total_time + accum;
    if(accum<min_time || min_time==-1) min_time=accum;
    if(accum>max_time) max_time=accum;
    if(recv_bytes==4){
	nsuccess++;	
	coap_dtg_t *recv_dtg = (coap_dtg_t *) &recvt;	
	if((recv_dtg->vtl & 0b00110000) == 0b00110000) response_type = "RST";
	if(recv_dtg->id == datagram.id) id_mismatch = "";
	
	getnameinfo((struct sockaddr *)&raddr, sizeof(raddr), host, sizeof(host), NULL, 0, 0);	
	printf("%d bytes from %s (%s): type=%s time=%.2lf ms %s\n",recv_bytes,host,inet_ntoa(raddr.sin_addr),response_type,accum,id_mismatch); 
	
    }
    else if(recv_bytes<0){
	nfail++;
	printf("Error: %s\n",strerror(errno));
    }
    else{
	nfail++;	
	getnameinfo((struct sockaddr *)&raddr, sizeof(raddr), host, sizeof(host), NULL, 0, 0);	
	printf("%d bytes from %s (%s): [¿COAP?] time=%.2lf ms\n",recv_bytes,host,inet_ntoa(raddr.sin_addr),accum);	
    }
    sleep(SLEEP_SEC);
    close(sockfd);
    return 0;
}


// /vs0.inf.ethz.ch
