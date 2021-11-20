#include <linux/fb.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

 

void print_help(){
	printf("ili9488 TFT frame buffer tester\r\n\n");
	printf("Arguments:\r\n"
		   "Test type [-r,-b,-d]:\r\n" 
		   "   -r: default - reactangle areas test with RAM buffer (full screen write to FB)\r\n"
		   "   -d: reactangle areas test without RAM buffer (partial screen write to FB)\r\n"
		   "   -b: moving bars test without RAM buffer, full screen write to FB\r\n"
		   "Frame buffer number -f=[n]\r\n"
		   "   n: fbnumber default - 1 (\"/dev/fb1\")\r\n\n"
		);

	printf("for use ili9488 as frame buffer run:\r\n\n");
	printf("sudo modprobe fbtft_device name=flexfb gpios=reset:203,dc:1,led:0,cs:67 speed=40000000 &&\\\r\n"
		   "sudo modprobe flexfb width=480 height=480 buswidth=8 init=\\\r\n"
			"-1,0x01,\\\r\n"
			"-2,120,\\\r\n"
			"-1,0x36,0xE8,\\\r\n"
			"-1,0x3A,0x66,\\\r\n"
			"-1,0x21,\\\r\n"
			"-1,0x11,\\\r\n"
			"-2,120,\\\r\n"
			"-1,0x29,\\\r\n"
			"-2,20,\\\r\n"
			"-1,0x13,\\\r\n"
			"-3\r\n"
		);

}


int main(int argc, char *argv[]) {

	void*   m_FrameBuffer;
	struct  fb_fix_screeninfo m_FixInfo;
	struct  fb_var_screeninfo m_VarInfo;
	int m_FBFD;

	char test_type ='r';
	char fb_name[40] ="/dev/fb1";

	if(argc>=2){
		for(int i=1;i<argc;i++){
			if(argv[i][0]!='-'){
				printf("Wrong argument format: %s\r\n\n",(char*)argv[i]);
				print_help();
				return 1;
			}
			switch((char)argv[1][1]){
				case 'r':
					test_type = 'r';
					break;
				case 'd':
					test_type = 'd';
					break;
				case 'b':
					test_type = 'b';
					break;
				case 'f':
					if(strncmp((char*)argv[i],"-f=",3)){
						printf("Wrong FB format: %s\r\n\n",(char*)argv[i]);
						print_help();
						return 1;
					}
					snprintf(fb_name,40,"/dev/fb%s",&argv[i][3]);
					break;
				default:
					printf("Unknown argument: %s\r\n\n",(char*)argv[i]);
					print_help();
					return 1;
			}
		}	
	}


	int iFrameBufferSize;
	/* Open the framebuffer device in read write */
	m_FBFD = open(fb_name, O_RDWR);
	if (m_FBFD < 0) {
		printf("Unable to open %s.\n", fb_name);
		return 1;
	}
	/* Do Ioctl. Retrieve fixed screen info. */
	if (ioctl(m_FBFD, FBIOGET_FSCREENINFO, &m_FixInfo) < 0) {
		printf("get fixed screen info failed: %s\n",
			  strerror(errno));
		close(m_FBFD);
		return 1;
	}
	/* Do Ioctl. Get the variable screen info. */
	if (ioctl(m_FBFD, FBIOGET_VSCREENINFO, &m_VarInfo) < 0) {
		printf("Unable to retrieve variable screen info: %s\n",
			  strerror(errno));
		close(m_FBFD);
		return 1;
	}

	/* Calculate the size to mmap */
	iFrameBufferSize = m_FixInfo.line_length * m_VarInfo.yres;
	printf("Line length %d\n", m_FixInfo.line_length);
	/* Now mmap the framebuffer. */
	m_FrameBuffer = mmap(NULL, iFrameBufferSize, PROT_READ | PROT_WRITE,
		 MAP_SHARED, m_FBFD,0);
	if (m_FrameBuffer == NULL) {
		printf("mmap failed:\n");
		close(m_FBFD);
		return 1;
	}

	switch(test_type){
		case 'r':
			printf("Run test: reactangle areas test with RAM buffer (full screen write to FB)\r\n");
			break;
		case 'd':
			printf("Run test: reactangle areas test without RAM buffer (partial screen write to FB)\r\n");
			break;
		case 'b':
			printf("Run test: moving bars test without RAM buffer, full screen write to FB\r\n");
	}

	uint8_t *p = (uint8_t*)m_FrameBuffer;
	if(test_type=='r'){
		p = (uint8_t *)malloc(iFrameBufferSize);
	}
	
	uint32_t xres=480;
	uint32_t yres=320;
	uint8_t bytepp=3; 
	int x,y;

	uint16_t xbarbeg=0;
	uint16_t xbarend=10;
	uint16_t ybarbeg=0;
	uint16_t ybarend=10;

	#define DRAW_RECT(bx,by,ex,ey,r,g,b)           			\
	for (y=by; y<ey; y++) {	                       			\
		for (x=bx; x<ex; x++) {                    		\
				uint32_t offset = (x + y*xres)*bytepp;	\
				p[offset++] = r;                      	\
				p[offset++] = g;                      	\
				p[offset] =   b;                      	\
		}                                          		\
	}

	#define DRAW_TEST_STEP(q,st,r,g,b)						\
	do{										\
		uint16_t xst = ((q==0 || q==3) ? 240 : 0) + (30*(st&0x03));	    	\
		uint16_t yst = ((q==2 || q==3) ? 160 : 0) + (30*(st&0x03));	    	\
		uint16_t xend =((q==0 || q==3) ? 480 : 240) - (30*(st&0x03));		\
		uint16_t yend =((q==2 || q==3) ? 320 : 160) - (30*(st&0x03));		\
	    DRAW_RECT(xst,yst,xend,yend,r,g,b);						\
	}while(0);


	time_t start = time(NULL);
	uint8_t disp=0;
	while(1) { 
		if(test_type=='b'){
			for (y=0; y<yres; y++) {
				for (x=0; x<xres; x++) {
				uint32_t offset = (x + y*xres)*bytepp;
						p[offset++] = x<xbarbeg ? 0x00 : (x>xbarend ? 0x00 : 0xFF);   //Red
				p[offset++] = 0xFF;   //Green
				p[offset] =   y<ybarbeg ? 0x00 : (y>ybarend ? 0x00 : 0xFF);   //Blue
				}
			}
		xbarbeg=(xbarbeg+10)%xres;
		xbarend=(xbarend+10)%xres;
		ybarbeg=(ybarbeg+10)%yres;
		ybarend=(ybarend+10)%yres;
		}else{
			switch (disp&0x03){
				case 0:
					DRAW_TEST_STEP(0,0,0xFF,0x00,0x00);
					DRAW_TEST_STEP(1,3,0x00,0xFF,0x00);
					DRAW_TEST_STEP(2,1,0x00,0xFF,0x00);
					DRAW_TEST_STEP(3,2,0xFF,0x00,0x00);
					break;
				case 1:
					DRAW_TEST_STEP(0,1,0x00,0xFF,0x00);
					DRAW_TEST_STEP(1,0,0x00,0xFF,0x00);
					DRAW_TEST_STEP(2,2,0x00,0x00,0xFF);
					DRAW_TEST_STEP(3,3,0x00,0x00,0xFF);
					break;
				case 3:
					DRAW_TEST_STEP(0,2,0x00,0x00,0xFF);
					DRAW_TEST_STEP(1,1,0x00,0x00,0xFF);
					DRAW_TEST_STEP(2,3,0xFF,0x00,0x00);
					DRAW_TEST_STEP(3,0,0x00,0x00,0xFF);
					break;
				default:
					DRAW_TEST_STEP(0,3,0xFF,0x00,0x00);
					DRAW_TEST_STEP(1,2,0xFF,0x00,0x00);
					DRAW_TEST_STEP(2,0,0xFF,0x00,0x00);
					DRAW_TEST_STEP(3,1,0xFF,0x00,0x00);
					break;  
			}
			usleep(400000);//400ms
		}

		if(test_type=='r'){
			memcpy(m_FrameBuffer,p,iFrameBufferSize);
		}

		if( ((disp++)&0x0f)==0){
			uint32_t run_time = (uint32_t)(time(NULL)-start);
			printf("\r%d:%02d",run_time/60,run_time%60);
			fflush(stdout);
		}
		usleep(100000);//100ms
	}

}
