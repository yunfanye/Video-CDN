#include "name_util.h"

/* input: chunk_name "/path/to/video/xxxSegx-Fragx"
 * output video_name "video"
 * return val: 0 on failure, 1 on success
 */
int get_videoname_from_chunkname(const char* chunk_name, char* video_name){
	char buffer[MAX_BUFFER];
	strcpy(buffer, chunk_name);
	char* pos = strtok(buffer, "/");
	if(pos==NULL){
		return 0;
	}
	char* pos_next = strtok(NULL, "/");
	if(pos_next==NULL){
		return 0;
	}
	char* temp = pos_next;
	while(temp!= NULL){
		temp = strtok(NULL, "/"); 
		if(temp!=NULL){
			pos = pos_next;
			pos_next = temp;
		}
		// printf("%s, %s\n", pos, pos_next);
	}
	strcpy(video_name, pos);
	return 1;
}

// int main(){
// 	char video_name[1000];
// 	get_videoname_from_chunkname("/path/to/video/xxxSegx-Fragx", video_name);
// 	printf("%s\n", video_name);
// 	get_videoname_from_chunkname("video/xxxSegx-Fragx", video_name);
// 	printf("%s\n", video_name);
// 	get_videoname_from_chunkname("/path/to/asd/asddd/video-name/xxxSegx-Fragx", video_name);
// 	printf("%s\n", video_name);
// 	if(get_videoname_from_chunkname("/xxxSegx-Fragx", video_name)>0){
// 		printf("%s\n", video_name);
// 	}
// 	if(get_videoname_from_chunkname("xxxSegx-Fragx", video_name)>0){
// 		printf("%s\n", video_name);
// 	}
// }