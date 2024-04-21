#include<stdint.h>
#include<stdio.h>

#define NAME "cksum (canoutils)"
#define VERSION "0.0.1"
#define AUTHOR "cospplredman"

#include"../version_info.h"

#define X(a,...) for(a)for(r=(r<<8)^(e&0xff)^__VA_ARGS__,e=0,i=31;i>23;i--)if(r&(1<<i))r^=(1<<i)|(q>>(32-i)),e^=q<<(i-24);

uint32_t q=0x04c11db7,r,e;d,i=1,t,j,k,f=2;main(c,v)char**v;{
	if(c<2)exit(1);
	for(;++k<c;){
		if(f&&(!strcmp(v[k], "--")|!strcmp(v[k], "-v"))){
			if(v[k][1]=='-')f=0;
			if(f==2){f=1;print_version();}
			continue;
		}
		FILE*f=fopen(v[k],"r");
		if(f==NULL)exit(1);
		X(;(d=getc(f))>=0;t++,d)X(j=t;j;,(j&0xff),j>>=8)X(j=3;j--;,0)
		if(printf("%u %d %s\n",~((r<<8)|(e&0xff)),t,v[k])<0)exit(1);
		r=e=t=0;
	}
}
