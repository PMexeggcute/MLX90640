#include "main.h"
#include "math.h"
#include "stdio.h"
#include "IMG.h"
// #include "arm_math.h"
#include "stdlib.h"
#include "math.h"

/*========================================================================*/
//RGB565拆分R5 G5 B5
/*========================================================================*/
void RGB565_split(uint16_t RGB565_DATA,uint8_t *tdata){  		
	tdata[0]=(RGB565_DATA>>11)&0X1F;
	tdata[1]=(RGB565_DATA>>6)&0X1F;
	tdata[2]=RGB565_DATA&0X1F;	
}
/*========================================================================*/
//R8 G8 B8合成RGB565
/*========================================================================*/
uint16_t rgb_565(uint16_t COLOR_R,uint16_t COLOR_G,uint16_t COLOR_B){
	uint16_t RGB565=0;
	RGB565=((COLOR_R&0XF8)<<8)+((COLOR_G&0XFC)<<3)+((COLOR_B&0XF8)>>3);
	return RGB565;
}
/*========================================================================*/
// 1/9像素边缘经检测
/*========================================================================*/
void IMG_edge_tidy(uint16_t *pdata,uint16_t *tdata,uint16_t w,uint16_t h){		
	uint32_t i,j,k;			//,l,m,n;
	int8_t Gx[9]={-1,0,1,
				  -2,0,2,
				  -1,0,1};
	int8_t Gy[9]={1,2,1,
				  0,0,0,
				  -1,-2,-1};
	int resx,resy,resxr,resxg,resxb,resyr,resyg,resyb,res;
	uint8_t buf[9][3];
	uint32_t pix_line1,pix_line2,pix_line3;	
	for(i=0;i<h-10;i++){
		for(j=0;j<w;j++){
			//if((i>0)&&(i<h-1)&&(j>0)&&(j<w-1)){
			if((i%3==1)&&(j%3==1)&&(i<h-1)&&(j<w-1)){
				pix_line1=(i-1)*w+j;
				pix_line2=i*w+j;
				pix_line3=(i+1)*w+j;
				RGB565_split(pdata[pix_line1-1]	,buf[0]);
				RGB565_split(pdata[pix_line1]	,buf[1]);
				RGB565_split(pdata[pix_line1+1]	,buf[2]);				
				RGB565_split(pdata[pix_line2-1]	,buf[3]);
				RGB565_split(pdata[pix_line2]	,buf[4]);
				RGB565_split(pdata[pix_line2+1]	,buf[5]);				
				RGB565_split(pdata[pix_line3-1]	,buf[6]);
				RGB565_split(pdata[pix_line3]	,buf[7]);
				RGB565_split(pdata[pix_line3+1]	,buf[8]);
				resxr=0;resxg=0;resxb=0;
				resyr=0;resyg=0;resyb=0;
				for(k=0;k<9;k++){
						resxr+=Gx[k]*buf[k][0];
						resxg+=Gx[k]*buf[k][1];
						resxb+=Gx[k]*buf[k][2];
						resyr+=Gy[k]*buf[k][0];
						resyg+=Gy[k]*buf[k][1];
						resyb+=Gy[k]*buf[k][2];
				}
				resx=abs(resxr)+abs(resxg)+abs(resxb);
				resy=abs(resyr)+abs(resyg)+abs(resyb);
				res=resx+resy;
				if (res>24)tdata[pix_line2]=0xffff;
				else tdata[pix_line2]=0;
			}
			else tdata[i*w+j]=0;
		
		}
	}
	
	
}
/*========================================================================*/
//全像素边缘经检测
/*========================================================================*/
void IMG_edge_full(uint16_t *pdata,uint16_t *tdata,uint16_t w,uint16_t h){			
	edge_rgbbuf_typdef rgbbuf;
	uint16_t k1,k2;
	uint32_t i,j,k,l,m,n;
	int8_t Gx[9]={-1,0,1,
				  -2,0,2,
				  -1,0,1};
	int8_t Gy[9]={1,2,1,
				  0,0,0,
				  -1,-2,-1};
	int resx,resy,resxr,resxg,resxb,resyr,resyg,resyb,res;
	rgbbuf.num[0]=0;
	rgbbuf.num[1]=1;
	rgbbuf.num[2]=2;
	for(n=0;n<320;n++){RGB565_split(pdata[n],rgbbuf.list[0][n]);};
	for(n=0;n<320;n++){RGB565_split(pdata[n+320],rgbbuf.list[1][n]);};
	for(n=0;n<320;n++){RGB565_split(pdata[n+640],rgbbuf.list[2][n]);};		
	for(i=0;i<240;i++){
		if(i>0&&i<239){
			m=(i+1)*320;
			if (rgbbuf.num[0]>=3)for(n=0;n<320;n++){RGB565_split(pdata[m+n],rgbbuf.list[0][n]);rgbbuf.num[0]=2;};
			if (rgbbuf.num[1]>=3)for(n=0;n<320;n++){RGB565_split(pdata[m+n],rgbbuf.list[1][n]);rgbbuf.num[1]=2;};
			if (rgbbuf.num[2]>=3)for(n=0;n<320;n++){RGB565_split(pdata[m+n],rgbbuf.list[2][n]);rgbbuf.num[2]=2;};
			for(j=0;j<320;j++){
				if(j>0&&j<319){
					resxr=0;resxg=0;resxb=0;
					resyr=0;resyg=0;resyb=0;	
					for(k=0;k<3;k++){
						for(l=0;l<3;l++){
							k2=j-1+l; k1=k*3+l;
							resxr+=Gx[k1]*rgbbuf.list[rgbbuf.num[k]][k2][0];
							resxg+=Gx[k1]*rgbbuf.list[rgbbuf.num[k]][k2][1];
							resxb+=Gx[k1]*rgbbuf.list[rgbbuf.num[k]][k2][2];
							resyr+=Gy[k1]*rgbbuf.list[rgbbuf.num[k]][k2][0];
							resyg+=Gy[k1]*rgbbuf.list[rgbbuf.num[k]][k2][1];
							resyb+=Gy[k1]*rgbbuf.list[rgbbuf.num[k]][k2][2];
						}
					}
					resx=abs(resxr)+abs(resxg)+abs(resxb);
					resy=abs(resyr)+abs(resyg)+abs(resyb);
					res=resx+resy;
					if (res>24)tdata[i* 320+j]=0xffff;
					else tdata[i*320+j]=0;
				}
			}
			rgbbuf.num[0]--;
			rgbbuf.num[1]--;
			rgbbuf.num[2]--;
		}
	}
}



/*========================================================================*/
//伪彩编码
/*========================================================================*/	
uint16_t color_code(uint16_t grayValue,uint16_t mode){
	uint16_t colorR,colorG,colorB;
    colorR=0;
    colorG=0;
    colorB=0;
    if (mode==0){
        colorR=abs(0-grayValue);
        colorG=abs(127-grayValue);
        colorB=abs(255-grayValue);
		}
    else if (mode==1){
        if ((grayValue>=0) && (grayValue<=63)){
            colorR=0;
            colorG=0;
            colorB=round(grayValue/64.0*255.0);
			}
        else if ((grayValue>=64) && (grayValue<=127)){
            colorR=0;
            colorG=round((grayValue-64)/64.0*255.0);
            colorB=round((127-grayValue)/64.0*255.0);
			}
        else if ((grayValue>=128) && (grayValue<=191)){
            colorR=round((grayValue-128)/64.0*255.0);
            colorG=255;
            colorB=0;
			}
        else if ((grayValue>=192) && (grayValue<=255)){
            colorR=255;
            colorG=round((255-grayValue)/64.0*255.0);
            colorB=0;
			}
		}
    else if (mode==2){ 
        if ((grayValue>=0) && (grayValue<=63)){
            colorR=0; 
            colorG=0; 
            colorB=round(grayValue/64.0*255.0); 
			}
        else if ((grayValue>=64) && (grayValue<=95)){
        
            colorR=round((grayValue-63)/32.0*127.0); 
            colorG=round((grayValue-63)/32.0*127.0); 
            colorB=255; 
			}
        else if ((grayValue>=96) && (grayValue<=127)){
        
            colorR=round((grayValue-95)/32.0*127.0)+128; 
            colorG=round((grayValue-95)/32.0*127.0)+128; 
            colorB=round((127-grayValue)/32.0*255.0); 
			}
        else if ((grayValue>=128) && (grayValue<=191)){
            colorR=255; 
            colorG=255; 
            colorB=0;
			}
        else if ((grayValue>=192) && (grayValue<=255)){
        
            colorR=255; 
            colorG=255; 
            colorB=round((grayValue-192)/64.0*255.0);
			}
		}
    else if (mode==3){  
        colorR=0; 
        colorG=0;
        colorB=0;
        if ((grayValue>=0) && (grayValue<=16)){
            colorR=0;} 
        else if ((grayValue>=17) && (grayValue<=140)){ 
            colorR=round((grayValue-16)/124.0*255.0);
			}
        else if ((grayValue>=141) && (grayValue<=255)){  
            colorR=255; 
			}
		
        if ((grayValue>=0) && (grayValue<=101)){
            colorG=0;
			}
        else if ((grayValue>=102) && (grayValue<=218)){
            colorG=round((grayValue-101)/117.0*255.0);
			}
        else if ((grayValue>=219) && (grayValue<=255)){  
            colorG=255; 
			}
        if ((grayValue>=0) && (grayValue<=91)){
            colorB=28+round((grayValue-0)/91.0*100.0);
			}
        else if ((grayValue>=92) && (grayValue<=120)){
            colorB=round((120-grayValue)/29.0*128.0);
			}
        else if ((grayValue>=129) && (grayValue<=214)){
            colorB=0;
			}			
        else if ((grayValue>=215) && (grayValue<=255)){
            colorB=round((grayValue-214)/41.0*255.0);
			}
		}
    else if (mode==4){ 
        if ((grayValue>=0) && (grayValue<=31)){
            colorR=0; 
            colorG=0; 
            colorB=round(grayValue/32.0*255.0);
			}			
        else if ((grayValue>=32) && (grayValue<=63)){
            colorR=0; 
            colorG=round((grayValue-32)/32.0*255.0); 
            colorB=255;
			}			
        else if ((grayValue>=64) && (grayValue<=95)){
            colorR=0; 
            colorG=255; 
            colorB=round((95-grayValue)/32.0*255.0);
			}
        else if ((grayValue>=96) && (grayValue<=127)){
            colorR=round((grayValue-96)/32.0*255.0); 
            colorG=255;
            colorB=0;
			}
        else if ((grayValue>=128) && (grayValue<=191)){
            colorR=255; 
            colorG=round((191-grayValue)/64.0*255.0); 
            colorB=0;
			}
        else if ((grayValue>=192) && (grayValue<=255)){
            colorR=255;
            colorG=round((grayValue-192)/64.0*255.0);
            colorB=round((grayValue-192)/64.0*255.0); 
			}
		}
    else if (mode==5){
        if ((grayValue>=0) && (grayValue<=63)){
            colorR=0;
            colorG=round((grayValue-0)/64.0*255.0);
            colorB=255;
			}
        else if ((grayValue>=64) && (grayValue<=95)){
            colorR=0; 
            colorG=255; 
            colorB=round((95-grayValue)/32.0*255.0);
			}
        else if ((grayValue>=96) && (grayValue<=127)){
            colorR=round((grayValue-96)/32.0*255.0); 
            colorG=255; 
            colorB=0;
			}
        else if ((grayValue>=128) && (grayValue<=191)){
            colorR=255 ;
            colorG=round((191-grayValue)/64.0*255.0); 
            colorB=0;
			}
        else if ((grayValue>=192) && (grayValue<=255)){
            colorR=255;
            colorG=round((grayValue-192)/64.0*255.0);
            colorB=round((grayValue-192)/64.0*255.0);
			}
		}
    else if (mode==6){
        if ((grayValue>=0) && (grayValue<=51)){
            colorR=0;
            colorG=grayValue*5;
            colorB=255;
			}
        else if ((grayValue>=52) && (grayValue<=102)){
            colorR=0;
            colorG=255;
            colorB=255-(grayValue-51)*5;
			}
        else if ((grayValue>=103) && (grayValue<=153)){
            colorR=(grayValue-102)*5;
            colorG=255;
            colorB=0;
			}
        else if ((grayValue>=154) && (grayValue<=204)){
            colorR=255;
            colorG=round(255.0-128.0*(grayValue-153.0)/51.0);
            colorB=0;
			}
        else if ((grayValue>=205) && (grayValue<=255)){
            colorR=255;
            colorG=round(127.0-127.0*(grayValue-204.0)/51.0);
            colorB=0;
			}
		}
    else if (mode==7){
        if ((grayValue>=0) && (grayValue<=63)){
            colorR=0;
            colorG=round((64-grayValue)/64.0*255.0);
            colorB=255;
			}
        else if ((grayValue>=64) && (grayValue<=127)){
            colorR=0 ;
            colorG=round((grayValue-64)/64.0*255.0);
            colorB=round((127-grayValue)/64.0*255.0);
			}
        else if ((grayValue>=128) && (grayValue<=191)){
            colorR=round((grayValue-128)/64.0*255.0);
            colorG=255;
            colorB=0;
			}
        else if ((grayValue>=192) && (grayValue<=255)){
            colorR=255;
            colorG=round((255-grayValue)/64.0*255.0);
            colorB=0;
			}
		}
    else if (mode==8){
        if ((grayValue>=0) && (grayValue<=63)){
            colorR=0;
            colorG=254-4*grayValue;
            colorB=255;
			}
        else if ((grayValue>=64) && (grayValue<=127)){
            colorR=0;
            colorG=4*grayValue-254;
            colorB=510-4*grayValue;
			}
        else if ((grayValue>=128) && (grayValue<=191)){
            colorR=4*grayValue-510;
            colorG=255;
            colorB=0;
			}
        else if ((grayValue>=192) && (grayValue<=255)){
            colorR=255;
            colorG=1022-4*grayValue;
            colorB=0;
			}
		}
    else{
        colorR=grayValue;
        colorG=grayValue; 
        colorB=grayValue;
	}
	return rgb_565(colorR,colorG,colorB);
}

/*========================================================================*/
//生成伪彩表
/*========================================================================*/
void color_listcode(uint16_t *color_list,uint16_t mode ){
	uint16_t i;
	for (i=0;i<256;i++){
		color_list[i]=color_code(i,mode);
	}
}




/*========================================================================*/
//获取最高/最低温度值及地址
/*========================================================================*/
void temp_limit(float *flist,float *maxtemp,uint16_t *max_addr,float *mintemp,uint16_t *min_addr){
	float max_buf=flist[0],min_buf=flist[0];
	uint16_t x_min,y_min,x_max,y_max,n,m;
	for(n=0;n<24;n++){
		for(m=0;m<32;m++){
			if(max_buf<flist[n*32+31-m]){
				max_buf=flist[n*32+31-m];
				x_max=m;
				y_max=n;
			}
			if(min_buf>flist[n*32+31-m]){
				min_buf=flist[n*32+31-m];
				x_min=m;
				y_min=n;
			}
		}
	}
	*maxtemp=max_buf;
	max_addr[0]=x_max;
	max_addr[1]=y_max;
	*mintemp=min_buf;
	min_addr[0]=x_min;
	min_addr[1]=y_min;	
}

/*========================================================================*/
//温度列表转灰度列表+插值+伪彩
/*========================================================================*/
void display_code(float *flist,uint16_t *show_list,uint16_t *color_list,uint16_t size,float tmax,float tmin){
	float tstep;
	uint32_t n,m,i,j,k;
	uint16_t intlist[32*24];
	tstep=(tmax-tmin)/255.0;
	for(n=0;n<24;n++){
		for(m=0;m<32;m++){
			intlist[n*32+m]=(int16_t)((flist[n*32+31-m]-tmin)/tstep)%256;
		}
	}
	for(i=0;i<24;i++){
		for(j=0;j<32;j++){
			show_list[i*3200+j*10]=intlist[i*32+j];
			if(j<31){
				float step=(intlist[i*32+j+1]-intlist[i*32+j])/9.0;
				for(k=0;k<9;k++){
					show_list[i*3200+j*10+1+k]=intlist[i*32+j]+(int16_t)(step*k);//行插值
				}
			}
			else{
				float step=(intlist[i*32+j]-intlist[i*32+j-1])/9.0;
				for(k=0;k<9;k++){
					int16_t buf;
					buf=intlist[i*32+j]+(int16_t)(step*k);
					if (buf<0)buf=0;
					if (buf>255)buf=255;
					show_list[i*3200+j*10+1+k]=buf;//行尾插值
				}
			}
		}
	}
	for(i=0;i<23;i++){
		for(j=0;j<320;j++){
			float step=(show_list[(i*10+10)*320+j]-show_list[i*3200+j])/9.0;
			for(k=0;k<9;k++){
				show_list[(i*10+k+1)*320+j]=show_list[i*3200+j]+(int16_t)(step*k);//列插值
			}
			if(i==22){			
				for(k=0;k<9;k++){
				int16_t buf;
				buf=show_list[230*320+j]+(int16_t)(step*k);
				if (buf<0)buf=0;
				if (buf>255)buf=255;
				show_list[(231+k)*320+j]=buf;//列插值
				}
			}	
		}
	}
	
	for(i=0;i<240;i++){
		for(j=0;j<320;j++){
			show_list[i*320+j]=color_list[show_list[i*320+j]];//灰度转伪彩
		}
	}
	
	
	
}

