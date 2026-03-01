#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	READ_INT32BE(ptr,idx)	(\
	(((unsigned int)((ptr)[((idx)+0)])&0xff)<<24)	|\
	(((unsigned int)((ptr)[((idx)+1)])&0xff)<<16)	|\
	(((unsigned int)((ptr)[((idx)+2)])&0xff)<< 8)	|\
	(((unsigned int)((ptr)[((idx)+3)])&0xff)<< 0)	|\
	0)
#define	READ_INT32LE(ptr,idx)	(\
	(((unsigned int)((ptr)[((idx)+3)])&0xff)<<24)	|\
	(((unsigned int)((ptr)[((idx)+2)])&0xff)<<16)	|\
	(((unsigned int)((ptr)[((idx)+1)])&0xff)<< 8)	|\
	(((unsigned int)((ptr)[((idx)+0)])&0xff)<< 0)	|\
	0)


#define	READ_INT16BE(ptr,idx)	(\
	(((unsigned int)((ptr)[((idx)+0)])&0xff)<< 8)	|\
	(((unsigned int)((ptr)[((idx)+1)])&0xff)<< 0)	|\
	0)


#define WRITE_INT32BE(buf,idx,val)      \
        (buf)[(idx+0)]=((unsigned int)(val)>>24)&0xff;  \
        (buf)[(idx+1)]=((unsigned int)(val)>>16)&0xff;  \
        (buf)[(idx+2)]=((unsigned int)(val)>> 8)&0xff;  \
        (buf)[(idx+3)]=((unsigned int)(val)>> 0)&0xff;

unsigned char data[(1<<20)];
unsigned char data2[1<<20];


int main(int argc,char** argv)
{
	FILE *f;
	FILE *g;
	int code_size_old,code_size_new;
	int string1_size;
	int string2_size;
	int dict_size;
	int idx_old,idx_new;

	int i;
	int n;
	memset(data2,0,sizeof(data2));

	if (argc!=3)
	{
		fprintf(stderr,"please run with %s INPUT.mag OUTPUT.mag\n",argv[0]);
		return 1;
	}

	f=fopen(argv[1],"rb");
	if (!f) 
	{
		fprintf(stderr,"unable to open %s.\n",argv[1]);
		return 1;
	}
	n=fread(data,sizeof(char),sizeof(data),f);
	fclose(f);


	printf("%d bytes read\n",n);

	
	code_size_old=READ_INT32BE(data,14);	printf("size of code:    %6d\n",code_size_old);
	string1_size=READ_INT32BE(data,18);	printf("size of string1: %6d\n",string1_size);
	string2_size=READ_INT32BE(data,22);	printf("size of string2: %6d\n",string2_size);
	dict_size=READ_INT32BE(data,26);	printf("size of dict:    %6d\n",dict_size);
	printf("huffmantree @%08x\n",READ_INT32BE(data,30));
	code_size_new=98304;

	idx_old=0;
	idx_new=0;
	// header
	// 00.3 masc
	// 4..7 size of all the sections(*)
	// 8..11 size of the header
	// 12: ?
	// 13: version

	// 14..17: game code size
	// 18..21: string1 size
	// 22..25: string2 size
	// 26..29: dict size
	// 30..33: pointer to the beginning of the huffman tree
	// 34..37: size of the undo section
	// 38..41: undo pc

	for (i=0;i<42;i++)
	{
		data2[idx_new]=data[idx_old];
		idx_old++;
		idx_new++;
	}
	
	WRITE_INT32BE(data2,14,code_size_new);
	
	memcpy(&data2[idx_new],&data[idx_old],code_size_old);
	idx_old+=code_size_old;
	idx_new+=code_size_new;

	memcpy(&data2[idx_new],&data[idx_old],string1_size);
	idx_old+=string1_size;
	idx_new+=string1_size;

	memcpy(&data2[idx_new],&data[idx_old],string2_size);
	idx_old+=string2_size;
	idx_new+=string2_size;

	memcpy(&data2[idx_new],&data[idx_old],dict_size);
	idx_old+=dict_size;
	idx_new+=dict_size;

	WRITE_INT32BE(data2,4,idx_new);

	g=fopen(argv[2],"wb");
	fwrite(&data2,sizeof(char),idx_new,g);
	fclose(g);	


	return 0;	
	
}

