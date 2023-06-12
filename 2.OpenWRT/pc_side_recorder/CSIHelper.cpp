//
// Created by Orlando Chen on 2023/06/12.
//

#include "CSIHelper.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define csi_st_len 23

/* test and find out the system is big endian or not*/
//for 16/32/64 system this should be all fine.
bool is_big_endian(){
    unsigned int a = 0x1;
    unsigned char b = *(unsigned char *)&a;
    if ( b == 0)
    {
        return true;
    }
    return false;
}

int bit_convert(int data, int maxbit)
{
    if ( data & (1 << (maxbit - 1)))
    {
        /* negative */
        data -= (1 << maxbit);
    }
    return data;
}

void fill_csi_matrix(u_int8_t* csi_addr, int nr, int nc, int num_tones, COMPLEX(* csi_matrix)[3][114]){
    u_int8_t k;
    u_int8_t bits_left, nr_idx, nc_idx;
    u_int32_t bitmask, idx, current_data, h_data, h_idx;
    int real,imag;
    /* init bits_left
     * we process 16 bits at a time*/
    bits_left = 16;

    /* according to the h/w, we have 10 bit resolution
     * for each real and imag value of the csi matrix H
     */
    bitmask = (1 << 10) - 1;
    idx = h_idx = 0;
    /* get 16 bits for processing */
    h_data = csi_addr[idx++];
    h_data += (csi_addr[idx++] << 8);
    current_data = h_data & ((1 << 16) -1);

    /* loop for every subcarrier */
    for(k = 0;k < num_tones;k++){
        /* loop for each tx antenna */
        for(nc_idx = 0;nc_idx < nc;nc_idx++){
            /* loop for each rx antenna */
            for(nr_idx = 0;nr_idx < nr;nr_idx++){
                /* bits number less than 10, get next 16 bits */
                if((bits_left - 10) < 0){
                    h_data = csi_addr[idx++];
                    h_data += (csi_addr[idx++] << 8);
                    current_data += h_data << bits_left;
                    bits_left += 16;
                }

                imag = current_data & bitmask;
                imag = bit_convert(imag, 10);
                //printf("imag is: %d | ",imag);
                csi_matrix[nr_idx][nc_idx][k].imag = imag;
                //printf("imag is: %d | ",csi_matrix[nr_idx][nc_idx][k].imag);


                bits_left -= 10;
                current_data = current_data >> 10;

                /* bits number less than 10, get next 16 bits */
                if((bits_left - 10) < 0){
                    h_data = csi_addr[idx++];
                    h_data += (csi_addr[idx++] << 8);
                    current_data += h_data << bits_left;
                    bits_left += 16;
                }

                real = current_data & bitmask;
                real = bit_convert(real,10);
                //printf("real is: %d |",real);
                csi_matrix[nr_idx][nc_idx][k].real = real;
                //printf("real is: %d \n",csi_matrix[nr_idx][nc_idx][k].real);

                bits_left -= 10;
                current_data = current_data >> 10;
            }
        }

    }
}

int read_csi_buf(unsigned char* buf_addr,int fd, int BUFSIZE){
    int cnt;
    /* listen to the port
     * read when 1, a csi is reported from kernel
     *           2, time out
     */
    cnt = read(fd,buf_addr,BUFSIZE);
    if(cnt)
        return cnt;
    else
        return 0;
}

 void record_status(unsigned char* buf_addr, int cnt, csi_struct* csi_status){
     if (is_big_endian()){
         csi_status->timestamp  =
             ((buf_addr[0] << 56) & 0x00000000000000ff) | ((buf_addr[1] << 48) & 0x000000000000ff00) |
             ((buf_addr[2] << 40) & 0x0000000000ff0000) | ((buf_addr[3] << 32) & 0x00000000ff000000) |
             ((buf_addr[4] << 24) & 0x000000ff00000000) | ((buf_addr[5] << 16) & 0x0000ff0000000000) |
             ((buf_addr[6] << 8)  & 0x00ff000000000000) | ((buf_addr[7])       & 0xff00000000000000) ;
         csi_status->csi_len = ((buf_addr[8] << 8) & 0xff00) | (buf_addr[9] & 0x00ff);
         csi_status->channel = ((buf_addr[10] << 8) & 0xff00) | (buf_addr[11] & 0x00ff);
         csi_status->data_len = ((buf_addr[cnt-2] << 8) & 0xff00) | (buf_addr[cnt-1] & 0x00ff);
         csi_status->payload_len = ((buf_addr[csi_st_len] << 8) & 0xff00) |
             ((buf_addr[csi_st_len + 1]) & 0x00ff);
     }else{
         csi_status->timestamp  =
             ((buf_addr[7] << 56) & 0x00000000000000ff) | ((buf_addr[6] << 48) & 0x000000000000ff00) |
             ((buf_addr[5] << 40) & 0x0000000000ff0000) | ((buf_addr[4] << 32) & 0x00000000ff000000) |
             ((buf_addr[3] << 24) & 0x000000ff00000000) | ((buf_addr[2] << 16) & 0x0000ff0000000000) |
             ((buf_addr[1] << 8)  & 0x00ff000000000000) | ((buf_addr[0])       & 0xff00000000000000) ;
         csi_status->csi_len = ((buf_addr[9] << 8) & 0xff00) | (buf_addr[8] & 0x00ff);

         csi_status->channel = ((buf_addr[11] << 8) & 0xff00) | (buf_addr[10] & 0x00ff);

         csi_status->data_len = ((buf_addr[cnt-1] << 8) & 0xff00) | (buf_addr[cnt-2] & 0x00ff);

         csi_status->payload_len = ((buf_addr[csi_st_len+1] << 8) & 0xff00) |
             (buf_addr[csi_st_len] & 0x00ff);
     }
     csi_status->err_info       = buf_addr[12];
     csi_status->noise_floor    = buf_addr[13];
     csi_status->rate           = buf_addr[14];
     csi_status->bandWidth      = buf_addr[15];
     csi_status->num_tones      = buf_addr[16];
     csi_status->nr             = buf_addr[17];
     csi_status->nc             = buf_addr[18];

     csi_status->rssi           = buf_addr[19];
     csi_status->rssi_0         = buf_addr[20];
     csi_status->rssi_1         = buf_addr[21];
     csi_status->rssi_2         = buf_addr[22];
 }

 void record_csi_payload(unsigned char* buf_addr, csi_struct* csi_status, unsigned char* data_buf, COMPLEX(* csi_matrix)[3][114]){
     int i;
     int nr,nc,num_tones;
     u_int8_t* csi_addr;
     u_int16_t payload_len, csi_len;

     nr          = csi_status->nr;
     nc          = csi_status->nc;
     num_tones   = csi_status->num_tones;
     payload_len = csi_status->payload_len;
     csi_len     = csi_status->csi_len;

     /* record the data to the data buffer*/
     for (i=1;i<=payload_len;i++){
     //    printf("i is: %d \n",i);
         data_buf[i-1] = buf_addr[csi_st_len + csi_len + i + 1];
     }

     /* extract the CSI and fill the complex matrix */
     csi_addr = buf_addr + csi_st_len + 2;
     fill_csi_matrix(csi_addr,nr,nc,num_tones, csi_matrix);
 }

 void  porcess_csi(unsigned char* data_buf, csi_struct* csi_status,COMPLEX(* csi_buf)[3][114]){
     /* here is the function for csi processing
      * you can install your own function */
     return;
 }