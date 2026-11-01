#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <linux/types.h>
#include "dns_printer.h"

#define T_A 1 			/*Ipv4 address*/
#define T_NS 2 			/*Nameserver*/
#define T_CNAME 5 		/*canonical name*/
#define T_SOA 6 		/*start of authority zone */
#define T_PTR 12 		/*domain name pointer */
#define T_MX 15 		/*Mail server*/

/*DNS header*/
struct DNS_HEADER
{
	unsigned short id; 	// identification number

# if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned char rd :1; 	// recursion desired
	unsigned char tc :1; 	// truncated message
	unsigned char aa :1; 	// authoritive answer
	unsigned char opcode :4; // purpose of message
	unsigned char qr :1; 	// query/response flag

	unsigned char rcode :4; // response code
	unsigned char cd :1; 	// checking disabled
	unsigned char ad :1; 	// authenticated data
	unsigned char z :1; 	// reserved and unused
	unsigned char ra :1; 	// recursion available
# endif
# if __BYTE_ORDER == __BIG_ENDIAN
	unsigned char qr :1;
	unsigned char opcode :4;
	unsigned char aa :1;
	unsigned char tc :1;
	unsigned char rd :1;

	unsigned char ra :1;
	unsigned char z :1;
	unsigned char ad :1;
	unsigned char cd :1;
	unsigned char rcode :4;
# endif

	unsigned short q_count; // number of question entries
	unsigned short ans_count; // number of answer entries
	unsigned short auth_count; // number of authority entries
	unsigned short add_count; // number of resource entries
};

struct QUESTION				/*QUESTION DATA*/
{
	unsigned short qtype;		/*query type:IN,NS,CNAME,SOA,PTR,MX*/
	unsigned short qclass;		/*query class:IN or CHAOS*/
};


#pragma pack(push, 1)
struct  R_DATA				/*RESOURCE RECORD DATA*/
{
	unsigned short type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short data_len;
};
#pragma pack(pop) 

struct RES_RECORD			/*RESOURCE RECORD FIELD:AUTHORITATIVE,ANSWER or ADDITIONAL*/
{
	unsigned char *name;
	struct R_DATA *resource;
	unsigned char *rdata;
};

unsigned char* ReadName(unsigned char* reader,unsigned char* buffer,int* count)
{
	unsigned char *name;
	unsigned int p=0,jumped=0,offset;
	int i , j;

	*count = 1;
	name = (unsigned char*)malloc(256);		/*maximum allowed length is 256*/

	name[0]='\0';

	while(*reader!=0)
	{
		if(*reader>=192)
		{
			offset = (*reader)*256 + *(reader+1) - 49152;
			reader = buffer + offset - 1;
			jumped = 1; 
		}
		else
			name[p++]=*reader;
		reader = reader+1;
		if(jumped==0)
			*count = *count + 1;
	}

	name[p]='\0';
	if(jumped==1)
		*count = *count + 1;

	for(i=0;i<(int)strlen((const char*)name);i++) 
	{
		p=name[i];
		for(j=0;j<(int)p;j++) 
		{
			name[i]=name[i+1];
			i=i+1;
		}
		name[i]='.';
	}
	name[i-1]='\0';
	return name;
}

void print_dns_resopens(unsigned char* data)
{
    int i , j , stop , s;
    unsigned char *qname,*reader;
    struct sockaddr_in a,dest;
    struct RES_RECORD answers[50],auth[50],addinfo[50];
	struct DNS_HEADER* dns = (struct DNS_HEADER*) data;

    printf("qr:%d\n", (int)dns->qr);
	
	if(dns->ra==0)
	{
		//printf("Recursion not supported..quitting\n");
		//return;
	}
	
	if(dns->aa==0)
		printf("The server used is a non-authoritative server in the domain\n");
	else
		printf("The server used is an authoritative server in the domain\n");


	if(dns->rcode==0)
	{
        qname =(unsigned char*)&data[sizeof(struct DNS_HEADER)];	
		reader = &data[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION)];	/*THE RESPONSE*/

		printf("\nThe response contains : ");
		printf("\n %d Questions.",ntohs(dns->q_count));
		printf("\n %d Answers.",ntohs(dns->ans_count));
		printf("\n %d Authoritative Servers.",ntohs(dns->auth_count));
		printf("\n %d Additional records.\n\n",ntohs(dns->add_count));

		stop=0;

		for(i=0;i<ntohs(dns->ans_count);i++)
		{
			answers[i].name=ReadName(reader,data,&stop);	
			reader = reader + stop;

			answers[i].resource = (struct R_DATA*)(reader);
			reader = reader + sizeof(struct R_DATA);

			if(ntohs(answers[i].resource->type) == 1) 	/*read address*/
			{
				answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource->data_len));

				for(j=0 ; j<ntohs(answers[i].resource->data_len) ; j++)
					answers[i].rdata[j]=reader[j];

				answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';

				reader = reader + ntohs(answers[i].resource->data_len);
			}
			else						/*read name*/
			{
				answers[i].rdata = ReadName(reader,data,&stop);
				reader = reader + stop;
			}
		}

		//read authorities
		for(i=0;i<ntohs(dns->auth_count);i++)			
		{
			auth[i].name=ReadName(reader,data,&stop);
			reader+=stop;

			auth[i].resource=(struct R_DATA*)(reader);
			reader+=sizeof(struct R_DATA);

			if(ntohs(auth[i].resource->type)==1)		/*read address*/
                        {
                                auth[i].rdata = (unsigned char*)malloc(ntohs(auth[i].resource->data_len));
                                for(j=0;j<ntohs(auth[i].resource->data_len);j++)
                                        auth[i].rdata[j]=reader[j];

                                auth[i].rdata[ntohs(auth[i].resource->data_len)]='\0';
                                reader+=ntohs(auth[i].resource->data_len);
                        }
                        else						/*read name*/
                        {
                                auth[i].rdata=ReadName(reader,data,&stop);
                                reader+=stop;
                        }

		}

		//read additional
		for(i=0;i<ntohs(dns->add_count);i++)
		{
			addinfo[i].name=ReadName(reader,data,&stop);
			reader+=stop;

			addinfo[i].resource=(struct R_DATA*)(reader);
			reader+=sizeof(struct R_DATA);

			if(ntohs(addinfo[i].resource->type)==1)				/*read address*/
			{
				addinfo[i].rdata = (unsigned char*)malloc(ntohs(addinfo[i].resource->data_len));
				for(j=0;j<ntohs(addinfo[i].resource->data_len);j++)
					addinfo[i].rdata[j]=reader[j];

				addinfo[i].rdata[ntohs(addinfo[i].resource->data_len)]='\0';
				reader+=ntohs(addinfo[i].resource->data_len);
			}
			else								/*read name*/
			{
				addinfo[i].rdata=ReadName(reader,data,&stop);
				reader+=stop;
			}
		}

		//print answers
		printf("\nAnswer Records : %d \n" , ntohs(dns->ans_count) );
		for(i=0 ; i < ntohs(dns->ans_count) ; i++)
		{
			if(ntohs(answers[i].resource->type) == 12)
				printf("Address : %s ",answers[i].name);
			else
				printf("Name : %s ",answers[i].name);
			
			if( ntohs(answers[i].resource->type) == T_A) //IPv4 address
			{
				long *p;
				p=(long*)answers[i].rdata;
				a.sin_addr.s_addr=(*p); 
				printf("has IPv4 address : %s",inet_ntoa(a.sin_addr));
			}
			else if(ntohs(answers[i].resource->type)==5) 
				printf("has alias name : %s",answers[i].rdata);
			else if(ntohs(answers[i].resource->type)==12)
				printf("has domain name :%s",answers[i].rdata);
			printf("\n");
		}

		//print authorities
		printf("\nAuthoritive Records : %d \n" , ntohs(dns->auth_count) );
		for( i=0 ; i < ntohs(dns->auth_count) ; i++)
		{

			printf("Name : %s ",auth[i].name);
			if(ntohs(auth[i].resource->type)==2)
				printf("has nameserver : %s",auth[i].rdata);
			else if(ntohs(auth[i].resource->type)==6)
				printf("has start of authority : %s",auth[i].rdata);
			else if(ntohs(auth[i].resource->type)==12)
				printf("has domain name : %s",auth[i].rdata);
			printf("\n");
		}

		//print additional resource records
		printf("\nAdditional Records : %d \n" , ntohs(dns->add_count) );
		for(i=0; i < ntohs(dns->add_count) ; i++)
		{
			printf("Name : %s ",addinfo[i].name);
			if(ntohs(addinfo[i].resource->type)==1)
			{
				long *p;
				p=(long*)addinfo[i].rdata;
				a.sin_addr.s_addr=(*p);
				printf("has IPv4 address : %s",inet_ntoa(a.sin_addr));
			}
			printf("\n");
		}
	}
	else
	{
		if(dns->rcode==1)
			printf("The name server was unable to interpret the query\n");
		else if(dns->rcode==2)
			printf("The name server was unable to process this query due to a problem with the name server.\n");
		else if(dns->rcode==3)
			printf("domain name referenced in the query does not exist\n");
		else if(dns->rcode==4)
			printf("The name server does not support the requested kind of query.\n");
		else if(dns->rcode==5)
			printf("The server refused to answer\n");
		else if(dns->rcode==6)
			printf("A name exists when it should not\n");
		else if(dns->rcode==7)
			printf("A resource record set exists that should not\n");
		else if(dns->rcode==8)
			printf("A resource record set that should exist does not\n");
		else if(dns->rcode==9)
			printf("The name server receiving the query is not authoritative for the zone specified\n");
		else if(dns->rcode==10)
			printf("A name specified in the message is not within the zone specified in the message\n");
		else
			printf("Unknown error\n");
    }
}