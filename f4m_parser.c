#define _GNU_SOURCE

#include "f4m_parser.h"

/* extract bitrate value from
 * <media url="/myvideo/high" bitrate="1708" width="1920" height="1080"/> 
 * return malloc-ed bitrate list
 */
unsigned* extract_bitrate_list(char* buf, int size){
	// buf[size] = '\0';
	xmlDocPtr doc = xmlReadMemory(buf, size, "noname.xml", NULL, 0);
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
					bitrates->next = bitrates;
					bitrates = temp;
				}
			}
			cur = cur->next;
		}
	}
	unsigned* bitrates_int = (unsigned*)malloc(sizeof(unsigned)*size);
	struct bitrate* temp = bitrates;
	int i=0;
	while(temp){
		bitrates_int[i] = temp->bitrate;
		i++;
		temp = temp->next;
	}
	return bitrates_int;
}
// unsigned * extract_bitrate_list(char * buf, int size) {
	
// 	buf[size] = '\0';
// 	int index = 0;
// 	while(strcasestr(buf + index, "<meida") != NULL) {
	
// 	}
// 	return bitrates;
// 	// while(strcasestr()) {
	
// 	// }
// }

