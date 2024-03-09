#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <iostream> 
#include <fstream>
#include <sstream>

//rgb limit between selecting white or black pixel
const int BLACK_WHITE_LIMIT = 150;

//print debug log or not
const bool DEBUG = true;

const std::string BA_VID_PATH = "badapple1080.mp4";
const std::string B_VID_PATH = "PS2_black.mp4";
const std::string W_VID_PATH = "PS2_white.mp4";

void printDebug(std::string debugString) {
	if (!DEBUG) return;
	std::cout << std::endl << "DEBUG : " << debugString << std::endl;
}

int main()
{
	//open the bad apple video file
	cv::VideoCapture ba_cap(BA_VID_PATH);
	if (!ba_cap.isOpened()) {
		std::cout << "Cannot open the BA video file.\n";
		return -1;
	}

	//open the ps2 black video file
	cv::VideoCapture b_cap(B_VID_PATH);
	if (!b_cap.isOpened()) {
		std::cout << "Cannot open the black video file.\n";
		return -1;
	}

	//open the ps2 white video file
	cv::VideoCapture w_cap(W_VID_PATH);
	if (!w_cap.isOpened()) {
		std::cout << "Cannot open the white video file.\n";
		return -1;
	}

	int frameCount = ba_cap.get(cv::CAP_PROP_FRAME_COUNT);

	cv::Size outputSize(1920, 1080);

	cv::Mat baFrame;
	cv::Mat whiteFrame;
	cv::Mat blackFrame;

	//iterate over the frames in the BA vid
	for (int f = 0; f < frameCount; f++) {

		ba_cap.set(cv::CAP_PROP_POS_FRAMES, f);
		b_cap.set(cv::CAP_PROP_POS_FRAMES, f);
		w_cap.set(cv::CAP_PROP_POS_FRAMES, f);

		ba_cap.read(baFrame);
		b_cap.read(blackFrame);
		w_cap.read(whiteFrame);

		cvtColor(baFrame, baFrame, cv::COLOR_BGR2GRAY);

		//iterate over the pixels of the frame
		for (int i = 0; i < baFrame.rows; i++) {

			uchar* rowPointer = baFrame.ptr<uchar>(i);
			for (int j = 0; j < baFrame.cols; j++) {

				//store the pixel value
				uchar pixelValue = rowPointer[j];

				//if white, change the pixel fo the base black frame to the white frame's pixel
				if (pixelValue > BLACK_WHITE_LIMIT) {
					int b = whiteFrame.at<cv::Vec3b>(i, j)[0];
					int g = whiteFrame.at<cv::Vec3b>(i, j)[1];
					int r = whiteFrame.at<cv::Vec3b>(i, j)[2];

					blackFrame.at<cv::Vec3b>(i, j)[0] = b;
					blackFrame.at<cv::Vec3b>(i, j)[1] = g;
					blackFrame.at<cv::Vec3b>(i, j)[2] = r;
				}

				//otherwise, ignore (keep the black pixel)
			}
		}

		//save the now changed black frame to a png
		std::vector<uchar> buffer;
		cv::imencode(".png", blackFrame, buffer);
		cv::imwrite("ps2frames\\frame_"+std::to_string(f)+".png", blackFrame);

		printDebug("screenshot " + std::to_string(f) + " saved");

		//clear the memory
		buffer.clear();
	}

	
	//create a video with all the frames
	system("ffmpeg -start_number 0 -r 29.97 -i ps2frames\\frame_%d.png -vcodec libx264 -b:v 4000k video_output.avi");
	printDebug("created video_output.avi");

	//add audio to the video
	system("ffmpeg -i video_output.avi -i badapple.mp3 -map 0:v -map 1:a -b:a 255k -b:v 4000k baps2.avi");
	printDebug("created final video");
	

	return 0;
}
