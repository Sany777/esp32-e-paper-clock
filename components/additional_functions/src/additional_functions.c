#include "additional_functions.h"


#include "stdlib.h"




unsigned get_num(char *data, const unsigned size)
{
	unsigned res = 0;
	const char *end = data+size;
	char c;
	while(data != end){
		c = *(data++);
		if(c<'0' || c>'9') break;
		res *= 10;
		res += c-'0';
	}
	return res;
}

char * num_to_str(char *buf, unsigned num, unsigned char digits, const unsigned char base)
{
    const char hex_chars[] = "0123456789ABCDEF";
    unsigned char d = digits;
    char tmp,
        * end = buf,
        * ptr = buf;
        
    do{
        *end = hex_chars[num % base];
        if(d < 2) break;
        d -= 1;
        num /= base;
        end += 1;
    }while(1);

    while (ptr < end) {
        tmp = *ptr;
        *ptr = *end;
        *end = tmp;
        ptr += 1;
        end -= 1;
    }
	buf[digits] = 0;
    return &buf[digits];
}


unsigned num_arr_to_str(char *dst, unsigned *src, unsigned char dst_digits, unsigned src_size)
{
    if(src == NULL || src_size == 0)return 0;
    unsigned *src_end = src+src_size;
    char *start_dst = dst;
    while(src < src_end){
        dst = num_to_str(dst, *src, dst_digits, 10);
        ++src;
    }
    return dst - start_dst;
}





