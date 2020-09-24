#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/types_c.h"
#include "vector"
#include "string"

using namespace cv;
class Detect {
private:
	Mat srcImage;//ԭͼ
	Mat desImage;//Ԥ�������ͼ

public:
	Detect(std::string);
	std::vector<RotatedRect> getRect();
	Mat getSrc();

};
Detect::Detect(std::string imgURL) {
	srcImage = imread(imgURL);//"F:\Pictures\qr.png""F:\Pictures\������.png"

	if (srcImage.empty()) {
		printf("�ļ�������");
		exit(1);
	}
	cvtColor(srcImage, desImage, CV_RGBA2GRAY);
	//imshow("�Ҷ�ͼ", desImage);
	//waitKey(0);
	/*if (show)
		imshow("�Ҷ�ͼ", cimage);
	return cimage;*/
	GaussianBlur(desImage, desImage, Size(3, 3), 0);
	//imshow("��˹ģ��ͼ", desImage);
	//waitKey(0);
	Mat imageSobelX, imageSobelY;
	Sobel(desImage, imageSobelX, CV_16S, 1, 0);
	Sobel(desImage, imageSobelY, CV_16S, 0, 1);
	convertScaleAbs(imageSobelX, imageSobelX, 1, 0);
	convertScaleAbs(imageSobelY, imageSobelY, 1, 0);

	Mat horizontalImage = imageSobelX - imageSobelY;
	Mat verticalImage = imageSobelY - imageSobelX;
	imshow("x-y", horizontalImage);
	imshow("y-x", verticalImage);

	//waitKey(0);
	Mat horizontalElement = getStructuringElement(MORPH_CROSS, Size(5, 1));
	Mat verticalElement = getStructuringElement(MORPH_CROSS, Size(1, 5));
	Mat element = getStructuringElement(MORPH_CROSS, Size(10, 10));

	auto process = [horizontalElement, verticalElement, element](Mat img, bool direction) {
		blur(img, img, Size(3, 3));
		medianBlur(img, img, 3);
		//imshow("��ֵ�˲�ͼ", desImage);
		//waitKey(0);

		threshold(img, img, 80, 255, CV_THRESH_BINARY);
		//imshow("��ֵ��ͼ", desImage);
		//waitKey(0);

		if (direction) {
			//����ˮƽ���������ͣ���������м�Ŀ�϶
			morphologyEx(img, img, MORPH_DILATE, horizontalElement);

			//�ڴ�ֱ�����ϸ�ʴ������������ַ�
			morphologyEx(img, img, MORPH_ERODE, verticalElement);
		}
		else {
			morphologyEx(img, img, MORPH_DILATE, verticalElement);
			morphologyEx(img, img, MORPH_ERODE, horizontalElement);


		}
		morphologyEx(img, img, MORPH_OPEN, element);
		morphologyEx(img, img, MORPH_CLOSE, element);

	};

	process(verticalImage, false);
	process(horizontalImage, true);

	//resize(image, image, Size(srcimage.cols, srcimage.rows), 0, 0, INTER_AREA);

	desImage = verticalImage + horizontalImage;
	//ȥ���ַ�


	erode(desImage, desImage, element);
	erode(desImage, desImage, element);
	dilate(desImage, desImage, element);

	imshow("ȥ����϶", desImage);
	waitKey();

}
Mat Detect::getSrc() {
	return srcImage;
}
std::vector<RotatedRect> Detect::getRect() {
	std::vector<std::vector<Point>> contours;
	std::vector<Vec4i> hiera;
	Mat cimage;
	findContours(desImage, contours, hiera, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	std::vector<RotatedRect>barcodes;
	for (int i = 0; i < contours.size(); i++)
	{
		double area = contourArea(contours[i]);
		RotatedRect rect = minAreaRect(contours[i]);
		double ratio = area / (rect.size.width * rect.size.height);
		if (ratio > 0.75 && area > 200)
		{
			if (rect.size.width < rect.size.height) {
				rect.angle -= 90.;
				swap(rect.size.width, rect.size.height);
			}
			rect.size.width *= 1.1;
			//drawContours(srcImage, contours, i, 255, -1);
			barcodes.push_back(rect);
		}
	}
	//imshow("���ͼ", srcImage);
	//waitKey();
	return barcodes;
}
int main(int argc, char* argv)
{
	if (argc < 2) {
		std::string path = "";
		std::cin >> path;//"F:\\VirtualBox\\project\\AWS.png"
		Detect* myclass = new Detect(path);
		std::vector<RotatedRect> barcodes = myclass->getRect();
		for (int n = 0; n < barcodes.size(); n++) {

			//printf("%f %f\n", barcodes[n].size.width, barcodes[n].size.height);

			Point2f vertices[4];
			barcodes[n].points(vertices);
			Point2f dst_vertices[] = {
				Point2f(0, barcodes[n].size.height - 1),
				Point2f(0, 0),
				Point2f(barcodes[n].size.width - 1, 0),
				Point2f(barcodes[n].size.width - 1, barcodes[n].size.height - 1) };

			Mat M = getPerspectiveTransform(vertices, dst_vertices);
			Mat perspective;
			warpPerspective(myclass->getSrc(), perspective, M, barcodes[n].size, cv::INTER_LINEAR);
			imshow("���ͼ", perspective);
			waitKey();

			for (int i = 0; i < 4; i++)
				line(myclass->getSrc(), vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0));
		}
		imshow("���ͼ", myclass->getSrc());
		waitKey();

	}
	return 0;
}
