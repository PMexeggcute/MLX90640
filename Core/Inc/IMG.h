#ifndef     __IMG_H__
#define     __IMG_H__


#include "main.h"


typedef struct{
	uint8_t num[3];
	uint8_t list[3][320][3];
} edge_rgbbuf_typdef;


void IMG_edge_tidy(uint16_t *pdata,uint16_t *tdata,uint16_t w,uint16_t h);
void IMG_edge_full(uint16_t *pdata,uint16_t *tdata,uint16_t w,uint16_t h);

uint16_t rgb_565(uint16_t COLOR_R,uint16_t COLOR_G,uint16_t COLOR_B);
uint16_t color_code(uint16_t grayValue,uint16_t mode);
void color_listcode(uint16_t *color_list,uint16_t mode);
void display_code(float *flist,uint16_t *show_list,uint16_t *color_list,uint16_t size,float tmax,float tmin);
void temp_limit(float *flist,float *maxtemp,uint16_t *max_addr,float *mintemp,uint16_t *min_addr);





#endif
