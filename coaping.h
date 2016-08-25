#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <errno.h>


#define DEFAULT_CLIENT_PORT 5683
#define BUFF_LEN 4
#define TIMEOUT_MICROS 100000 // 100 ms 
#define SLEEP_SEC 1
#define BILLION 1E6


typedef struct {
    uint8_t vtl;           /* Version (2 bits), type (2 bits), length (2 bits)*/
    uint8_t msg;         /* request method (value 1--10) or response code (value 40-255) */
    uint16_t id;           /* message id */
} coap_dtg_t;

int port = 0;
int ntimes = 0;
char *address;

struct hostent *server_address;
struct in_addr server_ip;

double total_time = 0;
double min_time = -1;
double max_time = 0;

unsigned int nsuccess = 0;
unsigned int nfail = 0;

unsigned int stop = 0;

void print_help();
int ping(uint16_t id);
void resolve();
void show_resume();
