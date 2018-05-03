#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAX_BUFFER_BOYUTU (1024*1024*100) //100 MB
#define MAX_HUFF_BUFFER_BOYUTU (20*1024*1024) //20 MB
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
}typedef Dugum;

struct huffmanDugumu
{
	unsigned long long kod;
	unsigned char kodUzunlugu;
}typedef HuffmanDugumu;

struct buffer {
	unsigned char *veri;
	unsigned uzunluk;
}typedef Buffer;

unsigned int frekans[256];
HuffmanDugumu karakterler[256];
unsigned int dugumSayisi = 0;
Dugum *kokDugum, *sonDugum;

unsigned long long bitMask[65]={0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000, 0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000, 0x1000000, 0x2000000, 0x4000000, 0x8000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000, 0x100000000, 0x200000000, 0x400000000, 0x800000000, 0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000, 0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000, 0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000, 0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000, 0x10000000000000, 0x20000000000000, 0x40000000000000, 0x80000000000000, 0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000, 0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000};
unsigned char bitVer(const unsigned long long deger,unsigned int bitIndex)
{
    if(deger&bitMask[bitIndex])
        return 1;
    else return 0;
}
unsigned int dosyaBoyutuVer(char *input)
{
	FILE *fp = fopen(input, "rb");
	fseek(fp, 0L, SEEK_END);
	long boyut = ftell(fp);
	fclose(fp);
	return (unsigned int)boyut;
}

void uintYaz(unsigned int deger, FILE *z)
{
	unsigned char *p;
	p = (unsigned char*)&(deger);
	fputc(*p, z);
	fputc(*(p + 1), z);
	fputc(*(p + 2), z);
	fputc(*(p + 3), z);
}

void huffmanKodla(char *dosyaAdi)
{
	char *temp = (char *)calloc(1, strlen(dosyaAdi) + 6);
	strcpy(temp, dosyaAdi);
	strcat(temp, ".huff");
	FILE *okunan = fopen(dosyaAdi, "rb");
	FILE *yazilacak = fopen(temp, "ab");
	HuffmanDugumu h;
	unsigned char byteSayaci = 0;
	unsigned char byte = 0;
	unsigned long long kod;
	unsigned int bufferSayaci = 0;
	long dosyaBoyutu = dosyaBoyutuVer(dosyaAdi);
	int parcaSayisi = (int)ceil((double)dosyaBoyutu / (MAX_BUFFER_BOYUTU));
    unsigned char kodUzunlugu=0;
	FILE *dosya = fopen(dosyaAdi, "rb");

	Buffer *b = (Buffer *)calloc(1, sizeof(Buffer));
	b->veri = (unsigned char *)calloc(1, MAX_BUFFER_BOYUTU + 1);

	Buffer *yazmaBufferi = (Buffer *)calloc(1, sizeof(Buffer));
	yazmaBufferi->veri = (unsigned char *)calloc(1, MAX_HUFF_BUFFER_BOYUTU);

	for (int i = 0; i < parcaSayisi; i++)
	{
		fseek(dosya, i*MAX_BUFFER_BOYUTU, SEEK_SET);
		if (i!=0 && i + 1 == parcaSayisi)for (int j = 0; j < MAX_BUFFER_BOYUTU; j++)b->veri[j] = '\0';
		fread(b->veri, MAX_BUFFER_BOYUTU, 1, dosya);
		if (i + 1 == parcaSayisi)
			b->uzunluk = (dosyaBoyutu % MAX_BUFFER_BOYUTU);
		else
			b->uzunluk = MAX_BUFFER_BOYUTU;
		unsigned int sayac = 0;
        unsigned int bUzunluk = b->uzunluk;
		while (sayac < bUzunluk)
		{
			h = karakterler[b->veri[sayac++]];
			kod = h.kod;
            kodUzunlugu = h.kodUzunlugu;
			for (int i = 0; i < h.kodUzunlugu; ++i)
			{
				byte = byte << 1 | (bitVer(kod,kodUzunlugu-i));
				++byteSayaci;
				if (byteSayaci == 8)
				{
					if (bufferSayaci == MAX_HUFF_BUFFER_BOYUTU)
					{
						fwrite(yazmaBufferi->veri, bufferSayaci, 1, yazilacak);
						bufferSayaci = 0;
						for (int i = 0; i<MAX_HUFF_BUFFER_BOYUTU; i++)yazmaBufferi->veri[i] = '\0';
					}
					yazmaBufferi->veri[bufferSayaci] = byte;
					++bufferSayaci;
					byte = 0;
					byteSayaci = 0;
				}
			}
		}
		fwrite(yazmaBufferi->veri, bufferSayaci, 1, yazilacak);
		bufferSayaci = 0;
	}
	fclose(dosya);
	for (int i = 0; i < 8 - byteSayaci; i++)
		byte = byte << 1;
	fputc(byte, yazilacak);
	fputc(8 - byteSayaci, yazilacak);//artik bit sonda;
	fclose(okunan);
	fclose(yazilacak);
}

void siraliEkle(Dugum *yeniDugum)
{
	if (kokDugum == NULL)
		kokDugum = yeniDugum;
	else
	{
		Dugum *temp = kokDugum;
		while (temp->sonraki != NULL && temp->frekans > yeniDugum->frekans)
			temp = temp->sonraki;
		if (temp == kokDugum && temp->frekans <= yeniDugum->frekans)
		{
			yeniDugum->sonraki = kokDugum;
			kokDugum->onceki = yeniDugum;
			kokDugum = yeniDugum;
		}
		else if (temp->sonraki == NULL && temp->frekans > yeniDugum->frekans)
		{
			temp->sonraki = yeniDugum;
			yeniDugum->onceki = temp;
		}
		else
		{
			temp->onceki->sonraki = yeniDugum;
			yeniDugum->onceki = temp->onceki;
			yeniDugum->sonraki = temp;
			temp->onceki = yeniDugum;
		}
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
void huffmanDizisiOlustur(Dugum *kok)
{
	if (kok != NULL)
	{
		if (kok->karakter != 1000)
		{
			karakterler[kok->karakter].kod = kok->kod;
			karakterler[kok->karakter].kodUzunlugu = kok->kodUzunlugu;
			return;
		}
		huffmanDizisiOlustur(kok->sag);
		huffmanDizisiOlustur(kok->sol);
	}
}
void huffmanAgaciOlustur()
{
	while (kokDugum->sonraki != NULL)
	{
		Dugum* yeniDugum = (Dugum *)calloc(1, sizeof(Dugum));
		while (sonDugum->sonraki != NULL)
			sonDugum = sonDugum->sonraki;
		yeniDugum->sag = sonDugum;
		yeniDugum->sol = sonDugum->onceki;
		yeniDugum->karakter = 1000;
		yeniDugum->frekans = yeniDugum->sag->frekans + yeniDugum->sol->frekans;
		if (kokDugum->sonraki->sonraki != NULL)
		{
			sonDugum = sonDugum->onceki->onceki;
			sonDugum->sonraki = NULL;
			siraliEkle(yeniDugum);
		}
		else
		{
			kokDugum = yeniDugum;
		}
		dugumSayisi--;
	}
	unsigned int kod = 0;
	huffmanKoduOlustur(kokDugum, kod, 0);
	huffmanDizisiOlustur(kokDugum);
}

void bagliListeOlustur()
{
	for (int i = 0; i < 256; i++)
		if (frekans[i])
		{
			Dugum *yeniDugum = (Dugum *)calloc(1, sizeof(Dugum));
			yeniDugum->karakter = i;
			yeniDugum->frekans = frekans[i];
			siraliEkle(yeniDugum);
			dugumSayisi++;
		}
	sonDugum = kokDugum;
	while (sonDugum->sonraki != NULL)
		sonDugum = sonDugum->sonraki;
}

void frekansCikar(char *dosyaAdi)
{
	long dosyaBoyutu = dosyaBoyutuVer(dosyaAdi);
	int parcaSayisi = (int)ceil((double)dosyaBoyutu / (MAX_BUFFER_BOYUTU));
	FILE *dosya = fopen(dosyaAdi, "rb");
	Buffer *b = (Buffer *)calloc(1, sizeof(Buffer));
	b->veri = (unsigned char *)calloc(1, MAX_BUFFER_BOYUTU + 1);
	for (int i = 0; i < parcaSayisi; i++)
	{
		fseek(dosya, i*MAX_BUFFER_BOYUTU, SEEK_SET);
		if (i!=0 && i + 1 == parcaSayisi)
			for (int j = 0; j < MAX_BUFFER_BOYUTU; j++)b->veri[j] = '\0';
		fread(b->veri, MAX_BUFFER_BOYUTU, 1, dosya);
		if (i + 1 == parcaSayisi)
			b->uzunluk = (dosyaBoyutu % MAX_BUFFER_BOYUTU);
		else
			b->uzunluk = MAX_BUFFER_BOYUTU;
		unsigned int sayac = 0;
        unsigned int boyut = b->uzunluk;
		while (sayac < boyut)
		{
			frekans[b->veri[sayac]]++;
			sayac++;
		}
	}
	fclose(dosya);
}
void headerYaz(char *dosyaAdi)
{
	char *temp = (char *)calloc(1, strlen(dosyaAdi) + 6);
	strcpy(temp, dosyaAdi);
	strcat(temp, ".huff");
	FILE *yazilacak = fopen(temp, "wb");
	unsigned int charKodUzunlugu = (unsigned int)ceil((double)h.kodUzunlugu / 8);
	uintYaz(h.karakterSayisi, yazilacak);
	uintYaz(h.kodUzunlugu, yazilacak);
	for (unsigned int i = 0; i < charKodUzunlugu; i++)
		fputc(h.kod[i], yazilacak);
	for (int i = 0; i < h.karakterSayisi; i++)
		fputc(h.karakterler[i], yazilacak);
	fclose(yazilacak);
}
unsigned char *binaryToCharArray(unsigned char* binary, unsigned int bitDizisiUzunlugu)
{
	unsigned int charDizisiUzunlugu = (unsigned int)ceil((double)bitDizisiUzunlugu / 8);
	unsigned char *charDizisi = (unsigned char*)calloc(charDizisiUzunlugu, sizeof(char));
	unsigned char bitDizisiSayaci = 0;
	unsigned int charDizisiSayaci = 0;
	for (unsigned int i = 0; i < bitDizisiUzunlugu; i++)
	{
		charDizisi[charDizisiSayaci] = charDizisi[charDizisiSayaci] << 1 | (binary[i] - 48);
		bitDizisiSayaci++;
		if (bitDizisiSayaci == 8)
		{
			bitDizisiSayaci = 0;
			charDizisiSayaci++;
		}
	}
	for (unsigned int i = 0; i < charDizisiUzunlugu * 8 - bitDizisiUzunlugu; i++)
		charDizisi[charDizisiSayaci] = charDizisi[charDizisiSayaci] << 1;
	return charDizisi;
}
unsigned char *uintToBinaryChar(unsigned int deger)
{
	unsigned char *bin = (unsigned char *)calloc(sizeof(char), 32);
	unsigned int temp = deger;
	for (int i = 0; i < 32; i++)
	{
		bin[i] = (temp % 2) + 48;
		temp = temp >> 1;
	}
	return bin;
}
unsigned char* tempKarakter, *tempKod;
void agaciOku(Dugum *kok, unsigned int *kodUzunlugu, unsigned int*karakterUzunlugu)
{
	if (kok != NULL)
	{
		if (kok->karakter != 1000)
		{
			tempKarakter = (unsigned char *)calloc(1, (*karakterUzunlugu) + 1);
			for (unsigned int i = 0; i < *karakterUzunlugu; i++)tempKarakter[i] = h.karakterler[i];
			tempKarakter[*karakterUzunlugu] = kok->karakter;
			(*karakterUzunlugu)++;
			h.karakterler = tempKarakter;
		}
		tempKod = (unsigned char *)calloc(1, (*kodUzunlugu) + 1);
		strncpy((char *)tempKod, (char *)h.kod, *kodUzunlugu);
		if (kok->karakter != 1000)tempKod[*kodUzunlugu] = '1';
		else tempKod[*kodUzunlugu] = '0';
		(*kodUzunlugu)++;
		h.kod = tempKod;
		agaciOku(kok->sol, kodUzunlugu, karakterUzunlugu);
		agaciOku(kok->sag, kodUzunlugu, karakterUzunlugu);
	}
}
void huffmanAgaciOku(Dugum *kok)
{
	unsigned int kodUzunlugu = 0;
	unsigned int karakterUzunlugu = 0;
	agaciOku(kok, &kodUzunlugu, &karakterUzunlugu);
	h.kodUzunlugu = kodUzunlugu;
	h.karakterSayisi = karakterUzunlugu;
	h.kod = binaryToCharArray(h.kod, h.kodUzunlugu);
}
int main(int argc, char *argv[])
{
	char *dosyaAdi = argv[1];
	frekansCikar(dosyaAdi);
	bagliListeOlustur();
	huffmanAgaciOlustur();
	huffmanAgaciOku(kokDugum);
	headerYaz(dosyaAdi);
	huffmanKodla(dosyaAdi);
}