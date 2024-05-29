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

class Unpack24 {
public:
	virtual int unpack24(FILE* fin, FILE* fout) = 0;

	static Unpack24& factory();
};

class Unpack24b: public Unpack24 {
public:
	virtual int unpack24(FILE* fin, FILE* fout);
};

class Unpack24lw: public Unpack24 {
public:
	virtual int unpack24(FILE* fin, FILE* fout);
};

int Unpack24b::unpack24(FILE* fin, FILE* fout)
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

#define AAS 24
#define BBS 16
#define CCS  8
#define DDS  0

#define AA (0xFFU<<AAS)
#define BB (0xFFU<<BBS)
#define CC (0xFFU<<CCS)
#define DD (0xFFU<<DDS)

int Unpack24lw::unpack24(FILE* fin, FILE* fout)
{
	int issw_d24 = G::nchan*3/4;
	int issw = issw_d24 + G::nspad;
	int ossw = G::nchan + G::nspad;
	unsigned *ibuf = new unsigned[issw];
	unsigned *obuf = new unsigned[ossw];

	fprintf(stderr, "Unpack24lw::unpack24\n");

	while(fread(ibuf, sizeof(unsigned), issw, fin) == issw){
		unsigned *obp = obuf;
		unsigned chid = 0x20;
		for (int iw = 0; iw < issw_d24; iw +=3, obp += 4){
			unsigned *ibp = ibuf+iw;

			obp[0] = (ibp[0]&(BB|CC|DD)) <<  8                       | chid++;
			obp[1] = (ibp[0]&(AA      )) >> 16| (ibp[1]&(CC|DD)) <<16| chid++;
			obp[2] = (ibp[1]&(AA|BB   )) >>  8| (ibp[2]&(DD))    <<24| chid++;
			obp[3] = (ibp[2]&(AA|BB|CC))                             | chid++;

		}
		for (int iw = 0; iw < G::nspad; iw +=1, obp += 1){
			*obp = ibuf[issw_d24+iw];
		}
		if (fwrite(obuf, sizeof(unsigned), ossw, fout) != ossw){
			return -1;
		}
	}
	return 0;
}

Unpack24& Unpack24::factory()
{
	const char* value = getenv("UNPACK24LW");
	if (value && *value == '1'){
		return *new Unpack24lw;
	}else{
		return *new Unpack24b;
	}
}

int main(int argc, char* argv[])
{
	if (argc >= 2) G::nchan = atoi(argv[1]);
	if (argc >= 3) G::nspad = atoi(argv[2]);

	assert(G::nchan%32 == 0 && G::nchan>=32 && G::nchan<=192);
	assert(G::nspad >= 0 && G::nspad <= 16);
	assert(sizeof(unsigned) == 4);

	return Unpack24::factory().unpack24(stdin, stdout);
}
