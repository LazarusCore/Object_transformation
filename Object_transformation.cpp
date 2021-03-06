// Object_transformation.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

using namespace cv;
using namespace std;


Mat image, image_gray, contour_image;
string filename = "Reference Image.png";


int main(int argc, char** argv)
{
	vector<Vec4i> hierarchy;
	vector<vector<Point>> contour_points;
	vector<vector<Point>> kept_points;
	vector<vector<Point>> obj1, obj2;



	//load image, check if its loading properly
	image = imread(filename.c_str(), IMREAD_COLOR);
	if (image.empty())
	{
		cout << "No image to display" << endl;
		return -1;
	}

	//Configure a window
	namedWindow("Reference Image", CV_WINDOW_AUTOSIZE);
	imshow("Reference Image", image);
	waitKey(0);

	//Carry out Binarisation for contour recognition
	cvtColor(image, image_gray, CV_BGR2GRAY);
	threshold(image_gray, contour_image, 125, 255, THRESH_BINARY);
	imshow("Reference Image", contour_image);
	waitKey(0);

	//find the contours in the image, and output them
	findContours(contour_image, contour_points, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));

	//find the inner triangle of each one
	for (int i = 0; i < contour_points.size(); i++)
	{
		if (contourArea(contour_points[i]) > 20 && contourArea(contour_points[i]) < 1000)
		{
			kept_points.push_back(contour_points[i]);
		}
	}
	
	Mat output = Mat::zeros(contour_image.size(), CV_8UC3);
	vector<vector<Point>> approx_contours(kept_points.size());


	for (int i = 0; i < kept_points.size(); i++)
	{
		
		approxPolyDP(Mat(kept_points[i]), approx_contours[i], 3, true);
		drawContours(output, approx_contours, i, Scalar(255, 0, 0), 1, 8, hierarchy, 0, Point());
	}
	
	vector<Point> organised(3);
	vector<vector<Point>>organised_triangles(3);
	Point a, b, c;									//placehold for points
	double min_dist;								//placeholder for the minimum distance
	float difference1, difference2, difference3;	//placeholder for diference calcs
	int q = 0;										//counter for ordering


	for (int i = 0; i < approx_contours.size(); i++)
	{
		
		//find the first set of min dist
		difference1 = sqrt(pow(approx_contours[i][0].x - approx_contours[i][1].x, 2) + pow(approx_contours[i][0].y - approx_contours[i][1].y, 2));
		min_dist = difference1;

		//dump coords into placeholders
		a = cv::Point(approx_contours[i][0].x, approx_contours[i][0].y);
		b = cv::Point(approx_contours[i][1].x, approx_contours[i][1].y);
		c = cv::Point(approx_contours[i][2].x, approx_contours[i][2].y);

		//check the second set of points
		difference2 = sqrt(pow(approx_contours[i][1].x - approx_contours[i][2].x, 2) + pow(approx_contours[i][1].y - approx_contours[i][2].y, 2));

		if (difference2 < min_dist)
		{
			//Then this becomes the new minimum distance 
			min_dist = difference2;
			//Place holders obtain the new coordinate sets
			a = cv::Point(approx_contours[i][1].x, approx_contours[i][1].y);
			b = cv::Point(approx_contours[i][2].x, approx_contours[i][2].y);
			c = cv::Point(approx_contours[i][0].x, approx_contours[i][0].y);
		}

		difference3 = sqrt(pow(approx_contours[i][2].x - approx_contours[i][0].x, 2) + pow(approx_contours[i][2].y - approx_contours[i][0].y, 2));
		//Rinse, Repeat
		if (difference3 < min_dist) 
		{
			//Then this becomes the new minimum distance 
			min_dist = difference3;
			//Place holders obtain the new coordinate sets
			a = cv::Point(approx_contours[i][2].x, approx_contours[i][2].y);
			b = cv::Point(approx_contours[i][0].x, approx_contours[i][0].y);
			c = cv::Point(approx_contours[i][1].x, approx_contours[i][1].y);
		}
		
		float difference_points[] = { difference1,difference2,difference3 };

		//Sort the points for use in findHomography, create a vector of the sorted points
		if (a.x > b.x) 
		{
			//Tip is to the left of A
			cv::Point temp = a; //Store this point into a temporary variable
			a = b;
			b = temp;
		}
		if (c.x <= a.x) 
		{ //If the tip is above A (x & y coordinates) but higher than point A
			if (c.y < a.y) 
			{
				//A is BL
				organised[0] = a; //Push to front
								//B is BR
				organised[2] = b; //Push to end
			}
			else {
				//B is BL
				organised[0] = b;
				//A is BR
				organised[2] = a;
			}
		}
		//If the tip is below point A/B and to the right of B 
		else  if (c.x >= b.x) 
		{
			//But y is still above B
			if (c.y < b.y) 
			{
				organised[0] = a;
				organised[2] = b;
			}
			else {
				//B is BR and A is BL
				organised[0] = b;
				organised[2] = a;
			}
		}
		else {
			//If the y coordinate tip is above A then A is BR and B is BL
			if (c.y < a.y) 
			{
				organised[0] = a;
				organised[2] = b;
			}
			else {
				organised[0] = b;
				organised[2] = a;
			}
			//First point in ordered will be BL, 2nd will be tip (c) and 3rd will be BR 
		}
		organised_triangles[q] = organised;
		q++;


	}


	//Use findHomography: This finds rotational and Translational matricies
	vector < vector<Point>> output_array, trans_output_array, normal_output_array;

	Mat H = findHomography(organised_triangles[0], organised_triangles[1], CV_RANSAC);
	
	//Cant use this as we need the extrinsic camera parameters
	//	decomposeHomographyMat(H, K, output_array, trans_output_array, normal_output_array);


	imshow("Reference Image", output);
	waitKey(0);

	return 0;

}

