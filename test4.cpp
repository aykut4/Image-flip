//	Programming And Algortihms 2,
//	Progtest homework #1,
//	Image flip.
//
//	This is a code review submission.
//
//	Author		   : Aykut Sahin
//	Contact		   : sahinayk@fit.cvut.cz
//
//	Program description: A program that may take an image file as an input
//			     and process its data in a desired way in order to
//			     write a new image file based on the processed data
//			     of the previous one.

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <vector>
#include <cmath>

using namespace std;

enum COLORDEPTH {
	monochrome = 0,      		// 1 bit, 1 channel  : 1bit/pixel
	rgb = 2,	     		// 1 bit, 3 channels : 3bit/pixel
	rgba = 3,            		// 1 bit, 4 channels : 4bit/pixel
	grayscale = 12,      		// 8 bit, 1 channels : 8bit/pixel
	highcolor = 14,      		// 8 bit, 3 channels: 24bit/pixel
	truecolor = 15,      		// 8 bit, 4 channels : 32bit/pixel
	truecoloralpha = 16, 		// 16 bit, 1 channels : 16bit/pixel
	deepcolor = 18,      		// 16 bit, 3 channels: 48bit/pixel
	ultracolor = 19      		// 16 bit, 4 channels: 64bit/pixel
};

enum OPERATION {
	getColorDepthLE = 100, 		// makes the util function binary get the colordepth when little endian
	getColorDepthBE = 101, 		// makes the util function binary get the colordepth when big endian
	getWidth = 102,        		// makes the util function binary get the actual width when big endian
	getHeight = 103,       		// makes the util function binary get the actual height when big endian
	getNumberChannel = 104 		// makes the util function binary get the number of channel(s)
};

class CImage {
	uint16_t endianity; 		// to store the 2B of data that tells about endianity
	uint16_t image_width; 		// to store the 2B of data that tells about image width in pixels
	uint16_t image_height; 		// to store the 2B of data that tells about image height in pixels
	uint16_t pixel_format; 		// to store the 2B of data that tells about pixel format of the image

	uint8_t * pixels_1bit; 	 	// is to build an array for processing of 1 bit images
	uint8_t * pixels_8bit; 	 	// is to build an array for processing of 8 bit images
	uint16_t * pixels_16bit; 	// is to build an array for processing of 16 bit images

	size_t size; 			// to store the size of the image in bytes
	int width; 			// to store the actual number of pixels in width
	int height; 			// to store the actual number of pixels in height
	int widthByte8; 		// to store the number of bytes in width
	int widthByte16; 		// to store the number of 2 bytes in width
	unsigned short bitPerPixel; 	// to store the number of bits in one pixel of the image
	unsigned short numberChannel; 	// to store the number of channel(s) in one pixel of the image
	COLORDEPTH colorDepth; 		// to store the colordepth

	bool isReadable; 		// if 3 conditions (header, pixelformat, content) are true, this is true
	bool isRead; 			// if the image was successfuly read
	bool isArray8bit; 		// if the array is a type of uint8_t
	bool isFlippedVertical; 	// if flipping vertically was successful
	bool isFlippedHorizontal; 	// if flipping horizontally was successful
	bool isWritten; 		// if writing the target .img file was successful

	public:
	CImage () {} // constructor
	~CImage () { // destructor
		if (this->isRead == true) {
			if (this->isArray8bit == true) delete [] this->pixels_8bit;
			else delete [] this->pixels_16bit;
		}
		else return;
	}
	void getSize (const char * srcFileName); 	     // gets the size of the image file
	void readHeader (const char * srcFileName); 	     // reads the header information
	void getWidthHeight (); 			     // gets the actual number of pixels in width and height
	void getColorDepth ();				     // gets the colordepth
	void verifyImage (); 				     // verifies if everything is okay about the image
		void binary (uint16_t number, OPERATION op); // utility function that changes its operation according to the parameter op
		bool isReadableFunction (); 		     // getter
		bool isReadFunction (); 		     // getter
		bool isWrittenFunction (); 		     // getter
	void readPixelData (const char * srcFileName);	     // reads the image pixel data
	void flipVertical (); 				     // flips vertically
	void flipHorizontal (); 			     // flips horizontally
		void swap8 (uint8_t & a, uint8_t & b); 	     // swaps the values of a and b
		void swap16 (uint16_t & a, uint16_t & b);    // same as 1 line above
	void write (const char * dstFileName); 		     // writes image output
	void printImageFile (); 			     // test purposes
};
// ######################################## GETSIZE ##########################################
void CImage::getSize (const char * srcFileName) {
	ifstream myImage (srcFileName, ios::in | ios::binary);	// load file
	myImage.seekg (0, ios::end);				// traverse to the end of the file
	this->size = myImage.tellg();				// get the size of the file
	myImage.seekg (0, ios::beg);				// traverse back to the beginning
	myImage.close();					// close file
	return;
}
// ######################################## READHEADER ##########################################
void CImage::readHeader (const char * srcFileName) {
	ifstream myImage (srcFileName, ios::in | ios::binary);		// load file
	myImage.read ( (char*)&this->endianity, sizeof(uint16_t) );	// read header
	myImage.read ( (char*)&this->image_width, sizeof(uint16_t) );	// " "
	myImage.read ( (char*)&this->image_height, sizeof(uint16_t) );	// " "
	myImage.read ( (char*)&this->pixel_format, sizeof(uint16_t) ); 	// " "
	myImage.close();						// close file
	return;
}
// ######################################## GETWIDTHHEIGHT ##########################################
void CImage::getWidthHeight () {
	// depends on the endianity; sets the width and height
	if (this->endianity == 0x4949) {this->width = this->image_width; this->height = this->image_height;}
	else if (this->endianity == 0x4d4d) {binary(this->image_width, getWidth); binary(this->image_height, getHeight);}
	else return;
	return;
}
// ######################################## GETCOLORDEPTH ##########################################
void CImage::getColorDepth () {
	// depends on the endianity; sets the colordepth
	if (this->endianity == 0x4949) binary(this->pixel_format, getColorDepthLE);
	else if (this->endianity == 0x4d4d) binary(this->pixel_format, getColorDepthBE);
	else return;
	return;
}
// ######################################## VERIFYIMAGE ##########################################
void CImage::verifyImage () {
	// Verify header
	if (this->endianity != 0x4949 && this->endianity != 0x4d4d) {this->isReadable = false; return;}
	if (this->width == 0 || this->height == 0) {this->isReadable = false; return;}
	// Verify pixel format
	switch (this->colorDepth) {
		case monochrome    : this->bitPerPixel = 1;  		break;
		case rgb           : this->bitPerPixel = 3;  		break;
		case rgba          : this->bitPerPixel = 4;  		break;
		case grayscale     : this->bitPerPixel = 8;  		break;
		case highcolor     : this->bitPerPixel = 24; 		break;
		case truecolor     : this->bitPerPixel = 32; 		break;
		case truecoloralpha: this->bitPerPixel = 16;		break;
		case deepcolor     : this->bitPerPixel = 48; 		break;
		case ultracolor    : this->bitPerPixel = 64; 		break;
		default            : this->isReadable = false;		return;
	}
	// Verify content
	if (this->colorDepth == monochrome || this->colorDepth == rgb || this->colorDepth == rgba) {
		if (this->width % 8 != 0) this->width += (8 - (this->width % 8));
	}
	int totalPixel = this->width * this->height;
	size_t sizePixelDataInBytes = (totalPixel * this->bitPerPixel)/8;
	size_t sizeImageFile = sizePixelDataInBytes + 8;

	if (this->size == sizeImageFile) isReadable = true;
	else isReadable = false;
	return;
}
// ######################################## GETTERS ##########################################
bool CImage::isReadableFunction () {
	return this->isReadable;
}
bool CImage::isReadFunction () {
	return this->isRead;
}
bool CImage::isWrittenFunction () {
	return this->isWritten;
}
// ######################################## BINARY ##########################################
void CImage::binary (uint16_t number, OPERATION op) {
	// brute force way to break a decimal number down to a binary 
	unsigned short m15 = 0, m14 = 0, m13 = 0, m12 = 0, m11 = 0, m10 = 0, m9 = 0, m8 = 0, m7 = 0, m6 = 0, m5 = 0, m4 = 0, m3 = 0, m2 = 0, m1 = 0, m0 = 0;
	if (number >= 32768)	{number %= 32768; m15++;}
	if (number >= 16384)	{number %= 16384; m14++;}
	if (number >= 8192)	{number %= 8192	; m13++;}
	if (number >= 4096)	{number %= 4096	; m12++;}
	if (number >= 2048)	{number %= 2048	; m11++;}
	if (number >= 1024)	{number %= 1024	; m10++;}
	if (number >= 512)	{number %= 512	; m9++ ;}
	if (number >= 256)	{number %= 256	; m8++ ;}
	if (number >= 128)	{number %= 128	; m7++ ;}
	if (number >= 64)	{number %= 64	; m6++ ;}
	if (number >= 32)	{number %= 32	; m5++ ;}
	if (number >= 16)	{number %= 16	; m4++ ;}
	if (number >= 8)	{number %= 8	; m3++ ;}
	if (number >= 4)	{number %= 4	; m2++ ;}
	if (number >= 2)	{number %= 2	; m1++ ;}
	if (number >= 1)	{number %= 1	; m0++ ;}
	// Operations:
	if (op == getColorDepthLE) this->colorDepth = (COLORDEPTH)((m4*16)+(m3*8)+(m2*4)+(m1*2)+(m0*1)); 	// set colordepth for little endian
	else if (op == getColorDepthBE) this->colorDepth = (COLORDEPTH)((m12*16)+(m11*8)+(m10*4)+(m9*2)+(m8*1));// set colordepth for big endian
	else if (op == getWidth) this->width = ((m8*1)+(m9*2)+(m10*4)+(m11*8)+(m12*16)+(m13*32)+(m14*64)+(m15*128)+(m0*256)+(m1*512)+(m2*1024)+(m3*2048)+(m4*4096)+(m5*8192)+(m6*16384)+(m7*32768));   // set width for big end
	else if (op == getHeight) this->height = ((m8*1)+(m9*2)+(m10*4)+(m11*8)+(m12*16)+(m13*32)+(m14*64)+(m15*128)+(m0*256)+(m1*512)+(m2*1024)+(m3*2048)+(m4*4096)+(m5*8192)+(m6*16384)+(m7*32768)); // set height for big end
	else if (op == getNumberChannel) this->numberChannel = (((m1*2)+(m0*1))+1);				// set num of channels off of the last 2 bits
	else return;
	return;
}
// ######################################## READPIXELDATA ##########################################
void CImage::readPixelData (const char * srcFileName) {
	// 16 bit per channel
	if (this->colorDepth == truecoloralpha || this->colorDepth == deepcolor || this->colorDepth == ultracolor) {
		this->pixels_16bit = new uint16_t [(this->size-8)/2]; 				   // mem alloc
		ifstream myImage (srcFileName, ios::in | ios::binary); 				   // file load
		if (!myImage.is_open()) {this->isRead = false; return;} 			   // file open, check
		myImage.seekg(8, ios::beg); 							   // skip header
		myImage.read ( (char*)this->pixels_16bit, sizeof(uint16_t) * ((this->size-8)/2) ); // read data
		if (myImage.fail()) {this->isRead = false; myImage.close(); return;} 		   // check read success
		myImage.close (); 								   // close file
		this->isArray8bit = false; 							   // set success bools
		this->isRead = true; 								   // set success bools
		return;
	}
	// 1 bit / 8 bit per channel
	else if (this->colorDepth == monochrome || this->colorDepth == rgb || this->colorDepth == rgba || this->colorDepth == grayscale || this->colorDepth == highcolor || this->colorDepth == truecolor) {
		this->pixels_8bit = new uint8_t [this->size-8];					   // mem alloc
		ifstream myImage (srcFileName, ios::in | ios::binary);				   // file load
		if (!myImage.is_open()) {this->isRead = false; return;}				   // file open, check
		myImage.seekg(8, ios::beg);							   // skip header
		myImage.read ( (char*)this->pixels_8bit, sizeof(uint8_t)*(this->size-8) );	   // read data
		if (myImage.fail()) {this->isRead = false; myImage.close(); return;}		   // check read success
		myImage.close ();								   // close file
		this->isArray8bit = true;							   // set success bools
		this->isRead = true;								   // set success bools
		return;
	}
	else {
		this->isRead = false;
		return;
	}
}
// ######################################## FLIPVERTICAL ##########################################
void CImage::flipVertical () {
	int currentRowHigh = this->height-1; // util variable
	// 1 bit / 8 bit per channel
	if (this->isArray8bit == true) {
		this->widthByte8 = (this->bitPerPixel * this->width)/8; // set number of bytes in a row
		// swapping appropriate pixels in order to achive a vertical flip
		for (int i = 0; i < floor(this->height/2); i++) {
			for (int j = 0; j < this->widthByte8; j++) {
				swap8 (pixels_8bit[ (i*this->widthByte8)+j ], pixels_8bit[ (currentRowHigh*this->widthByte8)+j ]);
			}
			currentRowHigh--;
		}
		this->isFlippedVertical = true;
	}
	// 16 bit per channel
	else {
		this->widthByte16 = (this->bitPerPixel * this->width)/16; // set number of 2 bytes in a row
		// swapping appropriate pixels in order to achive a vertical flip
		for (int i = 0; i < floor(this->height/2); i++) {
			for (int j = 0; j < this->widthByte16; j++) {
				swap16 (pixels_16bit[ (i*this->widthByte16)+j ], pixels_16bit[ (currentRowHigh*this->widthByte16)+j ]);
			}
			currentRowHigh--;
		}
		this->isFlippedVertical = true;
	}
	return;
}
// ######################################## FLIPHORIZONTAL ##########################################
void CImage::flipHorizontal () {
	if (this->colorDepth == monochrome || this->colorDepth == rgb || this->colorDepth == rgba) {
		// TO DO FOR 1 BIT IMAGES
	}
	else {
		uint16_t tmp = (uint16_t)this->colorDepth; // get colordepth
		binary (tmp, getNumberChannel); // get this->numberChannel
		// 8 bit per channel
		if (this->isArray8bit == true) {
			this->widthByte8 = (this->bitPerPixel*this->width)/8;
			// swapping appropriate pixels in order to achieve a horizontal flip
			for (int i = 0; i < floor(this->width/2); i++) {
				for (int j = 0; j < this->height; j++) {
					for (int k = 0; k < this->numberChannel; k++) {
						swap8 (pixels_8bit[(j*this->widthByte8)+(i*this->numberChannel)+k], pixels_8bit[((j+1)*this->widthByte8)-((i+1)*this->numberChannel)+k]);
					}
				}
			}
			this->isFlippedHorizontal = true;
		}
		// 16 bit per channel
		else {
			this->widthByte16 = (this->bitPerPixel*this->width)/16;
			// swapping appropriate pixels in order to achieve a horizontal flip
			for (int i = 0; i < floor(this->width/2); i++) {
				for (int j = 0; j < this->height; j++) {
					for (int k = 0; k < this->numberChannel; k++) {
						swap16 (pixels_16bit[(j*this->widthByte16)+(i*this->numberChannel)+k], pixels_16bit[((j+1)*this->widthByte16)-((i+1)*this->numberChannel)+k]);
					}
				}
			}
			this->isFlippedHorizontal = true;
		}
		return;
	}
}
// ######################################## SWAP8 ##########################################
void CImage::swap8 (uint8_t & a, uint8_t & b) {
	uint8_t tmp = a;
	a = b;
	b = tmp;
}
// ######################################## SWAP16 ##########################################
void CImage::swap16 (uint16_t & a, uint16_t & b) {
	uint16_t tmp = a;
	a = b;
	b = tmp;
}
// ######################################## WRITE ##########################################
void CImage::write (const char * dstFileName) {
	ofstream myImage (dstFileName, ios::out | ios::binary);							// load file
	if (!myImage.is_open()) {this->isWritten = false; return; }						// file open, failure check
	myImage.write ((char*)&this->endianity, sizeof(uint16_t));						// write header
	myImage.write ((char*)&this->image_width, sizeof(uint16_t));						// " "
	myImage.write ((char*)&this->image_height, sizeof(uint16_t));						// " "
	myImage.write ((char*)&this->pixel_format, sizeof(uint16_t));						// " "
	if (this->isArray8bit == true) myImage.write ((char*)this->pixels_8bit, sizeof(uint8_t)*(this->size-8));// write for 8 bit per channel
	else myImage.write ( (char*)this->pixels_16bit, sizeof(uint16_t) * ((this->size-8)/2) );		// write for 16 bit per channel
	if (myImage.fail()) {this->isWritten = false; myImage.close(); return;} 				// check for write failure
	myImage.close();											// close file
	this->isWritten = true;											// set bool for success check
	return;
}
// ######################################## PRINTIMAGEFILE ##########################################
void CImage::printImageFile ()
{
	cout << "Endianity: " << (int)this->endianity << endl << "Image width: " << (int)this->image_width << endl << "Image height: " << (int)this->image_height << endl << "Pixel format: " << (int)this->pixel_format << endl << "isReadable " << boolalpha << this->isReadable << endl << "isRead: " << boolalpha << this->isRead << endl << "isArray8bit: " << boolalpha << this->isArray8bit << endl << "isFlippedVertical: " << boolalpha << this->isFlippedVertical << "isFlippedHorizontal" << boolalpha << this->isFlippedHorizontal << endl << "isWritten: " << boolalpha << this->isWritten << endl << "colorDepth: " << this->colorDepth << endl << "Size: " << this->size << endl;

	if (this->isArray8bit == true && this->isRead == true) {
		int anan = 0;
		for (size_t i = 0; i < this->size-8; i++)
		{
			cout << (int)this->pixels_8bit[i] << "   ";
			anan++;
			if (this->width == anan) {cout << endl; anan = 0;}
		}
		cout << endl;
	}
	else if (this->isArray8bit == false && this->isRead == true) {
		int anan = 0;
		for (size_t i = 0; i < (this->size-8)/2; i++)
		{
			cout << (int)this->pixels_16bit[i] << "   ";
			anan++;
			if (this->width == anan) {cout << endl; anan = 0;}
		}
		cout << endl;
	}
	else {}
}
// ######################################## FLIPIMAGE ##########################################
bool flipImage (const char * srcFileName, const char * dstFileName, bool flipHorizontal, bool flipVertical)
{
	CImage c;
	c.getSize(srcFileName);
	c.readHeader(srcFileName);
	c.getWidthHeight();
	c.getColorDepth();
	c.verifyImage();
	if (!c.isReadableFunction()) return false;
	c.readPixelData (srcFileName);
	if (!c.isReadFunction()) return false;
	if (flipVertical == true) c.flipVertical();
	if (flipHorizontal == true) c.flipHorizontal();
	c.write (dstFileName);
	if (!c.isWrittenFunction()) return false;
	return true;
}
