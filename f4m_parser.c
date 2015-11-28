#define _GNU_SOURCE

#include "f4m_parser.h"

/* extract bitrate value from
 * <media url="/myvideo/high" bitrate="1708" width="1920" height="1080"/> 
 * return malloc-ed bitrate list
 */

unsigned* extract_bitrate_list(char* buf, int size){
	// buf[size] = '\0';
	// printf("In extract_bitrate_list....................................\n");
	// printf("%s\n", buf);
	char* new_buf = strstr(buf, "<?xml");
	// printf("%s\n", new_buf);
	xmlDocPtr doc = xmlReadMemory(new_buf, size, "noname.xml", NULL, 0);
	xmlNodePtr cur = xmlDocGetRootElement(doc);
	cur = cur->xmlChildrenNode;
	struct bitrate* bitrates = NULL;
	int bitrates_size = 0;
	while(cur){
		if(xmlStrcmp(cur->name, (const xmlChar*)"media")==0){
			xmlChar* bitrate = xmlGetProp(cur, (const xmlChar*)"bitrate");
			if(bitrate!=NULL){
				int bitrate_int = atoi((char*)bitrate);
				bitrates_size++;
				xmlFree(bitrate);
				if(bitrates==NULL){
					bitrates = (struct bitrate*)malloc(sizeof(struct bitrate));
					bitrates->bitrate = bitrate_int;
					bitrates->next = NULL;
				}
				else{
					struct bitrate* temp = (struct bitrate*)malloc(sizeof(struct bitrate));
					temp->bitrate = bitrate_int;
					temp->next = bitrates;
					bitrates = temp;
				}
			}
		}
		cur = cur->next;
	}
	unsigned* bitrates_int = (unsigned*)malloc(sizeof(unsigned)*(bitrates_size+1));
	int i=0;
	if(bitrates_size==1){
		bitrates_int[i] = bitrates->bitrate;
		i++;
	}
	else{
		bitrates_int[0] = find_min_bitrate_above_t(bitrates, -1);
		i++;
		while(i<bitrates_size){
			bitrates_int[i] = find_min_bitrate_above_t(bitrates, bitrates_int[i-1]);
			i++;
		}
	}
	bitrates_int[i] = 0;
	return bitrates_int;
}

int find_min_bitrate_above_t(struct bitrate* bitrates, int t){
	int min_bitrate = INT_MAX;
	struct bitrate* temp = bitrates;
	while(temp){
		if(temp->bitrate<min_bitrate&&temp->bitrate>t){
			min_bitrate = temp->bitrate;
		}
		temp = temp->next;
	}
	return min_bitrate;
}

// unsigned* extract_bitrate_list(char* buf, int size){
// 	size = size;
// 	buf = buf;
// 	unsigned* bitrates_int = (unsigned*)malloc(sizeof(unsigned)* 4);
// 	bitrates_int[0] = 100;
// 	bitrates_int[1] = 500;
// 	bitrates_int[2] = 1000;
// 	bitrates_int[3] = 0;
// 	return bitrates_int;
// }


// unsigned * extract_bitrate_list(char * buf, int size) {
	
// 	buf[size] = '\0';
// 	int index = 0;
// 	while(strcasestr(buf + index, "<meida") != NULL) {
	
// 	}
// 	return bitrates;
// 	// while(strcasestr()) {
	
// 	// }
// }

// int main(int argc, char* argv[]){
// 	if(argc<2){
// 		printf("Usage: ./f4m_parser /path/to/f4m_file\n");
// 		return -1;
// 	}
// 	char* filename = argv[1];
// 	FILE* fd = fopen(filename, "r");
// 	char buffer[MAX_BUFFER];
// 	size_t newLen = fread(buffer, sizeof(char), MAX_BUFFER, fd);
//     if(newLen == 0){
//         fputs("Error reading file", stderr);
//     }else{
//         buffer[newLen++] = '\0';
//     }
// 	unsigned* bitrates = extract_bitrate_list(buffer, newLen);
// 	int i=0;
// 	while(1){
// 		printf("%d\n", bitrates[i]);
// 		if(bitrates[i]==0){
// 			break;
// 		}
// 		i++;
// 	}
// 	return -1;
// }
