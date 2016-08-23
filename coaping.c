#include "coaping.h"

int main(int argc, char* argv[])
{
    srand(time(NULL));
    signal(SIGINT, show_resume);
    int opt;
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
	    if(startid==0xFFFF) startid = 0;
	    startid++;	    
	}
    }else{
	int n = 0;
	for(n=0; n!=ntimes; n++){
	   ping(startid);
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
     printf("%d packets transmitted, %d received, %d errors, %d%% packet loss\n",total,nsuccess,nfail,(nfail/total)*100);
     exit(0);
}
void print_help(){
    printf("\n pingcoap [-p port] [-n times] [ADDRESS]\n");
    printf("\n Opciones:\n");
    printf("\t -p port: para indicar que el servidor TIME al que nos conectamos escucha en un puerto diferente al 37. El puerto por defecto es el 37.\n");
    printf("\t -n times: Indica el numero de paquetes a enviar. Cero equivale a infinito.");
    printf("\t -?: Muestra esta ayuda.\n");

    printf("\n Si no se especifica la opción m, el programa arranca en modo consulta UDP, es decir: -m cu.\n Si alguno de los parámetros opcionales no se proporciona, se tomarán los valores por defecto.\n\n");
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

void ping(uint16_t id){
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
	    perror("Error al abrir conexión");
	    exit(-1);
    }

    //Enviamos datagrama vacío
    coap_dtg_t datagram;
    bzero(&datagram,sizeof(datagram));
    datagram.vtl = 0b01000000;
    datagram.id = htons(id);
    // Calculate time taken by a request
    clock_gettime(CLOCK_REALTIME, &requestStart);
    send(sockfd, &datagram,sizeof(datagram), 0);
    recv_bytes = recvfrom(sockfd, &recvt, BUFF_LEN, 0, (struct sockaddr *) &raddr, &fromlen); 			//size_t send(int sockfd, const void *buf, size_t len, int flags);
    //recv_bytes = recv(sockfd, &recvt, BUFF_LEN, 0);
    clock_gettime(CLOCK_REALTIME, &requestEnd);

    double accum = ( requestEnd.tv_sec - requestStart.tv_sec ) + ( requestEnd.tv_nsec - requestStart.tv_nsec )/ BILLION;
    total_time = total_time +accum;
    if(recv_bytes==4){
	nsuccess++;	
	coap_dtg_t *recv_dtg = (coap_dtg_t *) &recvt;	
	if((recv_dtg->vtl & 0b00110000) == 0b00110000) response_type = "RST";
	if(recv_dtg->id == datagram.id) id_mismatch = "";
	
	getnameinfo((struct sockaddr *)&raddr, sizeof(raddr), host, sizeof(host), NULL, 0, 0);	
	printf("%d bytes from %s (%s): type=%s time=%.2lf ms %s\n",recv_bytes,host,inet_ntoa(raddr.sin_addr),response_type,accum,id_mismatch); 
	
    }
    else{
	nfail++;
	printf("From %s: %s\n",inet_ntoa(server_ip),strerror(errno));	
    }
    sleep(SLEEP_SEC);
    close(sockfd);
}


// /vs0.inf.ethz.ch
