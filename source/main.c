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

void encode_hex(const unsigned char data, char* output)
{
	char* mapping = "0123456789abcdef";
	unsigned char d = data;
	unsigned char intern[2];
	for (int i = 1; i >= 0; i--) {
		intern[i] = (unsigned char)(d % 16);
		output[i] = mapping[d % 16];
		d = (unsigned char)((d - intern[i])/16);
	}
}

typedef enum {
	PURE,
	BOOL,
	BIN,
	ENC_TYPE,
	APNUM
} field_type_t;

void print_field(char* title, int decode, field_type_t field_type)
{
	char field[0xae];
	readfield(field, 0xac, decode);
	if (field_type == BOOL) {
		if (field[0] == '0')
			strcpy(field, "False");
		else
			strcpy(field, "True");
		printf("    %s : %s\n", title, field);
	} else if (field_type == ENC_TYPE) {
		if (field[0] == '0')
			strcpy(field, "Open");
		else if (field[0] == '1')
			strcpy(field, "WEP-64");
		else if (field[0] == '2')
			strcpy(field, "WEP-128");
		else if (field[0] == '3')
			strcpy(field, "WEP-152");
		else if (field[0] == '4')
			strcpy(field, "WPA-PSK (TKIP)");
		else if (field[0] == '5')
			strcpy(field, "WPA2-PSK (TKIP)");
		else if (field[0] == '6')
			strcpy(field, "WPA-PSK (AES)");
		else if (field[0] == '7')
			strcpy(field, "WPA2-PSK (AES)");
		else
			strcpy(field, "Unknown");
		printf("    %s : %s\n", title, field);
	} else if (field_type == APNUM) {
		printf("    %s : %s\n", title, field);
		char* region;
		switch (field[0]) {
			case '0':
				region = "JPN";
				break;
			case '1':
				region = "USA";
				break;
			case '2':
			case '3':
				region = "EUR";
				break;
			case '4':
				region = "KOR";
				break;
			case '5':
				region = "CHN";
				break;
			default:
				region = "UNK";
		}
		printf("      %s : %s\n", "Region", region);

		char svcid[3];
		memcpy(svcid, ((char*)field) + 1, 2);
		svcid[2] = 0x00;
		printf("      %s : %s\n", "Service ID", svcid);

		printf("      %s : %s\n", "Unique ID", ((char*)field)+3);
	} else if (field_type == BIN) {
		char blank[33];
		bzero(blank, 33);
		if (memcmp(field, blank, 32) == 0) {
			printf("    %s : %s\n", title, "None");
			return;
		}
		char hexfield[65];
		bzero(hexfield, 65);
		char* outputptr = (char*)hexfield;
		for (int i = 0; i < 32; ++i) {
			encode_hex((unsigned char)field[i], outputptr);
			outputptr += 2;
		}
		printf("    %s : %s\n", title, hexfield);
	} else {
		memmove(((char*)field)+1, field, 0xac);
		field[0] = '\'';
		size_t l = strlen(field);
		field[l] = '\'';
		field[l+1] = 0x00;
		printf("    %s : %s\n", title, field);
	}
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
			print_field("Service Name", 1, PURE);
			print_field("URL", 1, PURE);
			print_field("SSID", 1, PURE);
			print_field("Key", 1, BIN);
			print_field("Encryption type", 0, ENC_TYPE);
			print_field("NZone Beacon ApNum", 0, APNUM);
			print_field("IsVendorIE", 0, BOOL);
			print_field("IsBackground", 0, BOOL);
			print_field("IsBrowser", 0, BOOL);
			print_field("IsShop", 0, BOOL);
			print_field("IsGame", 0, BOOL);
			print_field("IsSetToFW", 0, BOOL);
			print_field("IsZone", 0, BOOL);
			printf("\n");
			hotspot_index++;
		}

		linenum++;
	}

	fclose(fconf);

	return 0;
}

