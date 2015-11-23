#ifndef _NAME_UTIL_H_
#define _NAME_UTIL_H_

/* input: chunk_name "/path/to/video/xxxSegx-Fragx"
 * output video_name "video"
 * return val: 0 on failure, 1 on success
 */
int get_videoname_from_chunkname(const char * chunk_name,
	char * video_name);

#endif