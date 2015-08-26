/* An example of how to read an image (img.tif) from file using freeimage and then
display that image using openGL's drawPixelCommand. Also allow the image to be saved
to backup.tif with freeimage and a simple thresholding filter to be applied to the image.
Conversion by Lee Rozema.
Added triangle draw routine, fixed memory leak and improved performance by Robert Flack (2008)
*/

#include <stdlib.h>
#include<math.h>

/*
JEFF KEKENJ 4759171
jk10kp@brocku.ca
Assignment 1 Part 1
COSC 3p98

Functions listen in Case statements
*/

// Visual Studio 2008 no longer compiles with this include.
// #include <GL/gl.h>

#include <stdio.h>
#include <malloc.h>
#include <freeglut.h>
#include <FreeImage.h>

int avg[3][3] = {{1,1,1},{1,1,1},{1,1,1}};
//int sobelHorizontal[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};
int sobelHorizontal[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};
int sobelVerticle[3][3] = {{1,0,-1},{2,0,-2},{1,0,-1}};
int Colors[8][3] = {{255,255,255},{0,0,0},{255,0,0},{0,255,0},{0,0,255},{255,165,0},{255,255,0},{128,0,128}};

//

//the pixel structure
typedef struct {
	GLubyte r, g, b;
} pixel;

//the global structure
typedef struct {
	pixel *data;
	pixel *buffer;
	pixel *temp;
	int w, h;
} glob;
glob global;

//read image
pixel *read_img(char *name, int *width, int *height) {
	FIBITMAP *image;
	int i,j,pnum;
	RGBQUAD aPixel;
	pixel *data;

	if((image = FreeImage_Load(FIF_TIFF, name, 0)) == NULL) {
		return NULL;
	}      
	*width = FreeImage_GetWidth(image);
	*height = FreeImage_GetHeight(image);

	data = (pixel *)malloc((*height)*(*width)*sizeof(pixel *));
	pnum=0;
	for(i = 0 ; i < (*height) ; i++) {
		for(j = 0 ; j < (*width) ; j++) {
			FreeImage_GetPixelColor(image, j, i, &aPixel);
			data[pnum].r = (aPixel.rgbRed);
			data[pnum].g = (aPixel.rgbGreen);
			data[pnum++].b = (aPixel.rgbBlue);
		}
	}
	FreeImage_Unload(image);
	return data;
}//read_img

//write_img
void write_img(char *name, pixel *data, int width, int height) {
	FIBITMAP *image;
	RGBQUAD aPixel;
	int i,j;

	image = FreeImage_Allocate(width, height, 24, 0, 0, 0);
	if(!image) {
		perror("FreeImage_Allocate");
		return;
	}
	for(i = 0 ; i < height ; i++) {
		for(j = 0 ; j < width ; j++) {
			aPixel.rgbRed = data[i*width+j].r;
			aPixel.rgbGreen = data[i*width+j].g;
			aPixel.rgbBlue = data[i*width+j].b;

			FreeImage_SetPixelColor(image, j, i, &aPixel);
		}
	}
	if(!FreeImage_Save(FIF_TIFF, image, name, 0)) {
		perror("FreeImage_Save");
	}
	FreeImage_Unload(image);
}//write_img


/*draw the image - it is already in the format openGL requires for glDrawPixels*/
void display_image(void)
{
	glDrawPixels(global.w, global.h, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)global.data);	
	glFlush();
}//display_image()

// Read the screen image back to the data buffer after drawing to it
void draw_triangle(void)
{
	glDrawPixels(global.w, global.h, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)global.data);
	glBegin(GL_TRIANGLES);
	glColor3f(1.0,0,0);
	glVertex2i(rand()%global.w,rand()%global.h);
	glColor3f(0,1.0,0);
	glVertex2i(rand()%global.w,rand()%global.h);
	glColor3f(0,0,1.0);
	glVertex2i(rand()%global.w,rand()%global.h);
	glEnd();
	glFlush();
	glReadPixels(0,0,global.w,global.h,GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)global.data);
}

/* A simple thresholding filter.
*/
void MyFilter(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;

	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			if (Im[x+y*myIm_Width].b > 128)
				Im[x+y*myIm_Width].b = 255;
			else 
				Im[x+y*myIm_Width].b = 0;

			if (Im[x+y*myIm_Width].g > 128)
				Im[x+y*myIm_Width].g = 255;
			else 
				Im[x+y*myIm_Width].g = 0;

			if (Im[x+y*myIm_Width].r > 128)
				Im[x+y*myIm_Width].r = 255;
			else 
				Im[x+y*myIm_Width].r = 0;
		}

	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn
}//My_Filter

void effect1(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;

	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			int avg = (Im[x+y*myIm_Width].r+Im[x+y*myIm_Width].g+Im[x+y*myIm_Width].b/3);
			Im[x+y*myIm_Width].r = avg;
			Im[x+y*myIm_Width].g = avg;
			Im[x+y*myIm_Width].b = avg;
		}

	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn
}//Custom Effect

void greyScale(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			int avg = ((Im[x+y*myIm_Width].r+Im[x+y*myIm_Width].g+Im[x+y*myIm_Width].b)/3); 
			Im[x+y*myIm_Width].r = avg;
			Im[x+y*myIm_Width].g = avg;
			Im[x+y*myIm_Width].b = avg;
		}

	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn
}//My_Filter

void monochrome(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			int avg = ((Im[x+y*myIm_Width].r+Im[x+y*myIm_Width].g+Im[x+y*myIm_Width].b)/3); 
			if (avg > 127){
				Im[x+y*myIm_Width].r = 255;
				Im[x+y*myIm_Width].g = 255;
				Im[x+y*myIm_Width].b = 255;
			}//white
			else{
				Im[x+y*myIm_Width].r = 000;
				Im[x+y*myIm_Width].g = 000;
				Im[x+y*myIm_Width].b = 000;
			}//black
		}

	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn
}//My_Filter

void NTSC(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			//int avg = (Im[x+y*myIm_Width].r*0.299)+(Im[x+y*myIm_Width].g*0.587)+(Im[x+y*myIm_Width].b*.114); 
			Im[x+y*myIm_Width].r = Im[x+y*myIm_Width].r*0.299;
			Im[x+y*myIm_Width].g = Im[x+y*myIm_Width].g*0.587;
			Im[x+y*myIm_Width].b = Im[x+y*myIm_Width].b*.114;
		}

	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn
}//My_Filter

void invert(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			Im[x+y*myIm_Width].r = 255-Im[x+y*myIm_Width].r;
			Im[x+y*myIm_Width].g = 255-Im[x+y*myIm_Width].g;
			Im[x+y*myIm_Width].b = 255-Im[x+y*myIm_Width].b;
		}

	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn
}//My_Filter

void Red(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			Im[x+y*myIm_Width].g = 000;
			Im[x+y*myIm_Width].b = 000;
		}

	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn
}//My_Filter

void Blue(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			Im[x+y*myIm_Width].r = 000;
			Im[x+y*myIm_Width].g = 000;
		}

	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn
}//My_Filter

void Green(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			Im[x+y*myIm_Width].r = 000;
			Im[x+y*myIm_Width].b = 00;
		}

	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn
}//My_Filter

void Max(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			if ((x > 0 && y > 0) && (x < myIm_Width && y < myIm_Height)){
				//red
				int tempRed = 0;
				int tempGreen = 0;
				int tempBlue = 0;
				int z;
				int q;
				tempRed = Im[x+y*myIm_Width].r;
				tempBlue = Im[x+y*myIm_Width].b;
				tempGreen = Im[x+y*myIm_Width].g;
				for (z=(x-1);z<(x+1);z++){
					for (q=(y-1);q<(y+1);q++){
						if (tempRed < Im[z+q*myIm_Width].r){
							tempRed = Im[z+q*myIm_Width].r;
						}
						if (tempGreen < Im[z+q*myIm_Width].g){
							tempGreen = Im[z+q*myIm_Width].g;
						}
						if (tempBlue < Im[z+q*myIm_Width].b){
							tempBlue = Im[z+q*myIm_Width].b;
						}
					}
				}
				global.temp[x+y*myIm_Width].r = tempRed;
				global.temp[x+y*myIm_Width].g = tempGreen;
				global.temp[x+y*myIm_Width].b = tempBlue;
			}//ignore edge
		}		
	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn	
}


void Min(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			if ((x > 0 && y > 0) && (x < myIm_Width && y < myIm_Height)){
				//red
				int tempRed = 0;
				int tempGreen = 0;
				int tempBlue = 0;
				int z;
				int q;
				tempRed = Im[x+y*myIm_Width].r;
				tempBlue = Im[x+y*myIm_Width].b;
				tempGreen = Im[x+y*myIm_Width].g;
				for (z=(x-1);z<(x+1);z++){
					for (q=(y-1);q<(y+1);q++){
						if (tempRed > Im[z+q*myIm_Width].r){
							tempRed = Im[z+q*myIm_Width].r;
						}
						if (tempGreen > Im[z+q*myIm_Width].g){
							tempGreen = Im[z+q*myIm_Width].g;
						}
						if (tempBlue > Im[z+q*myIm_Width].b){
							tempBlue = Im[z+q*myIm_Width].b;
						}
					}
				}
				global.temp[x+y*myIm_Width].r = tempRed;
				global.temp[x+y*myIm_Width].g = tempGreen;
				global.temp[x+y*myIm_Width].b = tempBlue;
			}//ignore edge
		}		
	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn	
}

//intensify 
//red
void redIntensify(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){			
			Im[x+y*myIm_Width].r += (Im[x+y*myIm_Width].r*.20);
		}

	glutPostRedisplay();
}
//blue
void blueIntensify(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){			
			Im[x+y*myIm_Width].b += (Im[x+y*myIm_Width].b*.20);
		}

	glutPostRedisplay();
}
//green
void greenIntensify(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){			
			Im[x+y*myIm_Width].g += (Im[x+y*myIm_Width].g*.20);
		}

	glutPostRedisplay();
}

void reset(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){			
			Im[x+y*myIm_Width].r = global.buffer[x+y*myIm_Width].r;
			Im[x+y*myIm_Width].b = global.buffer[x+y*myIm_Width].b;
			Im[x+y*myIm_Width].g = global.buffer[x+y*myIm_Width].g;
		}

	glutPostRedisplay();
}

//Part D
void MaskFilter(pixel* Im, int myIm_Width, int myIm_Height, int mask[3][3]){
	int x,y;
	//int avg[9] = {1,1,1,1,1,1,1,1,1};
	//int avg[3][3] = {{1,1,1},{1,1,1},{1,1,1}};
	//int sum = sizeof(avg)*sizeof(avg[0]);
	//int sum = sizeof(mask) / sizeof(mask[0]);
	int sum = 0;
	int i, j;
	for (i = 0; i<3;i++){
		for (j = 0; j<3;j++){
			sum += mask[i][j];
		}
	}
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			if ((x > 0 && y > 0) && (x < myIm_Width && y < myIm_Height)){					
				int tempRed = 0;
				int tempBlue = 0;
				int tempGreen = 0;	

				tempRed += Im[(x-1)+(y-1)*myIm_Width].r*(mask[0][0]);
				tempRed += Im[(x)+(y-1)*myIm_Width].r*(mask[1][0]);
				tempRed += Im[(x+1)+(y-1)*myIm_Width].r*(mask[2][0]);
				tempRed += Im[(x-1)+(y)*myIm_Width].r*(mask[0][1]);
				tempRed += Im[(x)+(y)*myIm_Width].r*(mask[1][1]);
				tempRed += Im[(x+1)+(y)*myIm_Width].r*(mask[2][1]);
				tempRed += Im[(x-1)+(y+1)*myIm_Width].r*(mask[0][2]);
				tempRed += Im[(x)+(y+1)*myIm_Width].r*(mask[1][2]);
				tempRed += Im[(x+1)+(y+1)*myIm_Width].r*(mask[2][2]);

				tempBlue += Im[(x-1)+(y-1)*myIm_Width].b*(mask[0][0]);
				tempBlue += Im[(x)+(y-1)*myIm_Width].b*(mask[1][0]);
				tempBlue += Im[(x+1)+(y-1)*myIm_Width].b*(mask[2][0]);
				tempBlue += Im[(x-1)+(y)*myIm_Width].b*(mask[0][1]);
				tempBlue += Im[(x)+(y)*myIm_Width].b*(mask[1][1]);
				tempBlue += Im[(x+1)+(y)*myIm_Width].b*(mask[2][1]);
				tempBlue += Im[(x-1)+(y+1)*myIm_Width].b*(mask[0][2]);
				tempBlue += Im[(x)+(y+1)*myIm_Width].b*(mask[1][2]);
				tempBlue += Im[(x+1)+(y+1)*myIm_Width].b*(mask[2][2]);

				tempGreen += Im[(x-1)+(y-1)*myIm_Width].g*(mask[0][0]);
				tempGreen += Im[(x)+(y-1)*myIm_Width].g*(mask[1][0]);
				tempGreen += Im[(x+1)+(y-1)*myIm_Width].g*(mask[2][0]);
				tempGreen += Im[(x-1)+(y)*myIm_Width].g*(mask[0][1]);
				tempGreen += Im[(x)+(y)*myIm_Width].g*(mask[1][1]);
				tempGreen += Im[(x+1)+(y)*myIm_Width].g*(mask[2][1]);
				tempGreen += Im[(x-1)+(y+1)*myIm_Width].g*(mask[0][2]);
				tempGreen += Im[(x)+(y+1)*myIm_Width].g*(mask[1][2]);
				tempGreen += Im[(x+1)+(y+1)*myIm_Width].g*(mask[2][2]);

				tempRed = tempRed/sum;
				tempBlue = tempBlue/sum;
				tempGreen = tempGreen/sum;

				global.temp[x+y*myIm_Width].r = tempRed;
				global.temp[x+y*myIm_Width].g = tempGreen;
				global.temp[x+y*myIm_Width].b = tempBlue;
			}//ignore edge
		}		
	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn	
}

//Part Sobel
void SobelH(pixel* Im, int myIm_Width, int myIm_Height, int mask[][3]){
	int x,y;
						printf("%d",mask[0][0]);
						printf("%d",mask[0][1]);
						printf("%d",mask[0][2]);

						printf("%d",mask[1][0]);
						printf("%d",mask[1][1]);
						printf("%d",mask[1][2]);			
						

						printf("%d",mask[2][0]);
						printf("%d",mask[2][1]);				
						printf("%d",mask[2][2]);
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			if ((x > 0 && y > 0) && (x < myIm_Width && y < myIm_Height)){
				int tempRed = 0;
				int tempBlue = 0;
				int tempGreen = 0;				

					/*tempRed += Im[(x-1)+(y-1)*myIm_Width].r*(-1);
					tempRed += Im[(x)+(y-1)*myIm_Width].r*(-2);
					tempRed += Im[(x+1)+(y-1)*myIm_Width].r*(-1);
					tempRed += Im[(x-1)+(y)*myIm_Width].r*(0);
					tempRed += Im[(x)+(y)*myIm_Width].r*(0);
					tempRed += Im[(x+1)+(y)*myIm_Width].r*(0);
					tempRed += Im[(x-1)+(y+1)*myIm_Width].r*(1);
					tempRed += Im[(x)+(y+1)*myIm_Width].r*(2);
					tempRed += Im[(x+1)+(y+1)*myIm_Width].r*(1);

					tempBlue += Im[(x-1)+(y-1)*myIm_Width].b*(-1);
					tempBlue += Im[(x)+(y-1)*myIm_Width].b*(-2);
					tempBlue += Im[(x+1)+(y-1)*myIm_Width].b*(-1);
					tempBlue += Im[(x-1)+(y)*myIm_Width].b*(0);
					tempBlue += Im[(x)+(y)*myIm_Width].b*(0);
					tempBlue += Im[(x+1)+(y)*myIm_Width].b*(0);
					tempBlue += Im[(x-1)+(y+1)*myIm_Width].b*(1);
					tempBlue += Im[(x)+(y+1)*myIm_Width].b*(2);
					tempBlue += Im[(x+1)+(y+1)*myIm_Width].b*(1);

					tempGreen += Im[(x-1)+(y-1)*myIm_Width].g*(-1);
					tempGreen += Im[(x)+(y-1)*myIm_Width].g*(-2);
					tempGreen += Im[(x+1)+(y-1)*myIm_Width].g*(-1);
					tempGreen += Im[(x-1)+(y)*myIm_Width].g*(0);
					tempGreen += Im[(x)+(y)*myIm_Width].g*(0);
					tempGreen += Im[(x+1)+(y)*myIm_Width].g*(0);
					tempGreen += Im[(x-1)+(y+1)*myIm_Width].g*(1);
					tempGreen += Im[(x)+(y+1)*myIm_Width].g*(2);
					tempGreen += Im[(x+1)+(y+1)*myIm_Width].g*(1);*/


				/*
						printf("%d",mask[0][0]);
						printf("%d",mask[0][1]);
						printf("%d",mask[0][2]);

						printf("%d",mask[1][0]);
						printf("%d",mask[1][1]);
						printf("%d",mask[1][2]);			
						

						printf("%d",mask[2][0]);
						printf("%d",mask[2][1]);				
						printf("%d",mask[2][2]);
				*/

					tempRed += Im[(x-1)+(y+1)*myIm_Width].r*(mask[0][0]);
					tempRed += Im[(x)+(y+1)*myIm_Width].r*(mask[0][1]);
					tempRed += Im[(x+1)+(y+1)*myIm_Width].r*(mask[0][2]);
					tempRed += Im[(x-1)+(y)*myIm_Width].r*(mask[1][0]);
					tempRed += Im[(x)+(y)*myIm_Width].r*(mask[1][1]);
					tempRed += Im[(x+1)+(y)*myIm_Width].r*(mask[1][2]);
					tempRed += Im[(x-1)+(y-1)*myIm_Width].r*(mask[2][0]);
					tempRed += Im[(x)+(y-1)*myIm_Width].r*(mask[2][1]);
					tempRed += Im[(x+1)+(y-1)*myIm_Width].r*(mask[2][2]);

					tempBlue += Im[(x-1)+(y+1)*myIm_Width].b*(mask[0][0]);
					tempBlue += Im[(x)+(y+1)*myIm_Width].b*(mask[0][1]);
					tempBlue += Im[(x+1)+(y+1)*myIm_Width].b*(mask[0][2]);
					tempBlue += Im[(x-1)+(y)*myIm_Width].b*(mask[1][0]);
					tempBlue += Im[(x)+(y)*myIm_Width].b*(mask[1][1]);
					tempBlue += Im[(x+1)+(y)*myIm_Width].b*(mask[1][2]);
					tempBlue += Im[(x-1)+(y-1)*myIm_Width].b*(mask[2][0]);
					tempBlue += Im[(x)+(y-1)*myIm_Width].b*(mask[2][1]);
					tempBlue += Im[(x+1)+(y-1)*myIm_Width].b*(mask[2][2]);

					tempGreen += Im[(x-1)+(y+1)*myIm_Width].g*(mask[0][0]);
					tempGreen += Im[(x)+(y+1)*myIm_Width].g*(mask[0][1]);
					tempGreen += Im[(x+1)+(y+1)*myIm_Width].g*(mask[0][2]);
					tempGreen += Im[(x-1)+(y)*myIm_Width].g*(mask[1][0]);
					tempGreen += Im[(x)+(y)*myIm_Width].g*(mask[1][1]);
					tempGreen += Im[(x+1)+(y)*myIm_Width].g*(mask[1][2]);
					tempGreen += Im[(x-1)+(y-1)*myIm_Width].g*(mask[2][0]);
					tempGreen += Im[(x)+(y-1)*myIm_Width].g*(mask[2][1]);
					tempGreen += Im[(x+1)+(y-1)*myIm_Width].g*(mask[2][2]);

					if (tempRed<0) tempRed = 0;
					if (tempRed>255) tempRed = 255;
					if (tempBlue<0) tempBlue = 0;
					if (tempBlue>255) tempBlue = 255;
					if (tempGreen<0) tempGreen = 0;
					if (tempGreen>255) tempGreen = 255;
					global.temp[x+y*myIm_Width].r = tempRed;
					global.temp[x+y*myIm_Width].g = tempGreen;
					global.temp[x+y*myIm_Width].b = tempBlue;

			}//ignore edge
		}		
	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn	
}

//Final Sobel
void FinalSobal(pixel* Im, int myIm_Width, int myIm_Height, int horizontal[][3], int vertical[][3]){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			if ((x > 0 && y > 0) && (x < myIm_Width && y < myIm_Height)){
				int tempRedH = 0;
				int tempBlueH = 0;
				int tempGreenH = 0;	
				int tempRedV = 0;
				int tempBlueV = 0;
				int tempGreenV = 0;	

					tempRedH += Im[(x-1)+(y+1)*myIm_Width].r*(horizontal[0][0]);
					tempRedH += Im[(x)+(y+1)*myIm_Width].r*(horizontal[0][1]);
					tempRedH += Im[(x+1)+(y+1)*myIm_Width].r*(horizontal[0][2]);
					tempRedH += Im[(x-1)+(y)*myIm_Width].r*(horizontal[1][0]);
					tempRedH += Im[(x)+(y)*myIm_Width].r*(horizontal[1][1]);
					tempRedH += Im[(x+1)+(y)*myIm_Width].r*(horizontal[1][2]);
					tempRedH += Im[(x-1)+(y-1)*myIm_Width].r*(horizontal[2][0]);
					tempRedH += Im[(x)+(y-1)*myIm_Width].r*(horizontal[2][1]);
					tempRedH += Im[(x+1)+(y-1)*myIm_Width].r*(horizontal[2][2]);

					tempBlueH += Im[(x-1)+(y+1)*myIm_Width].b*(horizontal[0][0]);
					tempBlueH += Im[(x)+(y+1)*myIm_Width].b*(horizontal[0][1]);
					tempBlueH += Im[(x+1)+(y+1)*myIm_Width].b*(horizontal[0][2]);
					tempBlueH += Im[(x-1)+(y)*myIm_Width].b*(horizontal[1][0]);
					tempBlueH += Im[(x)+(y)*myIm_Width].b*(horizontal[1][1]);
					tempBlueH += Im[(x+1)+(y)*myIm_Width].b*(horizontal[1][2]);
					tempBlueH += Im[(x-1)+(y-1)*myIm_Width].b*(horizontal[2][0]);
					tempBlueH += Im[(x)+(y-1)*myIm_Width].b*(horizontal[2][1]);
					tempBlueH += Im[(x+1)+(y-1)*myIm_Width].b*(horizontal[2][2]);

					tempGreenH += Im[(x-1)+(y+1)*myIm_Width].g*(horizontal[0][0]);
					tempGreenH += Im[(x)+(y+1)*myIm_Width].g*(horizontal[0][1]);
					tempGreenH += Im[(x+1)+(y+1)*myIm_Width].g*(horizontal[0][2]);
					tempGreenH += Im[(x-1)+(y)*myIm_Width].g*(horizontal[1][0]);
					tempGreenH += Im[(x)+(y)*myIm_Width].g*(horizontal[1][1]);
					tempGreenH += Im[(x+1)+(y)*myIm_Width].g*(horizontal[1][2]);
					tempGreenH += Im[(x-1)+(y-1)*myIm_Width].g*(horizontal[2][0]);
					tempGreenH += Im[(x)+(y-1)*myIm_Width].g*(horizontal[2][1]);
					tempGreenH += Im[(x+1)+(y-1)*myIm_Width].g*(horizontal[2][2]);


					tempRedV += Im[(x-1)+(y+1)*myIm_Width].r*(vertical[0][0]);
					tempRedV += Im[(x)+(y+1)*myIm_Width].r*(vertical[0][1]);
					tempRedV += Im[(x+1)+(y+1)*myIm_Width].r*(vertical[0][2]);
					tempRedV += Im[(x-1)+(y)*myIm_Width].r*(vertical[1][0]);
					tempRedV += Im[(x)+(y)*myIm_Width].r*(vertical[1][1]);
					tempRedV += Im[(x+1)+(y)*myIm_Width].r*(vertical[1][2]);
					tempRedV += Im[(x-1)+(y-1)*myIm_Width].r*(vertical[2][0]);
					tempRedV += Im[(x)+(y-1)*myIm_Width].r*(vertical[2][1]);
					tempRedV += Im[(x+1)+(y-1)*myIm_Width].r*(vertical[2][2]);

					tempBlueV += Im[(x-1)+(y+1)*myIm_Width].b*(vertical[0][0]);
					tempBlueV += Im[(x)+(y+1)*myIm_Width].b*(vertical[0][1]);
					tempBlueV += Im[(x+1)+(y+1)*myIm_Width].b*(vertical[0][2]);
					tempBlueV += Im[(x-1)+(y)*myIm_Width].b*(vertical[1][0]);
					tempBlueV += Im[(x)+(y)*myIm_Width].b*(vertical[1][1]);
					tempBlueV += Im[(x+1)+(y)*myIm_Width].b*(vertical[1][2]);
					tempBlueV += Im[(x-1)+(y-1)*myIm_Width].b*(vertical[2][0]);
					tempBlueV += Im[(x)+(y-1)*myIm_Width].b*(vertical[2][1]);
					tempBlueV += Im[(x+1)+(y-1)*myIm_Width].b*(vertical[2][2]);

					tempGreenV += Im[(x-1)+(y+1)*myIm_Width].g*(vertical[0][0]);
					tempGreenV += Im[(x)+(y+1)*myIm_Width].g*(vertical[0][1]);
					tempGreenV += Im[(x+1)+(y+1)*myIm_Width].g*(vertical[0][2]);
					tempGreenV += Im[(x-1)+(y)*myIm_Width].g*(vertical[1][0]);
					tempGreenV += Im[(x)+(y)*myIm_Width].g*(vertical[1][1]);
					tempGreenV += Im[(x+1)+(y)*myIm_Width].g*(vertical[1][2]);
					tempGreenV += Im[(x-1)+(y-1)*myIm_Width].g*(vertical[2][0]);
					tempGreenV += Im[(x)+(y-1)*myIm_Width].g*(vertical[2][1]);
					tempGreenV += Im[(x+1)+(y-1)*myIm_Width].g*(vertical[2][2]);

					if (tempRedH<0) tempRedH = 0;
					if (tempRedV<0) tempRedV = 0;
					if (tempRedH>255) tempRedH = 255;
					if (tempRedV>255) tempRedV = 255;
					if (tempBlueH<0) tempBlueH = 0;
					if (tempBlueV<0) tempBlueV = 0;
					if (tempBlueH>255) tempBlueH = 255;
					if (tempBlueV>255) tempBlueV = 255;
					if (tempGreenH<0) tempGreenH = 0;
					if (tempGreenV<0) tempGreenV = 0;
					if (tempGreenH>255) tempGreenH = 255;
					if (tempGreenV>255) tempGreenV = 255;
					
					global.temp[x+y*myIm_Width].r = sqrt((double)(tempRedH^2)+(double)(tempRedV^2));
					global.temp[x+y*myIm_Width].g = sqrt((double)(tempGreenH^2)+(double)(tempGreenV^2));
					global.temp[x+y*myIm_Width].b = sqrt((double)(tempBlueH^2)+(double)(tempBlueV^2));

			}//ignore edge
		}		
	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn	
}

void Quantize(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	int distance = 200;
	int Eucdistance = 0;
	int arrPos = 0;
	int i, j;
	int localColor[8][3] = {{255,255,255},{0,0,0},{255,0,0},{0,255,0},{0,0,255},{255,165,0},{255,255,0},{128,0,128}};

	/*for (i = 0; i < 8; i++) {
        for (j = 0; j < 3; j++) {
            //localColor[i][j] = '.';
            printf("%d ", localColor[i][j]);
        }
        printf("\n");
    }*/

	//printf("%d",localColor[4][2]);
	//printf("%d", sizeof(Colors) / sizeof(Colors[0]));
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			//Im[(x-1)+(y+1)*myIm_Width].r
			for (i=0; i<(sizeof(localColor) / sizeof(localColor[0]));i++) {
				//Eucdistance = ((Colors[i][1]-Im[x+y*myIm_Width].r^2+(Colors[i][2]-Im[(x-1)+(y+1)*myIm_Width].b)^2+(Colors[i][3]-Im[(x-1)+(y+1)*myIm_Width].g)^2);

				Eucdistance = ((localColor[i][1]-Im[x+y*myIm_Width].r)^2+(localColor[i][2]-Im[x+y*myIm_Width].g)^2+(localColor[i][3]-Im[x+y*myIm_Width].b)^2);
				//Eucdistance = ((Im[x+y*myIm_Width].r-Colors[i][1])^2+(Im[x+y*myIm_Width].g-Colors[i][2])^2+(Im[x+y*myIm_Width].b-Colors[i][3])^2);
				//if (Eucdistance<0) Eucdistance = 0;
				if (Eucdistance < distance){					
					distance = Eucdistance;
					//printf("%d",distance);
					arrPos = i;
				}
			}
			
			/*Im[x+y*myIm_Width].r = Colors[arrPos][1];
			Im[x+y*myIm_Width].g = Colors[arrPos][2];
			Im[x+y*myIm_Width].b = Colors[arrPos][3];*/
			//printf("%d",arrPos);
			global.temp[x+y*myIm_Width].r = localColor[arrPos][0];
			global.temp[x+y*myIm_Width].g = localColor[arrPos][1];
			global.temp[x+y*myIm_Width].b = localColor[arrPos][2];
			distance = 500;
			arrPos = 0;
		}
		//printf("%d",distance);
}


void effect2(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	//int avg[9] = {1,1,1,1,1,1,1,1,1};
	int avg[3][3] = {{1,1,1},{1,1,1},{1,1,1}};
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			if ((x > 0 && y > 0) && (x < myIm_Width && y < myIm_Height)){				
				int z;
				int q;
				int tempRed = 0;
				int tempBlue = 0;
				int tempGreen = 0;
				for (z=(x-1);z<(x+1);z++){
					for (q=(y-1);q<(y+1);q++){
						//Im[z+q*myIm_Width].r;
						tempRed += Im[z+q*myIm_Width].r*avg[z][q];
						tempBlue += Im[z+q*myIm_Width].b*avg[z][q];
						tempGreen += Im[z+q*myIm_Width].g*avg[z][q];
					}
				}
				//printf("%d",tempRed);
				tempRed = tempRed/9;
				tempBlue = tempBlue/9;
				tempGreen = tempGreen/9;
				//printf("%d",tempRed);
				global.temp[x+y*myIm_Width].r = tempRed;
				global.temp[x+y*myIm_Width].g = tempGreen;
				global.temp[x+y*myIm_Width].b = tempBlue;
			}//ignore edge
		}		
	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn	
}


void swap(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;
	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){			
			Im[x+y*myIm_Width].r = global.temp[x+y*myIm_Width].r;
			Im[x+y*myIm_Width].b = global.temp[x+y*myIm_Width].b;
			Im[x+y*myIm_Width].g = global.temp[x+y*myIm_Width].g;
		}

	glutPostRedisplay();
}


void resetImage(pixel* Im, int myIm_Width, int myIm_Height){
	//glDrawPixels(global.w, global.h, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)global.data);		
	
	//glClear(GL_COLOR_BUFFER_BIT);
	//glClearColor (0.0, 1.0, 0.0, 1.0);
	//glutDisplayFunc(display_image);
	//glFlush();	
}

/*glut keyboard function*/
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 0x1B:
	case'q':
	case 'Q':
		exit(0);
		break;
	case's':
	case'S':
		printf("SAVING IMAGE: backup.tif\n");
		write_img("backup.tif", global.data, global.w, global.h);
		break;
	case 't':
	case 'T':
		draw_triangle();
		break;
	case'f':
	case'F':
		MyFilter(global.data,global.w,global.h);
		break;
	case'r':
	case'R':
		//MyFilter(global.data,global.w,global.h);
		//resetImage();
		reset(global.data,global.w,global.h);
		break;
	case'g':
	case'G':
		greyScale(global.data,global.w,global.h);		
		break;
	case'm':
	case'M':
		monochrome(global.data,global.w,global.h);		
		break;
	case'n':
	case'N':
		NTSC(global.data,global.w,global.h);		
		break;
	case'i':
	case'I':
		invert(global.data,global.w,global.h);		
		break;
	case'p':
	case'P':
		Green(global.data,global.w,global.h);		
		break;
	case'o':
	case'O':
		Red(global.data,global.w,global.h);		
		break;
	case'l':
	case'L':
		Blue(global.data,global.w,global.h);		
		break;
	case'e':
	case'E':
		effect1(global.data,global.w,global.h);		
		break;
	case'w':
	case'W':
		effect2(global.data,global.w,global.h);	
		swap(global.data,global.w,global.h);
		break;
	case'z':
	case'Z':
		Max(global.data,global.w,global.h);		
		swap(global.data,global.w,global.h);
		break;
	case'x':
	case'X':
		Min(global.data,global.w,global.h);		
		swap(global.data,global.w,global.h);
		break;
	case'a':
	case'A':
		redIntensify(global.data,global.w,global.h);	
		break;
	case'y':
	case'Y':
		blueIntensify(global.data,global.w,global.h);	
		break;
	case'u':
	case'U':
		greenIntensify(global.data,global.w,global.h);	
		break;
	case'k':
	case'K':
		Quantize(global.data,global.w,global.h,Colors);
		swap(global.data,global.w,global.h);
		break;
	case'b':
	case'B':
		FinalSobal(global.data,global.w,global.h, sobelHorizontal, sobelVerticle);
		swap(global.data,global.w,global.h);
		break;
	case'h':
	case'H':
		greenIntensify(global.data,global.w,global.h);		
		swap(global.data,global.w,global.h);
		break;
	case'v':
	case'V':
		redIntensify(global.data,global.w,global.h);		
		swap(global.data,global.w,global.h);
		break;
	/*case'.':
	case'>':
		redIntensify(global.data,global.w,global.h);		
		swap(global.data,global.w,global.h);
		break;*/
		//MaskFilter(global.data,global.w,global.h, avg);
		//SobelH(global.data,global.w,global.h, sobelVerticle);
		//FinalSobal(global.data,global.w,global.h, sobelHorizontal, sobelVerticle);
		//swap(global.data,global.w,global.h);
	}
}//keyboard




int main(int argc, char** argv)
{
	global.data = read_img("img.tif", &global.w, &global.h);
	

	//global.buffer = (pixel *)malloc((global.h)*(global.w)*sizeof(pixel *));
	global.buffer = read_img("img.tif", &global.w, &global.h);
	global.temp = read_img("img.tif", &global.w, &global.h);
	//global.buffer = global.data;
	if (global.data==NULL)
	{
		printf("Error loading image file img.tif\n");
		return 1;
	}
	printf("Q:quit\nF:filter\nT:triangle\nS:save\nR:reset\nG:greyscale\nM:monochrome\nN:NTSC\nI:Invert\n");
	printf("P:Green\nO:Red\nL:Blue\nE:Effect\n");
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGB|GLUT_SINGLE);
	
	glutInitWindowSize(global.w,global.h);
	glutCreateWindow("SIMPLE DISPLAY");
	glShadeModel(GL_SMOOTH);

	glutDisplayFunc(display_image);

	glutKeyboardFunc(keyboard);
	glMatrixMode(GL_PROJECTION);
	glOrtho(0,global.w,0,global.h,0,1);
	
	glutMainLoop();

	return 0;
}