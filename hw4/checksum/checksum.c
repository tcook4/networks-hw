/*********************************************************/
/* Name: Checksum Computation Code                       */
/* Author: Robin Pottathuparambil                        */
/*********************************************************/

#include <stdio.h>
#include <string.h>



struct tcp_hdr{
                unsigned short int src;
                unsigned short int des;
                unsigned int seq;
                unsigned int ack;
                unsigned short int hdr_flags;
                unsigned short int rec;
                unsigned short int cksum;
                unsigned short int ptr;
                unsigned int opt;
              };


int main(void)
{

  struct tcp_hdr tcp_seg;
  unsigned short int cksum_arr[12];
  unsigned int i,sum=0, cksum, wrap;

  tcp_seg.src = 65234;
  tcp_seg.des = 40234;
  tcp_seg.seq = 1;
  tcp_seg.ack = 2;
  tcp_seg.hdr_flags = 0x2333;
  tcp_seg.rec = 0;
  tcp_seg.cksum = 0;  //Needs to be computed
  tcp_seg.ptr = 0;
  tcp_seg.opt = 0;

  memcpy(cksum_arr, &tcp_seg, 24); //Copying 24 bytes

  printf ("TCP Field Values\n");
  printf("0x%04X\n", tcp_seg.src); // Printing all values
  printf("0x%04X\n", tcp_seg.des);
  printf("0x%08X\n", tcp_seg.seq);
  printf("0x%08X\n", tcp_seg.ack);
  printf("0x%04X\n", tcp_seg.hdr_flags);
  printf("0x%04X\n", tcp_seg.rec);
  printf("0x%04X\n", tcp_seg.cksum);
  printf("0x%04X\n", tcp_seg.ptr);
  printf("0x%08X\n", tcp_seg.opt);

  printf ("\n16-bit values for Checksum Calculation\n");
  for (i=0;i<12;i++)            // Compute sum
  {
     printf("0x%04X\n", cksum_arr[i]);
     sum = sum + cksum_arr[i];
  }

  wrap = sum >> 16;             // Wrap around once
  sum = sum & 0x0000FFFF;
  sum = wrap + sum;

  wrap = sum >> 16;             // Wrap around once more
  sum = sum & 0x0000FFFF;
  cksum = wrap + sum;

  printf("\nSum Value: 0x%04X\n", cksum);
  /* XOR the sum for checksum */
  printf("\nChecksum Value: 0x%04X\n", (0xFFFF^cksum));

}

