/* unpack24.cpp
 * unpacks24 bit data back to original 32 bit pattern with channel id
 *
 * unpack24 NCHAN NSPAD
 * eg
 * ./unpack24 32 1 < DATA/shot_data | hexdump -e '33/4 "%08x," "\n"' | cut -d, -f 1-4,33 | more
 * nc UUT 4210 | unpack 32 1 > big-raw-file
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


namespace G {
	int nchan = 32;
	int nspad = 1;
};

unsigned ch_id(int ic)
{
	unsigned site = ic/32 + 1;
	return ic|site<<5;

}
int unpack24(FILE* fin, FILE* fout)
{
	int issb = G::nchan*3 + G::nspad*4;
	int ossw = G::nchan + G::nspad;
	unsigned char* ibuf = new unsigned char[issb];
	unsigned* obuf = new unsigned[ossw];

	while(fread(ibuf, 1, issb, fin) == issb){
		unsigned char* ip = ibuf;
		for (int ic = 0; ic < G::nchan; ++ic){
			unsigned char b1 = *ip++;
			unsigned char b2 = *ip++;
			unsigned char b3 = *ip++;

			obuf[ic] = b1<<8|b2<<16|b3<<24 | ch_id(ic);
		}
		for (int isp = 0; isp < G::nspad; ++isp, ip += sizeof(unsigned)){
			obuf[G::nchan+isp] = *(unsigned*)ip;
		}
		if (fwrite(obuf, sizeof(unsigned), ossw, fout) != ossw){
			return -1;
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	if (argc >= 2) G::nchan = atoi(argv[1]);
	if (argc >= 3) G::nspad = atoi(argv[2]);

	assert(G::nchan%32 == 0 && G::nchan>=32 && G::nchan<=192);
	assert(G::nspad >= 0 && G::nspad <= 16);
	assert(sizeof(unsigned) == 4);

	return unpack24(stdin, stdout);
}
