#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	Mat src, src_gray;
	int min_lab = 125, max_lab = 255, i_cont = 0, odleglosc = 106;
	float szerokosc, wysokosc, index = 750;
	int max_thresh = 255;
	RNG rng(12345);
	string window[] = { "Zrodlo", "Wyniki" }; 			// source, results

	//export results to document
	fstream file;
	stringstream stream_to_file;
	string string_to_file;
	file.open("wyniki_pomiaru.txt", ios::out);			//results.txt

	int mt_seconds = 0; 								//messuring time in seconds
	int frame_counter = 0;

	VideoCapture capture = VideoCapture(0);				//capturing images from usb camera
	/// creating windows
	namedWindow(window[0], CV_WINDOW_AUTOSIZE);	;namedWindow(window[1], CV_WINDOW_AUTOSIZE);
	createTrackbar("Odleglosc obiektywu od obiektu", window[0], &odleglosc, 1000, NULL); //len-messured item distance
	createTrackbar("Dolna wartosc progowa", window[0], &min_lab, max_thresh, NULL);		//down treshold
	createTrackbar("Gorna wartosc progowa", window[0], &max_lab, max_thresh, NULL);		//up treshold
	while (waitKey(20) != 27)															// capturing and analizing 27 fps
	{
		capture >> src;
		frame_counter++;
		if (frame_counter == 27)		// count 27 frames and increment mt(messuring time)
		{
			frame_counter = 0;
			mt_seconds++;
		}
		string string;
		stringstream out;
		Mat inRange_output;
		vector<vector<Point> > contours;
		vector<Point> cont_poly;
		vector<Vec4i> hierarchy;

		cvtColor(src, src_gray, CV_BGR2GRAY);					// rgb to gray conversion
		inRange(src_gray, min_lab, max_lab, inRange_output);	// edges detection
		erode(inRange_output, inRange_output, cv::Mat());
		dilate(inRange_output, inRange_output, cv::Mat());
		blur(inRange_output, inRange_output, Size(3, 3));
		findContours(inRange_output, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0)); //find contours

		vector<RotatedRect> minRect(contours.size());
		vector<RotatedRect> minEllipse(contours.size());

		for (uint i = 0; i < contours.size(); i++)
		{
			minRect[i] = minAreaRect(Mat(contours[i])); /// Find the rotated rectangles
			if (contours[i].size() > 100)
			{
				minEllipse[i] = fitEllipse(Mat(contours[i])); // Find ellipses for each contour
			}
			i_cont = i;
		}

		Mat drawing = Mat::zeros(inRange_output.size(), CV_8UC3);
		for (uint i = 0; i < contours.size(); i++)
		{
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
					rng.uniform(0, 255));
			drawContours(src_gray, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point()); // draw contours
			ellipse(src_gray, minEllipse[i], color, 2, 8);									// draw ellipses

			// Draw rotated rectangle
			Point2f rect_points[4];
			minRect[i].points(rect_points);
			for (int j = 0; j < 4; j++)
			{
				line(src_gray, rect_points[j], rect_points[(j + 1) % 4], color,
						1, 8);
			}
			if (minRect[i].size.width >= 60 && minRect[i].size.height >= 60				//setting size range of detected rectangles
					&& minRect[i].size.width <= 610										//to set info of their dimensions in widow 'drawing'
					&& minRect[i].size.height <= 450)
			{
				szerokosc = (odleglosc * minRect[i].size.width) / index;				//converting dimension in ppi to dimension in mm
				wysokosc = (odleglosc * minRect[i].size.height) / index;
				out << i << " : " << szerokosc << " x " << wysokosc << endl;			// stream to put in text in window
				string = out.str();
				out.flush();
				putText(drawing, string, Point(30, 10), FONT_HERSHEY_SIMPLEX, 0.3, (250, 250, 250), 1);//puting information about rectangles in 'drawing' window
				stream_to_file << "Czas: " << mt_seconds << " s. Klatka: "			//puting dimensions in mm to stream_to_file
						<< frame_counter << " Obiekt " << i + 1 << ": "
						<< szerokosc << " x " << wysokosc << " mm\n";
				string_to_file = stream_to_file.str();
				stream_to_file.flush();
			}
		}
		imshow(window[1], drawing);
		imshow(window[0], src_gray);
	}
	capture.release();
	if (file.good())
	{
		file << string_to_file << endl;												//puting stream_to_file saved in string to document file
		file.close();
	} else
		cout << "blad!";
	return 0;

}
