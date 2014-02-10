#include "Image.h"

//still need to write copy constrctr ( might improve performance over reading the complete image again and again )
//assignmentoperator - Rule of three?

Image::Image(){};

Image::Image(std::string filename) :
	filename(filename)
{
	inputImage = corona::OpenImage(filename.c_str(), corona::PF_R8G8B8); //Open the image using Corona lib

	imageWidth  = inputImage->getWidth();
	imageHeight = inputImage->getHeight();

	//allocate pixel buffer
	imageData = new Pixel[imageWidth * imageHeight];

	pixels = inputImage->getPixels();
	byte* p = (byte*)pixels;

	//(from the lib.. )
	// we're guaranteed that the first eight bits of every pixel is red
	for (int i = 0; i < imageWidth * imageHeight; ++i) 
	{
		imageData[i].r = *p++;
		imageData[i].g = *p++;
		imageData[i].b = *p++;
	}
}

Image::~Image()
{
	//delete &pixels;
	//delete [] imageData;
}

bool Image::saveToFile(std::string filename)
{
	byte* data = new byte[imageWidth * imageHeight * 3]; //3 bytes per pixel
	for (int i = 0; i < imageWidth * imageHeight; ++i) 
	{
		data[i * 3] = imageData[i].r;
		data[(i * 3) + 1] = imageData[i].g;
		data[(i * 3) + 2] = imageData[i].b;
	}

	corona::Image* saveImage = corona::CreateImage(imageWidth, imageHeight, corona::PF_R8G8B8, data);

	//delete [] data; //free up them mems

	return corona::SaveImage(filename.c_str(), corona::FF_PNG, saveImage);

	/*
	* Do we have to write our own file export?
	* If so, convert to BMP
	* Still need to hack all the details tho


	//Create a new file for writing
	FILE *pFile = fopen(filename.c_str(), "wb");

	if(pFile == NULL)
	{ 
	return false;
	}

	BITMAPINFOHEADER BMIH;
	BMIH.biSize = sizeof(BITMAPINFOHEADER);
	BMIH.biimageWidth = _imageWidth;
	BMIH.biimageHeight = _imageHeight;
	BMIH.biPlanes = 1;
	BMIH.biBitCount = 32;
	BMIH.biCompression = 0;
	BMIH.biSizeImage = _imageWidth * _imageHeight * 3;
	BMIH.biXPelsPerMeter = 0;
	BMIH.biYPelsPerMeter = 0;
	BMIH.biClrUsed = 0;
	BMIH.biClrImportant = 0;

	BITMAPFILEHEADER bmfh;
	int nBitsOffset = sizeof(BITMAPFILEHEADER) + BMIH.biSize; 
	long lImageSize = BMIH.biSizeImage;
	long lFileSize = nBitsOffset + lImageSize;

	bmfh.bfType = 0x4D42;
	bmfh.bfSize = lFileSize;
	bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = nBitsOffset;

	//Write the bitmap file header
	unsigned int nWrittenFileHeaderSize = fwrite(&bmfh, 1, sizeof(BITMAPFILEHEADER), pFile);

	//move file point to the end of file header
	fseek(pFile, sizeof(BITMAPFILEHEADER), SEEK_SET);

	//And then the bitmap info header
	unsigned int nWrittenInfoHeaderSize = fwrite(&BMIH, 1, sizeof(BITMAPINFOHEADER), pFile);

	//move file point to the begging of bitmap data
	fseek(pFile, nBitsOffset, SEEK_SET);

	unsigned int nWrittenDIBDataSize = 	fwrite(pixels, 1, lImageSize, pFile); //(void*)&imageData
	fclose(pFile);
	return true; */
}

void Image::convertToColor(OutputColorEnum color)
{
	switch(color)
	{
	case GRAYSCALE:
		for (int i = 0; i < imageWidth * imageHeight; ++i) 
		{
			int val = ((int)imageData[i].r + (int)imageData[i].g + (int)imageData[i].b) / 3;
			imageData[i].r = imageData[i].g = imageData[i].b = (byte)val;
		}
		break;

	case RED:
		for (int i = 0; i < imageWidth * imageHeight; ++i) 
		{
			imageData[i].g = (byte)0; //color em black? or white? what is best?
			imageData[i].b = (byte)0;
		}
		break;

	case GREEN:
		for (int i = 0; i < imageWidth * imageHeight; ++i) 
		{
			imageData[i].r = (byte)0; //I kinda prefer black
			imageData[i].b = (byte)0;
		}
		break;

	case BLUE:
		for (int i = 0; i < imageWidth * imageHeight; ++i) 
		{
			imageData[i].g = (byte)0; //Makes the image better defined
			imageData[i].r = (byte)0;
		}
		break;

	case INVERTED://added just for fun
		for (int i = 0; i < imageWidth * imageHeight; ++i) 
		{
			imageData[i].r = (byte)(255 - (int)imageData[i].r);
			imageData[i].g = (byte)(255 - (int)imageData[i].g);
			imageData[i].b = (byte)(255 - (int)imageData[i].b);
		}
		break;
	}
}

bool Image::calculateGrayBins(int bins) //presuming this is a grayscale image!
{	
	histogramBins = new int[bins](); //allocate new spees -> might be messy tho. 

	for (int i = 0; i < imageWidth * imageHeight; ++i) 
	{
		int r = imageData[i].r; 
		int g = imageData[i].g; 
		int b = imageData[i].b;

		int intensity = (int)((r+g+b) / 3);
		int val = floor((((intensity * bins)) / 256) + 0.5); //floor+0.5 for proper rounding
		histogramBins[val]++;
	}

	return true;
}

bool Image::EqualizeImage(int bins)
{
	int imageSize = (imageWidth * imageHeight);
	//scaling factor, x = 255 / number of pixels
	int scale = 255 / imageSize;

	int* lookupHistogram = new int[bins];

	// Build Cumulative Histogram which will be our lookup table 
	lookupHistogram[0] = histogramBins[0] * scale; 
	int sum = 0;
	for (int i = 1; i < bins; i++) { 
		sum += histogramBins[i];
		lookupHistogram[i] = sum * scale;
	}

	//now reassign all pixelss
	for (int i = 0; i < imageSize; i++) { 
		//
		int newValue = lookupHistogram[imageData[i].r];
		if (newValue > 255) 
		{ 
			imageData[i].r = imageData[i].g = imageData[i].b = (int)(255); 
		} 
		else 
		{
			imageData[i].r = imageData[i].g = imageData[i].b = (int)(newValue); 
		}
	} 	
	delete [] lookupHistogram; //cleanup
	return true;

}

bool Image::saveHistogramAsCSV(int bins, std::string color)
{
	std::ofstream csvFile;
	std::stringstream ss;
	ss << bins << "_" << color << "_" << getFileNameWithoutExtension() << ".csv";
	csvFile.open(ss.str()); 
	csvFile << "binnr" << "," << "Density"  << std::endl; 

	for (int i = 0 ; i < bins; i++) 
	{ 
		csvFile << i << "," << (int)histogramBins[i]  << std::endl; 
	} 

	csvFile.close(); 
	return true;
}

bool Image::Exists()
{
	return inputImage;
}

int Image::getWidth(){
	return imageWidth;
}

int Image::getHeight(){
	return imageHeight;
}

std::string Image::getFileName()
{
	return filename;
}

std::string Image::getFileNameWithoutExtension()
{
	std::stringstream ss(filename);
	std::string item;
	std::getline(ss, item, '.');
	return item;
}

