
#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h>
#include "timer.h"
#include <stdint.h> 
#include <string.h>
#include <math.h> 

//#define W 3840 //resolucao da tela 4k
//#define H 2160
#define W 1920 //full hd
#define H 1080
//#define W 1280
//#define H 720 //hd
#define M 255 //cor maxima do ppm
#define N_THREADS 4


#define MAX_IT 1000 //teto de iteracoes
     
unsigned int r[H][W], g[H][W], b[H][W]; //matrix de cores
double z_real,z_img; //julia set
double x_offset,y_offset,scale;//transformacoes

struct
{
	double x,y;
}typedef vec2d; //vetor (double,double) equivale a um double[2]

void vec2d_equal(vec2d* a,vec2d b)
{
	a->x = b.x;
	a->y = b.y;
}

//
float mandelbrot(double c_real,double c_img)//calcula o numero de iteracoes para cada pixel
{
	double real = z_real,img = z_img;
	unsigned int i;
	for(i = 0; i < MAX_IT && real*real + img*img < 4; i++)
	{
		double new_real = real*real - img*img + c_real;
		img = 2*real*img + c_img;
		real = new_real;
	}
	return (float)i/MAX_IT;
}
//funcao que pega um pixel da imagem e projeta no plano (Reais x Complexos) e translada/escala para visualizacao
vec2d screen_to_plane(int x,int y)
{
	vec2d plane;
	plane.x = (double)(x - W/2);	
	plane.y = (double)(y - H/2);
	plane.x *= 1/(scale * H);
	plane.y *= 1/(scale * H);
	plane.x += x_offset;
	plane.y += y_offset;
	
	//plane.x *= (double)W/H;
	
	return plane;
}

void create_img_seq()//preenche as cores
{
	vec2d point;
	for(int i = 0; i<H;i++)
	{
		for(int j = 0; j<W; j++)
		{
			point = screen_to_plane(j,i);
			uint8_t color =(uint8_t) floor(M * mandelbrot(point.x,point.y));
			r[i][j] = color%((int)floor(0.9*M));//colorindo
			g[i][j] = color%((int)floor(0.3*M));
			b[i][j] = color%((int)floor(0.7*M));
		}
	}
}
void* create_img_conc01(void* arg)//preenche as cores, arg eh um int*
{
	vec2d point;
	for(int i = *(int*)arg; i<H;i+= N_THREADS)//estrategia de linhas
	{
		for(int j = 0; j<W; j++)
		{
			point = screen_to_plane(j,i);
			uint8_t color =(uint8_t) floor(M * mandelbrot(point.x,point.y));
			r[i][j] = color%((int)floor(0.9*M));//colorindo
			g[i][j] = color%((int)floor(0.3*M));
			b[i][j] = color%((int)floor(0.7*M));
		}
	}
	return NULL;
}

     
void write_ppm ( const char *filename ) //escreve o arquivo ppm
{
	FILE *out = fopen ( filename, "w" ); 
	fprintf ( out, "P6 %d %d %d\n", W, H, M );
	for ( int i=0; i<H; i++ )
		for ( int j=0; j<W; j++ )
		{ 
			fwrite ( &r[i][j], sizeof(uint8_t), 1, out ); /* Red */ 
			fwrite ( &g[i][j], sizeof(uint8_t), 1, out ); /* Green */ 
			fwrite ( &b[i][j], sizeof(uint8_t), 1, out ); /* Blue */ 
            } 
	fclose ( out ); 
} 

float distance(float x1,float y1,float x2,float y2)
{
	return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

void cicle_test()//teste para debug
{
	vec2d point;
	uint8_t color;
	vec2d center;
	vec2d_equal(&center,screen_to_plane(W/2,H/2));
	for(int i = 0;i < W;i++)
	{
		for(int j = 0;j < H;j++)
		{
			
			vec2d_equal(&point,screen_to_plane(i,j));
			if(distance(point.x,point.y,center.x,center.y) <= 0.5f)
				color = M;
			else
				color = 0;		
			//color = M/2;
			r[j][i] = color;
			g[j][i] = color;
			b[j][i] = color;			
		}
	}
}
     
int main (int argc,char* argv[]) 
{ 
	//inicializando valores default
	x_offset = 0;
	y_offset = 0;
	z_real = 0;
	z_img = 0;
	scale = 1;
	char filename[255];
	if(argc >=2)
		strcpy(filename,argv[1]);
	else
	{
		printf("escreva o nome do arquivo\n");
		return 1;
	}
	if(argc >= 3)
		scale = atof(argv[2]);
	if(argc >= 4)
		x_offset = atof(argv[3]);
	if(argc >= 5)
		y_offset = atof(argv[4]);
	strcat(filename,"_seq.ppm");
	double inicio,fim,tempo_seq,tempo_conc;
	GET_TIME(inicio);
	
	//cicle_test();
	//write_ppm("cicle_test.ppm");
	
	//forma sequencial
	create_img_seq();
	GET_TIME(fim);
	tempo_seq = fim - inicio;
	write_ppm ( filename ); 
	printf("\"%s\", tempo de execucao sequencial: %f\n",filename,tempo_seq);
	//forma concorrente
	pthread_t pthread_ids[N_THREADS];
	int ids[N_THREADS];
	strcpy(filename,argv[1]);
	strcat(filename,"_conc.ppm");
	GET_TIME(inicio);
	for(int i = 0;i < N_THREADS; i++)
	{
		ids[i] = i;
		pthread_create(&pthread_ids[i], NULL, create_img_conc01,&ids[i]);
	}
	for(int i = 0; i < N_THREADS; i++)
	{
		pthread_join(pthread_ids[i],NULL);
	}
	GET_TIME(fim);
	tempo_conc = fim - inicio;
	write_ppm( filename );
	printf("\"%s\" tempo de execucao concorrente: %f, aceleracao: %f\n",filename,tempo_conc,tempo_seq/tempo_conc);
	return 0;
} 



















