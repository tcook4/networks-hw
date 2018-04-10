#include <stdio.h>

int main(int argc, char *argv[])
{
    struct ex {
    unsigned int
           tcp_res1:4,       /*little-endian*/
           tcph_hlen:4,      /*length of tcp header in 32-bit words*/
           tcph_fin:1,       /*Finish flag "fin"*/
           tcph_syn:1,       /*Synchronize sequence numbers to start a connection*/
           tcph_rst:1,       /*Reset flag */
           tcph_psh:1,       /*Push, sends data to the application*/
           tcph_ack:1,       /*acknowledge*/
           tcph_urg:1,       /*urgent pointer*/
           tcph_res2:2;
    };

    printf("ex struct sizeof is %d\n", sizeof(struct ex));


    unsigned int iz;

    printf("size of int is %d\n", sizeof(iz));
    return 0;
}
