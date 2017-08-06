#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "b64/cdecode.h"

FILE *fconf;
int field_linepos;
char line[1024];

int readline()
{
	int len = 0;
	int c;

	memset(line, 0, 1024);
	field_linepos = 0;

	while(1)
	{
		c = getc(fconf);
		if(c==EOF)return 1;
		if(c==0x0a)break;
		if(len>=1024)
		{
			printf("Line is too long, aborting...\n");
			return 2;
		}
		line[len] = (char)c;
		len++;
	}

	return 0;
}

int readfield(char *field, unsigned int maxlen, int decode)
{
	unsigned int fieldpos=0;
	base64_decodestate state;
	char tmpfield[0xac];

	memset(tmpfield, 0, 0xac);
	memset(field, 0, maxlen);

	while(line[field_linepos]!=',' && line[field_linepos]!=0x00)
	{
		if(fieldpos>=maxlen)
		{
			printf("Field is too long, aborting...\n");
			exit(1);
		}

		tmpfield[fieldpos] = line[field_linepos];
		field_linepos++;
		fieldpos++;
	}

	if(line[field_linepos]==',')
	{
		field_linepos++;
	}
	else if(field_linepos==0)
	{
		return 1;
	}

	if(decode)
	{
		base64_init_decodestate(&state);
		base64_decode_block(tmpfield, maxlen, field, &state);
	}
	else
	{
		strncpy(field, tmpfield, fieldpos);
	}

	return 0;
}

void print_field(char* title, int decode)
{
	char field[0xac];
	readfield(field, 0xac, decode);
	printf("    %s : '%s'\n", title, field);
}

int main(int argc, char **argv)
{
	int linenum = 0;
	int hotspot_index=0;

	if(argc<2)
	{
		printf("hotspotconf-tool by yellows8\n");
		printf("Parse the Nintendo 3DS NZone hotspot.conf file.\n");
		printf("Usage:\nhotspotconf-tool <hotspot.conf>\n");
		return 0;
	}

	fconf = fopen(argv[1], "r");
	if(fconf==NULL)
	{
		printf("Failed to open file %s, aborting...\n", argv[1]);
		return 1;
	}

	field_linepos = 0;

	while(readline()==0)
	{
		if(linenum<2)
		{
			if(linenum==0 && strcmp(line, "Interval"))
			{
				printf("Interval record is missing, aborting...\n");
				fclose(fconf);
				return 1;
			}
			if(linenum==1)printf("Interval: %s\n", line);
		}

		if(linenum==2)
		{
			if(strcmp(line, "ServiceName,Url,Ssid,SecurityKey,SecurityMode,ApNum,IsBackground,IsBrowser,IsShop,IsGame,IsSetToFW,IsVendorIE,IsZone"))
			{
				printf("Invalid records, aborting...\n");
				fclose(fconf);
				return 1;
			}
		}

		if(linenum>2)
		{
			printf("Hotspot %d\n", hotspot_index);
			print_field("Service Name", 1);
			print_field("URL", 1);
			print_field("SSID", 1);
			print_field("Key", 1);
			print_field("Encryption type", 0);
			print_field("NZone Beacon ApNum", 0);
			print_field("IsVendorIE", 0);
			print_field("IsBackground", 0);
			print_field("IsBrowser", 0);
			print_field("IsShop", 0);
			print_field("IsGame", 0);
			print_field("IsSetToFW", 0);
			print_field("IsZone", 0);
			printf("\n");
			hotspot_index++;
		}

		linenum++;
	}

	fclose(fconf);

	return 0;
}

