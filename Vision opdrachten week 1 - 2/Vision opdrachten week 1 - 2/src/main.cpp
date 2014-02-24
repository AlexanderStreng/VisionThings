#include <iostream>
#include "Image.h"
#include "Histogram.h"
#include "exectimer.h"
#include "basetimer.h"
#include "globals.h"
#include <random>
#include <time.h>       /* time */

std::string filename, yesOrNo;
Image originalImage;
std::stringstream ss;
BaseTimer* bt;

void stop(std::string msg);
void grayScaleRoutine(); //Preventing clutter in main function
void colorRoutine();
void invertRoutine();
void saltAndPepperRoutine();
void randomRoutine();

int main(int argc, char* argv[])
{
	//check argv[1] for input, if not, ask for input.
	if(argc < 2)
	{
		std::cout << std::endl << "No filename found. Could you please insert the filename? " << std::endl;
		getline(std::cin, filename);
	} 
	else
	{		
		std::cout << "Command line argument found." << std::endl;
		filename = argv[1];
	}

	std::cout << "I am going to use the file: '" << filename << "'" << std::endl;

	bt = new BaseTimer(); //lets create a fancy microsecond timer.

	bt->start();
	originalImage = Image(filename);
	bt->stop(); bt->store("Load_Image");

	if (!originalImage.Exists()) {
		stop("Image could not be loaded");
	}

	std::cout << "Loaded img:" << originalImage.getFileNameWithoutExtension() << " dimensions(WxH):"  
		<< originalImage.getWidth() << " x " << originalImage.getHeight() << " \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;

	grayScaleRoutine();
	colorRoutine();
	invertRoutine();
	saltAndPepperRoutine();

	ss.str("");
	ss << "Timings_" << originalImage.getFileNameWithoutExtension() << ".csv"; // save as png
	bt->save(ss.str());
	delete bt;
	stop("");
}

void grayScaleRoutine()
{
	std::cout << "Converting the image to grayscale." << std::endl;
	bt->reset(); bt->start();
	Image grayScaleImage = Image(originalImage);
	bt->stop(); bt->store("Copy_image");
	ss.str("");
	ss << "grey_" << grayScaleImage.getFileNameWithoutExtension() << ".png"; // save as png

	bt->reset(); bt->start();
	bool save = grayScaleImage.saveToFile(ss.str(), GRAYSCALE);
	bt->stop(); bt->store("Save_Grayscale");

	if(save)
	{
		std::cout<< "Saving image succeeded. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;

		{
			bt->reset();bt->start();
			Histogram greyScale10Histogram = Histogram(10, GRAYSCALE, &grayScaleImage);
			bt->stop(); bt->store("Calc_bin10_gray");

			if(greyScale10Histogram.saveHistogramAsCSV("Grayscale")){
				std::cout<< "Saving grayscale image-histogram(10) succeeded. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;
			}
		} //call histogram dtor

		{
			bt->reset();bt->start();
			Histogram greyScale256Histogram = Histogram(256, GRAYSCALE, &grayScaleImage);
			bt->stop(); bt->store("Calc_bin256_gray");

			bt->reset(); bt->start();
			greyScale256Histogram.saveHistogramAsCSV("Grayscale");
			bt->stop(); bt->store("SaveCSV_bin256_gray");

			std::cout<< "Saving grayscale image-histogram(256) succeeded. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;
			bt->reset(); bt->start();
			greyScale256Histogram.EqualizeImage();
			bt->stop(); bt->store("Equalize_image_bin256");

			std::cout << "Equalized image. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;
		}
		ss.str("");
		ss << "equalized_" <<grayScaleImage.getFileNameWithoutExtension() << ".png"; // save as png

		bt->reset(); bt->start();
		if( grayScaleImage.saveToFile(ss.str(), GRAYSCALE))
		{
			bt->stop(); bt->store("Save_Equalized");
			std::cout<< "Saving image succeeded. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;
		}
		else 
		{
			stop("Saving image failed.");
		}
	}
}

void invertRoutine()
{
	bt->reset(); bt->start();
	Image invertedImage = Image(originalImage);
	bt->stop(); bt->store("Copy_originalImage");

	bt->reset(); bt->start();
	invertedImage.convertToColor(INVERTED);
	bt->stop();	bt->store("Convert_Inverted");

	std::cout << "Inverting image colors. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;

	ss.str("");
	ss << "inverted_" <<invertedImage.getFileNameWithoutExtension() << ".png"; // save as png

	bt->reset(); bt->start();
	bool save = invertedImage.saveToFile(ss.str(), INVERTED);
	bt->stop();	bt->store("Save_inverted");

	if(save)
	{
		std::cout<< "Saving image succeeded. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;
	}
	else 
	{
		stop("Saving image failed.");
	}
}

void colorRoutine()
{
	bool save = false;
	{
		ss.str("");
		std::cout << "Converting the image to R(ed) color channel." << std::endl;

		bt->reset(); bt->start();
		Image colorImage = Image(originalImage);
		bt->stop();	bt->store("Copy_originalImage");

		ss << "R_" << colorImage.getFileNameWithoutExtension() << ".png"; // save as png

		bt->reset(); bt->start();
		save = colorImage.saveToFile(ss.str(), RED);
		bt->stop();	bt->store("Save_Red");

		if(save)
		{
			std::cout<< "Saving image RED succeeded. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;

			bt->reset(); bt->start();
			Histogram red10Histogram = Histogram(10, RED, &colorImage);
			bt->stop();	bt->store("Calc_bin10_red");

			bt->reset(); bt->start();
			red10Histogram.saveHistogramAsCSV("Red");
			bt->stop();	bt->store("SaveCSV_bin256_red");

			std::cout<< "Saving red image-histogram(10) succeeded. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;
		}

		ss.str("");
		std::cout << "Converting the image to G(reen) color channel." << std::endl;
		ss << "G_" << colorImage.getFileNameWithoutExtension() << ".png"; // save as png

		bt->reset(); bt->start();
		save = colorImage.saveToFile(ss.str(), GREEN);
		bt->stop();	bt->store("Save_Green");

		if(save)
		{
			std::cout<< "Saving image Green succeeded. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;

			bt->reset(); bt->start();
			Histogram green10Histogram = Histogram(10, GREEN, &colorImage);
			bt->stop();	bt->store("Calc_bin10_green");

			bt->reset(); bt->start();
			green10Histogram.saveHistogramAsCSV("Green");
			bt->stop();	bt->store("SaveCSV_bin256_green");

			std::cout<< "Saving green image-histogram(10) succeeded. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;
		}

		ss.str("");
		std::cout << "Converting the image to B(lue) color channel." << std::endl;
		ss << "B_" << colorImage.getFileNameWithoutExtension() << ".png"; // save as png
		bt->reset(); bt->start();
		save = colorImage.saveToFile(ss.str(), BLUE);
		bt->stop();	bt->store("Save_Blue");

		if(save)
		{
			std::cout<< "Saving image Blue succeeded. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;


			bt->reset(); bt->start();
			Histogram blue10Histogram = Histogram(10, BLUE, &colorImage);
			bt->stop();	bt->store("Calc_bin10_blue");

			bt->reset(); bt->start();
			blue10Histogram.saveHistogramAsCSV("Blue");
			bt->stop();	bt->store("SaveCSV_bin256_blue");

			std::cout<< "Saving blue image-histogram(10) succeeded. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;
		}
	}
}

void saltAndPepperRoutine()
{
	ss.str("");
	std::cout << "Applying Salt and pepper noise to image." << std::endl;

	bt->reset(); bt->start();
	Image saltAndPepperImage = Image(originalImage);
	bt->stop();	bt->store("Copy_originalImage");

	bt->reset(); bt->start();
	int bitsFlipped = saltAndPepperImage.addNoise(5, SALTANDPEPPER);
	bt->stop();	bt->store("Add_salt_pepper_noise");
	std::cout << "Added noise to image. Percentage converted: " << (((double)bitsFlipped / (originalImage.getWidth() * originalImage.getHeight())) * 100) << "%. \t(in " << bt->elapsedMilliSeconds()  << " milliseconds)" << std::endl;

	ss << "noise_" << saltAndPepperImage.getFileNameWithoutExtension() << ".png"; // save as png
	bt->reset(); bt->start();
	saltAndPepperImage.saveToFile(ss.str(), FULLCOLOUR);
	bt->stop();	bt->store("Save_saltAndPepperImage");
	std::cout<< "Saving noise image succeeded. \t(in " << bt->elapsedMilliSeconds() << " milliseconds)" << std::endl;
}

void stop(std::string msg)
{
	std::cout << msg << std::endl << "Press enter to stop.." << std::endl;
	std::cin.get();
	exit(0);
}