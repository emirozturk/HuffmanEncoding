#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#define LONG_LONG_BIT_UZUNLUGU (sizeof(long long) * 8)
#define MAX_HUFF_BUFFER_BOYUTU (20*1024*1024) //20 MB
unsigned long long ulonglongMSBMask = 1;
struct header
{
	unsigned int karakterSayisi;//Headerdan okunabilmesi iÁin ngram say˝s˝
	unsigned int kodUzunlugu;//Preorder gezilmi˛ aac˝n kod uzunluu
	unsigned char *kod;//Preorder gezilmi˛ aac˝n kodlar˝n˝n byte'a Áevrilmi˛ hali
	unsigned char* karakterler;
}typedef Header;
Header h;
struct dugum
{
	unsigned short karakter;
	unsigned int frekans;
	unsigned long long kod;
	unsigned int kodUzunlugu;
	struct dugum *sag;
	struct dugum *sol;
	struct dugum *onceki;
	struct dugum *sonraki;
	struct dugum *ata;
}typedef Dugum;

struct buffer {
	unsigned char *veri;
	unsigned uzunluk;
}typedef Buffer;

struct kodMap
{
    short uzunluk[64];
}typedef KodMap;
KodMap *kodDizisi[UINT_MAX];

Dugum *kok;

unsigned long long bitMask[65]={0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF, 0x1FFFF, 0x3FFFF, 0x7FFFF, 0xFFFFF, 0x1FFFFF, 0x3FFFFF, 0x7FFFFF, 0xFFFFFF, 0x1FFFFFF, 0x3FFFFFF, 0x7FFFFFF, 0xFFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF, 0x1FFFFFFFF, 0x3FFFFFFFF, 0x7FFFFFFFF, 0xFFFFFFFFF, 0x1FFFFFFFFF, 0x3FFFFFFFFF, 0x7FFFFFFFFF, 0xFFFFFFFFFF, 0x1FFFFFFFFFF, 0x3FFFFFFFFFF, 0x7FFFFFFFFFF, 0xFFFFFFFFFFF, 0x1FFFFFFFFFFF, 0x3FFFFFFFFFFF, 0x7FFFFFFFFFFF, 0xFFFFFFFFFFFF, 0x1FFFFFFFFFFFF, 0x3FFFFFFFFFFFF, 0x7FFFFFFFFFFFF, 0xFFFFFFFFFFFFF, 0x1FFFFFFFFFFFFF, 0x3FFFFFFFFFFFFF, 0x7FFFFFFFFFFFFF, 0xFFFFFFFFFFFFFF, 0x1FFFFFFFFFFFFFF, 0x3FFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFF, 0x1FFFFFFFFFFFFFFF, 0x3FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};

unsigned int uintOku(FILE *z)
{
	unsigned int deger = 0;
	unsigned char *p;
	p = (unsigned char*)&(deger);
	*p = fgetc(z);
	*(p + 1) = fgetc(z);
	*(p + 2) = fgetc(z);
	*(p + 3) = fgetc(z);
	return deger;
}
unsigned short varMi(unsigned long long kod, unsigned int kodUzunlugu)
{
    //000001010101011110101010111 3
    /*kod = kod << (LONG_LONG_BIT_UZUNLUGU - kodUzunlugu);
    Dugum *temp = kok;
    for (unsigned int i = 0; i < kodUzunlugu; i++)
    {
        if (kod & ulonglongMSBMask)
            temp = temp->sol;
        else
            temp = temp->sag;
        kod <<= 1;
    }
    return temp->karakter;*/
    kod = kod & bitMask[kodUzunlugu];
    if(kodDizisi[kod]==NULL)
        return 1000;
    return kodDizisi[kod]->uzunluk[kodUzunlugu];
}
void koduAc(FILE *okunacak, char *dosyaAdi, unsigned int offset)
{
	fseek(okunacak, -1, SEEK_END);
	unsigned char artikBitSayisi = fgetc(okunacak);
	long dosyaBoyutu = ftell(okunacak);
	long bitSayisi = (dosyaBoyutu - offset - 1) * 8 - artikBitSayisi;
	fseek(okunacak, offset, SEEK_SET);
	char *temp = (char *)malloc(strlen(dosyaAdi) - 4);
	strncpy(temp, dosyaAdi, strlen(dosyaAdi) - 5);
	temp[strlen(dosyaAdi) - 5] = 'o';
	temp[strlen(dosyaAdi) - 4] = '\0';
	FILE *acilan = fopen(temp, "wb");
	long sayac = 0;
	unsigned long long kod = 0;
	unsigned int kodUzunlugu = 0;
	unsigned char okunanKod = '\0';
	unsigned short bulunan;
	unsigned int bufferSayaci = 0;
	Buffer *b = (Buffer *)calloc(1, sizeof(Buffer));
	b->veri = (unsigned char *)calloc(MAX_HUFF_BUFFER_BOYUTU, sizeof(unsigned char));
	Buffer *hBufferi = (Buffer *)calloc(1, sizeof(Buffer));
	hBufferi->veri = (unsigned char *)calloc(MAX_HUFF_BUFFER_BOYUTU, 1);
	unsigned int hBufferSayaci = 0;
	while (sayac < bitSayisi)
	{
		if (sayac % 8 == 0)
		{
			if (hBufferSayaci%MAX_HUFF_BUFFER_BOYUTU == 0)
			{
				fread(hBufferi->veri, MAX_HUFF_BUFFER_BOYUTU, 1, okunacak);
				hBufferSayaci = 0;
			}
			okunanKod = hBufferi->veri[hBufferSayaci];
			++hBufferSayaci;
		}
		kod = kod << 1 | ((okunanKod >> 7) % 2);
		okunanKod = okunanKod << 1;
		++kodUzunlugu;
		bulunan = varMi(kod, kodUzunlugu);
		if (bulunan != 1000)
		{
			if (bufferSayaci == MAX_HUFF_BUFFER_BOYUTU)
			{
				fwrite(b->veri, bufferSayaci, 1, acilan);
				bufferSayaci = 0;
				for (int i = 0; i<MAX_HUFF_BUFFER_BOYUTU; i++)b->veri[i] = '\0';
			}
			b->veri[bufferSayaci] = bulunan;
			++bufferSayaci;
			kod = 0;
			kodUzunlugu = 0;
		}
		++sayac;
	}
	fwrite(b->veri, bufferSayaci, 1, acilan);
	bufferSayaci = 0;
	for (int i = 0; i<MAX_HUFF_BUFFER_BOYUTU; i++)b->veri[i] = '\0';
	fclose(acilan);
}
int a;
void huffmanDizisiOlustur(Dugum *kok)
{
    if (kok != NULL)
    {
        if (kok->karakter != 1000)
        {
            if(kodDizisi[kok->kod]==NULL)
            {
                kodDizisi[kok->kod] = (KodMap *)malloc(sizeof(KodMap));
                for(int i=0;i<64;i++)kodDizisi[kok->kod]->uzunluk[i] = 1000;
            }
            kodDizisi[kok->kod]->uzunluk[kok->kodUzunlugu] = kok->karakter;
            if(kok->kod == 32)
                a=1;
            return;
        }
        huffmanDizisiOlustur(kok->sag);
        huffmanDizisiOlustur(kok->sol);
    }
}
void huffmanKoduOlustur(Dugum *kok, unsigned int kod, unsigned int uzunluk)
{
    if (kok != NULL)
    {
        kok->kod = kod;
        kok->kodUzunlugu = uzunluk;
        huffmanKoduOlustur(kok->sag, kod * 2, uzunluk + 1);
        huffmanKoduOlustur(kok->sol, kod * 2 + 1, uzunluk + 1);
    }
}
Dugum *agaciOlustur()
{
	unsigned int karakterSayaci = 0;
	unsigned int kodSayaci = 0;
	Dugum *kok = (Dugum *)calloc(1, sizeof(Dugum));
	kok->karakter = 1000;
	kok->ata = NULL;
	Dugum *bulunulan = kok;
	for (unsigned int i = 0; i < h.kodUzunlugu - 1; i++)
	{
		if (i && ((i % 8) == 0))
			kodSayaci++;
		if (((h.kod[kodSayaci] >> 7) % 2) == 0)
		{
			Dugum* yeniDugum = (Dugum *)calloc(1, sizeof(Dugum));
			yeniDugum->karakter = 1000;
			bulunulan->sol = yeniDugum;
			yeniDugum->ata = bulunulan;
			bulunulan = bulunulan->sol;
		}
		else
		{
			bulunulan->karakter = h.karakterler[karakterSayaci];
			karakterSayaci++;
			bulunulan = bulunulan->ata;
			while (bulunulan->sol != NULL && bulunulan->sag != NULL)
				bulunulan = bulunulan->ata;
			Dugum* yeniDugum = (Dugum *)calloc(1, sizeof(Dugum));
			yeniDugum->karakter = 1000;
			bulunulan->sag = yeniDugum;
			yeniDugum->ata = bulunulan;
			bulunulan = bulunulan->sag;
		}
		h.kod[kodSayaci] = h.kod[kodSayaci] << 1;
	}
	bulunulan->karakter = h.karakterler[karakterSayaci];
    huffmanKoduOlustur(kok,0,0);
    huffmanDizisiOlustur(kok);
	return kok;
}
unsigned int headerOku(FILE *okunacak)
{
	h.karakterSayisi = uintOku(okunacak);
	h.kodUzunlugu = uintOku(okunacak);
	unsigned int charKodUzunlugu = (unsigned int)ceil((double)h.kodUzunlugu / 8);
	h.kod = (unsigned char *)calloc(1, charKodUzunlugu);
	for (unsigned int i = 0; i < charKodUzunlugu; i++)h.kod[i] = fgetc(okunacak);
	h.karakterler = (unsigned char *)calloc(1, h.karakterSayisi);
	for (unsigned int i = 0; i < h.karakterSayisi; i++)h.karakterler[i] = fgetc(okunacak);
	return 4 + 4 + charKodUzunlugu + h.karakterSayisi;
}
int main(int argc, char * argv[])
{
	ulonglongMSBMask <<= (LONG_LONG_BIT_UZUNLUGU - 1);
    char *dosyaAdi = argv[1];
	FILE *okunacak = fopen(dosyaAdi, "rb");
	unsigned int offset = headerOku(okunacak);
	kok = agaciOlustur();
	koduAc(okunacak, dosyaAdi, offset);
	fclose(okunacak);
}