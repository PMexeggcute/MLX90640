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
// 温度 → 灰度 → 逐块双线性插值放大（32×24 → 128×160） → 伪彩
// 每个内循环只处理一个 6×4 显示块（不连续插值）
// 已修复 vx 未定义问题
/*========================================================================*/
void display_code(float *flist,uint16_t *color_list,uint16_t size,float tmax,float tmin)
{
    const int SRC_W = 32;
    const int SRC_H = 24;
    const int DST_W = 128;
    const int DST_H = 160;

    uint8_t gray_src[SRC_H][SRC_W];   // 原始灰度图，仅 768 字节

    float tstep = (tmax - tmin) / 255.0f;

    /*================= 1. 温度 → 灰度（32×24），带左右翻转 =================*/
    for (int sy = 0; sy < SRC_H; sy++)
    {
        for (int sx = 0; sx < SRC_W; sx++)
        {
            float val = (flist[sy * SRC_W + (SRC_W - 1 - sx)] - tmin) / tstep;
            if (val < 0)   val = 0;
            if (val > 255) val = 255;
            gray_src[sy][sx] = (uint8_t)val;
        }
    }

    /*================= 2. 逐块双线性插值 + 伪彩（可直接刷屏） =================*/
    for (int sy = 0; sy < SRC_H; sy++)
    {
        for (int sx = 0; sx < SRC_W; sx++)
        {
            int dst_y_base = sy * 6;    // 当前块在显示图像中的起始行
            int dst_x_base = sx * 4;    // 当前块起始列

            // 获取当前原始像素及其右、下、右下四个角的灰度值
            uint8_t tl = gray_src[sy][sx];  // top-left（当前像素）

            // top-right
            uint8_t tr = (sx < SRC_W - 1) ? gray_src[sy][sx + 1]
                                         : (uint8_t)(tl + (tl - gray_src[sy][sx - 1]));  // 延伸
            if (tr > 255) tr = 255;

            // bottom-left
            uint8_t bl = (sy < SRC_H - 1) ? gray_src[sy + 1][sx]
                                         : (uint8_t)(tl + (tl - gray_src[sy - 1][sx]));  // 延伸
            if (bl > 255) bl = 255;

            // bottom-right（优先真实值，否则推算）
            uint8_t br;
            if (sx < SRC_W - 1 && sy < SRC_H - 1)
                br = gray_src[sy + 1][sx + 1];
            else
                br = (uint8_t)(tl + (tr - tl) + (bl - tl));  // 向量加法推算
            if (br > 255) br = 255;

            /*================= 当前 6×4 块内双线性插值 =================*/
            for (int by = 0; by < 6; by++)
            {
                int dy = dst_y_base + by;
                if (dy >= DST_H) continue;  // 超出 160 行（实际只到 143）

                float fy = by / 5.0f;        // 纵向插值比例：0.0 ~ 1.0（第0行和第5行对应原始行）

                // 先横向插值得到上下两条边
                // uint16_t top    = (uint16_t)tl + (uint16_t)((tr - tl) * (sx * 4 + 3) / 3.0f * fy);
                // 正确方式：先算上下两行的插值点
                uint16_t left   = (uint16_t)tl + (uint16_t)((bl - tl) * fy);
                uint16_t right  = (uint16_t)tr + (uint16_t)((br - tr) * fy);

                for (int bx = 0; bx < 4; bx++)
                {
                    int dx = dst_x_base + bx;

                    float fx = bx / 3.0f;    // 横向插值比例：0.0 ~ 1.0

                    // 双线性插值：先纵向得到左右，再横向混合
                    uint16_t gray = left + (uint16_t)((right - left) * fx);
                    if (gray > 255) gray = 255;

                    uint16_t color = color_list[gray];

                    // 直接写像素到屏幕（推荐！不占 RAM）
                    LCD_DrawPoint(dx, dy, color);

                    // 如果你暂时还想保留缓冲，可改为：
                    // show_list[dy][dx] = color;
                }
            }
        }
    }

    /*================= 3. 填充底部剩余 16 行（第 144~159 行）=================*/
    int base_y = (SRC_H - 1) * 6;  // 第 138 行（最后一组块的第0行）

    for (int dx = 0; dx < DST_W; dx++)
    {
        int sx = dx / 4;
        uint8_t base_gray = gray_src[SRC_H - 1][sx];

        // 计算纵向延伸步长（使用最后一组的趋势）
        uint8_t prev_gray = (SRC_H > 1) ? gray_src[SRC_H - 2][sx] : base_gray;
        float step = (float)(base_gray - prev_gray) / 5.0f;

        for (int offset = 1; offset <= 16; offset++)
        {
            int dy = base_y + offset;
            uint16_t gray = (uint16_t)(base_gray + step * offset);
            if (gray > 255) gray = 255;

            uint16_t color = color_list[gray];
            LCD_DrawPoint(dx, dy, color);
        }
    }
}

