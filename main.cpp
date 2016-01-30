#include <opencv2/opencv.hpp>
#include <string>

using namespace cv;

const int height = 640;
const int width = 480;
const int frameRate = 30;
const double brightness = 115.0/255;
const int blur_kernel_size = 3;
const int erode_kernel_size = 3;
const int dilate_kernel_size = 3;
const std::string mainWindow = "main";
const std::string altWindow = "alt";
//Hue 120-150
void filter_image_for_color(cv::Mat& hsv_src, cv::Mat& bin_src, int* thresh_src, cv::Mat erode_src, cv::Mat dilate_src)
{
	cv::inRange( hsv_src,
			cv::Scalar( thresh_src[0],
					thresh_src[2],
					thresh_src[4]
                               ),
                     cv::Scalar( thresh_src[1],
                                 thresh_src[3],
                                 thresh_src[5]
                               ),
                     bin_src
                   );

        // eroding then dilating the binary image
        // eroding
        cv::erode(bin_src, bin_src, erode_src);

        //dilating
        cv::dilate(bin_src, bin_src, dilate_src);

        // cv::imshow("show inside",bin_src);
}

int main ( int argc, char **argv )
{
	  VideoCapture cap(1);
	  namedWindow(mainWindow, WINDOW_AUTOSIZE );
	  namedWindow(altWindow,WINDOW_NORMAL);
	  if (!cap.isOpened())
	  {
		  	 std::cout << "Camera opening error" << std::endl;
		  	 return -1;
	  }
	  cap.set(CAP_PROP_FRAME_WIDTH,width);
	  cap.set(CAP_PROP_FRAME_HEIGHT, height);
	  cap.set(CAP_PROP_FPS, frameRate);
	  cap.set(CAP_PROP_BRIGHTNESS,brightness);

	  Mat frame(height,width,CV_8UC3); //inital frame
	  Mat blur_frame(height,width,CV_8UC3); //blured frame
	  Mat hsv_frame(height,width,CV_8UC3); //converted to HSV frame
	  Mat binary_frame(height,width,CV_8UC1);
	  Mat erode_element = getStructuringElement(MORPH_RECT,Size(erode_kernel_size,erode_kernel_size), Point(-1,1));
	  Mat dilate_element = getStructuringElement(MORPH_RECT,Size(dilate_kernel_size,dilate_kernel_size),Point(-1,1));
	  std::vector < std::vector <Point> > contours;
	  std::vector < RotatedRect > rectangles;
	  std::vector < double > areas;
	  std::vector<RotatedRect> sortedRectangles;
	  int thresh_src[] = {40,80,80,130,115,255};

	  cv::imshow(mainWindow,frame);
	  cv::imshow(altWindow,frame);

	  int key;
	  while (true)
	  {
		  key = waitKey(200) & 255;
		  if (key == 27)break;

		  contours.clear();
		  rectangles.clear();
		  sortedRectangles.clear();

		  if (!cap.read(frame))
		  {
			  std::cout << "Could not read frame" << std::endl;
			  continue;
		  }

		  imwrite( "./test.png", frame);

		  medianBlur(frame,blur_frame,blur_kernel_size);
		  cvtColor(blur_frame,hsv_frame,CV_BGR2HSV);
		  filter_image_for_color(hsv_frame,binary_frame, thresh_src,erode_element,dilate_element);
		  cv::findContours(binary_frame.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
		  for (std::vector<Point > cur : contours)
		  {
			  rectangles.push_back(minAreaRect (cur));
		  }

		  uint index = 0;
		  uint compare = 0;
		  bool end_reached = false;

		  //Sort rectangles by area
		  while (!rectangles.empty())
		  {
			  if (index == compare) {
					 compare++;
			  } else if (rectangles.at(compare).size.area() > rectangles.at(index).size.area()) {
				  index = compare;
				  compare = 0;
			  } else {
				  compare ++;

			  }

			  if (compare > rectangles.size()-1) {
			  	end_reached = true;
			  }

			  if (end_reached) {
				  sortedRectangles.push_back(rectangles.at(index));
				  rectangles.erase(rectangles.begin()+index);
				  end_reached = false;
				  index = 0;
				  compare = 0;
			  }
		  }

		  if (sortedRectangles.size() >= 1)
		  {
			  Point2f pts[4];
			  sortedRectangles.at(0).points(pts);
			  rectangle(frame,pts[0],pts[2],Scalar(0,0,0));
		  }

		  drawContours(frame,contours,-1,Scalar(0,255,0),3);
		  imshow(mainWindow, binary_frame);
		  imshow(altWindow,frame);
		  //std::cout << "Brightness is: " << cap.get(CAP_PROP_BRIGHTNESS) << std::endl;
	  }
}
