#include <stdio.h>
#include <time.h>
//#include <cv.h>
//#include <highgui.h>


#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;


Point2f point;
bool addRemovePt = false;

static void onMouse(int event, int x, int y, int /*flags*/, void* /*param*/)
{
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		point = Point2f((float)x, (float)y);
		addRemovePt = true;
	}
}

int main(void){

	VideoCapture cap;
	TermCriteria termcrit(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);
	Size subPixWinSize(10, 10), winSize(31, 31);

	const int MAX_COUNT = 100;
	bool needToInit = true;
	bool nightMode = false;

	cap.open(0);

	if (!cap.isOpened())
	{
		printf("Could not initialize capturing...\n");
		return 0;
	}

	namedWindow("LK Demo", 1);
	//setMouseCallback("LK Demo", onMouse, 0);

	Mat gray, prevGray, image, frame;
	vector<Point2f> points[2];

	for (;;)
	{
		cap >> frame;
		if (frame.empty())
			break;

		frame.copyTo(image);
		cvtColor(image, gray, COLOR_BGR2GRAY);

		if (nightMode)
			image = Scalar::all(0);

		int min_size = (points[0].size() < points[1].size()) ? points[0].size() : points[1].size();
		if (needToInit || min_size < 50)
		{
			// automatic initialization
			goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);
			cornerSubPix(gray, points[1], subPixWinSize, Size(-1, -1), termcrit);
			addRemovePt = false;
			needToInit = false;
		}
		else if (!points[0].empty())
		{
			vector<uchar> status;
			vector<float> err;
			if (prevGray.empty())
				gray.copyTo(prevGray);
			calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize, 3, termcrit, 0, 0.001);
			size_t i, k;
			for (i = k = 0; i < points[1].size(); i++)
			{
				if (addRemovePt)
				{
					if (norm(point - points[1][i]) <= 5)
					{
						addRemovePt = false;
						continue;
					}
				}

				if (!status[i])
					continue;

				points[1][k++] = points[1][i];
				circle(image, points[1][i], 3, Scalar(0, 255, 0), -1, 8);
			}
			points[1].resize(k);
		}

		min_size = (points[0].size() < points[1].size()) ? points[0].size() : points[1].size();

		//if ( min_size < 50)		needToInit = true;

		float dx = 0;
		float dy = 0;
		for (int i = 0 ; i < min_size ; i++){
			circle(image, points[0][i], 3, Scalar(0, 0, 255), -1, 8); // pre
			circle(image, points[1][i], 3, Scalar(0, 255, 0), -1, 8); // cur
			// calc flow of each features
			dx += points[1][i].x - points[0][i].x;
			dy += points[1][i].y - points[0][i].y;
		}
		printf("dx = %5.2f, dy = %f5.2 \n", dx / min_size, dy / min_size);

		if (addRemovePt && points[1].size() < (size_t)MAX_COUNT)
		{
			vector<Point2f> tmp;
			tmp.push_back(point);
			cornerSubPix(gray, tmp, winSize, cvSize(-1, -1), termcrit);
			points[1].push_back(tmp[0]);
			addRemovePt = false;
		}

		/*
		for (int i = 0; i < points[1].size() ; i++){
			circle(image, points[0][i], 3, Scalar(0, 0, 255), -1, 8);
			circle(image, points[1][i], 3, Scalar(0, 255, 0), -1, 8);
		}
		*/
		needToInit = false;
		imshow("LK Demo", image);

		char c = (char)waitKey(10);
		if (c == 27)
			break;
		switch (c)
		{
		case 'r':
			needToInit = true;
			break;
		case 'c':
			points[0].clear();
			points[1].clear();
			break;
		case 'n':
			nightMode = !nightMode;
			break;
		}

		std::swap(points[1], points[0]);
		cv::swap(prevGray, gray);
	}

	return 0;
}




/*
#define PI 3.1416
#define WINSIZE 7            //optical flow�� ������ �������� ������.  ���⼭�� 7*7
#define MAX_COUNT 100  //flow �� ��ó ����Ʈ ��� ����ϴ°�
void main()
{ 
	int L_WIDTH = 100;	// limited window width
	int L_HEIGHT = 100;	// limited window height


	int pressed_key;
    int width, height;
    IplImage* src_color_R; 
    IplImage* src_grey_R; 
    IplImage* pre_src_color_R;  //t-1�� �̹��� ������
    IplImage* pre_src_grey_R;  //t-1�� �̹��� ������
    IplImage* result_view_R;
    CvCapture* capture_R = cvCaptureFromCAM(0);  //��ķ �����

    cvNamedWindow( "Optical Flow", CV_WINDOW_AUTOSIZE ); 
    cvMoveWindow("Optical Flow",0,0);
    //ù �̹��� �޾ƿ���
	src_color_R = cvQueryFrame(capture_R); cvWaitKey(300);
    width  = src_color_R->width;    //�ҽ� �̹��� ����ü���� �̹��� �� �ҷ�����
    height = src_color_R->height;   //�ҽ� �̹��� ����ü���� �̹��� ���� �ҷ�����
    printf("image width : %d    height : %d  \n",width,height);  //�̹����� ũ�⸦ ����ϱ�
    src_grey_R        = cvCreateImage(cvSize(L_WIDTH,L_HEIGHT), 8, 1); 
    pre_src_color_R = cvCreateImage(cvSize(L_WIDTH,L_HEIGHT), 8, 3);   
    pre_src_grey_R  = cvCreateImage(cvSize(L_WIDTH,L_HEIGHT), 8, 1);
    result_view_R     = cvCreateImage(cvSize(width,height), 8, 3);  
    //goodfeature to track 
    //int MAX_COUNT = 100;  //flow �� ��ó ����Ʈ ��� ����ϴ°�
    int num_of_pts_R;
    int pre_num_of_pts_R;
    IplImage* eig_R  = cvCreateImage(cvSize(L_WIDTH,L_HEIGHT), 32, 1 );
    IplImage* temp_R = cvCreateImage(cvSize(L_WIDTH,L_HEIGHT), 32, 1 );
    //������ ����Ǵ°�  �׳� ������ �ſ� ũ�� ����  
    CvPoint2D32f feature_points_R[5000];
    CvPoint2D32f pre_feature_points_R[5000];
    //OpenCV ȭ�鿡 ���ھ���
    char s_output_result[50];
	char window_xy[50];
    CvFont font;
    //����ð� ������
    clock_t start, finish; //����ð�üũ
    double duration;
    //Optical Flow
    char* status_R = 0;
    status_R = new char [MAX_COUNT];
    int flags_R=0;
    IplImage* pyramid_R        = cvCreateImage(cvSize(L_WIDTH,L_HEIGHT), 8, 1);   
    IplImage* prev_pyramid_R = cvCreateImage(cvSize(L_WIDTH,L_HEIGHT), 8, 1);   
    //result_view_R->origin=-1;  //�̹��� ���Ʒ� ������ ���̰� �ϱ�
	//==================================================================
	// �������� ��������� ��ǥ(���Ʒ� �������� ȭ���̱� ������ �����ϴ��� ��ǥ)
	int x_ref = width/2 - L_WIDTH/2;
	int y_ref = height/2 - L_HEIGHT/2;
	//==================================================================

    // initial work  ù �̹����� �� �̹��� ������ ��� ���ִ°�
    //goodfeature to track
    double quality = 0.01;
    double min_distance = 10;
    num_of_pts_R = MAX_COUNT;
    src_color_R = cvQueryFrame( capture_R );
	cvSetImageROI(src_color_R,cvRect(x_ref,y_ref,L_WIDTH,L_HEIGHT));
	cvCvtColor(src_color_R, src_grey_R, CV_BGR2GRAY);
	cvResetImageROI(src_color_R);

	//tracking�ϱ� ���� ��(corner)���� ã��
    cvGoodFeaturesToTrack( src_grey_R, eig_R, temp_R, feature_points_R, &num_of_pts_R, quality, min_distance, 0, WINSIZE, 0, 0.04 );
    cvFindCornerSubPix(src_grey_R, feature_points_R, num_of_pts_R, cvSize(WINSIZE,WINSIZE), cvSize(-1,-1), cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));   
 
    //������ draw
    for(int i=0; i<num_of_pts_R; ++i) 
         cvCircle( result_view_R, cvPointFrom32f(feature_points_R[i]), 1, CV_RGB(0,255,0), -1, 4,0);
 
    //���� �� ó�� ���� ���� ���� ���� �������� ����
    cvCopy(src_grey_R, pre_src_grey_R);
    cvCopy(pyramid_R, prev_pyramid_R);
    //���� ã�� �� ��ǥ ����
    for(int i=0; i<num_of_pts_R; ++i) 
        pre_feature_points_R[i] = feature_points_R[i];
 
    pre_num_of_pts_R = num_of_pts_R;

	// ref�� �̵��ϴ� ������ ����/
	//CvPoint2D32f delta_ref;
		struct Window_move{
		long delta_x;
		long delta_y;
		int d_count;
		int offset;
		CvPoint before_move;
		CvPoint after_move;
	};
	Window_move WinMove;
	WinMove.delta_x = 0;
	WinMove.delta_y = 0;
	WinMove.d_count = 0;
	WinMove.offset = 0;

	while(1)
    {
        start=clock();  //����ð�����
		// feature points�� ��ȭ���� ����� ������ �̵��� ����
		x_ref -= (int)WinMove.delta_x;
		y_ref -= (int)WinMove.delta_y;

		if(x_ref > width - L_WIDTH)		x_ref = width - L_WIDTH -1;
		if(x_ref < 0)							x_ref = 0;
		if(y_ref > height - L_HEIGHT)		y_ref = height - L_HEIGHT -1;
		if(y_ref < 0)							y_ref = 0;
		
		src_color_R = cvQueryFrame( capture_R );
		cvCopy(src_color_R, result_view_R);
		cvSetImageROI(src_color_R,cvRect(x_ref,y_ref,L_WIDTH,L_HEIGHT));
		cvCvtColor(src_color_R, src_grey_R, CV_BGR2GRAY);
		cvResetImageROI(src_color_R);

		// feature points�� ��ȭ���� ����� ���ϱ� ���� �ʱ�ȭ
		WinMove.delta_x = 0;
		WinMove.delta_y = 0;
		WinMove.d_count = 0;

		if(pre_num_of_pts_R>0)
        {
            printf("opflow start\n");
            cvCalcOpticalFlowPyrLK( pre_src_grey_R, src_grey_R, prev_pyramid_R, pyramid_R, pre_feature_points_R, feature_points_R, num_of_pts_R, cvSize(WINSIZE,WINSIZE), 3, status_R, 0,cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), flags_R );
            flags_R |= CV_LKFLOW_PYR_A_READY;
            int valid_pt_R=0;  //valid�� ���� ���� count
            for(int i=0; i<num_of_pts_R; ++i)  //�� point�� valid ����
            {
                if(status_R[i]!=0)
                    ++valid_pt_R;
            }
   
            printf("valid_pts_R:%d\n",valid_pt_R);
            sprintf(s_output_result,"valid points : %d",valid_pt_R );    //�켱 sprintf�� ���ڿ� ����
            cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, 0.5, 0.5, 0, 1);  //�̷� ���� ����.
            cvPutText(result_view_R, s_output_result ,cvPoint(15,height-20),&font,cvScalar(0,255,0));   //cvPoint�� ���� ���� ��ġ ����(uv)
 
            //draw result, ȭ��ǥ ������� �׸��� ���� ������ �Ÿ��� 3�� ���̷� ȭ��ǥ �׷���
            for(int i=0; i<num_of_pts_R; i++)
            {     
                if ( status_R[i] == 0 )  //valid �� ���� �ƴϸ� �Ѿ
                    continue;
                int line_thickness = 1;
                CvScalar line_color = CV_RGB(255,0,0);
                CvPoint p,q;
				p.x = (int) pre_feature_points_R[i].x + (int)x_ref;
				p.y = (int) pre_feature_points_R[i].y + (int)y_ref;
				q.x = (int) feature_points_R[i].x + (int)x_ref;
				q.y = (int) feature_points_R[i].y + (int)y_ref;
                double angle; 
                angle = atan2( (double) p.y - q.y, (double) p.x - q.x );
                double arrow_length; 
                arrow_length = sqrt( pow((float)(p.y - q.y),2) + pow((float)(p.x - q.x),2) );
    
                q.x = (int) (p.x - 3 * arrow_length * cos(angle));  //3�� ���̷� ȭ��ǥ �׷���
                q.y = (int) (p.y - 3 * arrow_length * sin(angle));
       
                //������������ ȭ��ǥ�� �׸��� �ʴ´�.
                if((arrow_length<3)|(arrow_length>49)) //ȭ��ǥ ���̰� �ʹ�ª�ų� ��� �ȱ׸���.
                    continue;
                //draw arrow
                cvLine( result_view_R, p, q, line_color, line_thickness, CV_AA, 0 );
                p.x = (int) (q.x + 5 * cos(angle + PI / 4));
                if(p.x>=width)
                    p.x=width-1;
                else if(p.x<0)
                    p.x=0;
                p.y = (int) (q.y + 5 * sin(angle + PI / 4));
                if(p.y>=height)
                    p.y=height-1;
                else if(p.y<0)
                    p.y=0;
                cvLine( result_view_R, p, q, line_color, line_thickness, CV_AA, 0 );
                p.x = (int) (q.x + 5 * cos(angle - PI / 4));
                if(p.x>=width)
                    p.x=width-1;
                else if(p.x<0)
                    p.x=0;
                p.y = (int) (q.y + 5 * sin(angle - PI / 4));
                if(p.y>height)
                    p.y=height-1;
                else if(p.y<0)
                    p.y=0;
                cvLine( result_view_R, p, q, line_color, line_thickness, CV_AA, 0 );


				// ã���� ����(��ȿ��)�� �̵����� ��� ����: sum(delta feature points)
				WinMove.delta_x += (long)(pre_feature_points_R[i].x-feature_points_R[i].x);
				WinMove.delta_y += (long)(pre_feature_points_R[i].y-feature_points_R[i].y);
				WinMove.d_count++;
			
			}
						// �� ������ �̵����� ���: average(delta feature points)
			if( WinMove.d_count == 0 )	WinMove.d_count = 1;
			WinMove.delta_x = WinMove.delta_x/WinMove.d_count;
			WinMove.delta_y = WinMove.delta_y/WinMove.d_count;

			WinMove.after_move.x = x_ref+L_WIDTH/2 - WinMove.delta_x;
			WinMove.after_move.y = y_ref+L_WIDTH/2 - WinMove.delta_y;
			WinMove.before_move.x = x_ref+L_WIDTH/2;
			WinMove.before_move.y = y_ref+L_HEIGHT/2;
			
			cvLine( result_view_R, WinMove.before_move, WinMove.after_move, CV_RGB(0,255,255), 5, CV_AA, 0 );

			//
			sprintf(window_xy,"[%+3d , %+3d]:%3d",(int)(x_ref),(int)(y_ref),WinMove.d_count); // sprintf�� ���ڿ� ����
			cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, 0.5, 0.5, 0, 1); // Test setting
			cvPutText(result_view_R, window_xy ,cvPoint(x_ref,y_ref),&font,cvScalar(0,255,255));   //cvPoint�� ���� ���� ��ġ ����(uv)
//			if(WinMove.d_count == valid_pt_R )		cvPutText(result_view_R, window_xy ,cvPoint(x_ref,y_ref),&font,cvScalar(255,255,0));   //cvPoint�� ���� ���� ��ġ ����(uv)
//			else											cvPutText(result_view_R, window_xy ,cvPoint(x_ref,y_ref),&font,cvScalar(0,255,255));   //cvPoint�� ���� ���� ��ġ ����(uv)
		}
        //goodfeature to track
        num_of_pts_R = MAX_COUNT;
        cvGoodFeaturesToTrack( src_grey_R, eig_R, temp_R, feature_points_R, &num_of_pts_R, quality, min_distance, 0, WINSIZE, 0, 0.04 );
 
        cvFindCornerSubPix(src_grey_R, feature_points_R, num_of_pts_R, cvSize(WINSIZE,WINSIZE), cvSize(-1,-1), cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));   
 
        printf("max_count_R : %d\n",num_of_pts_R);
 
		// ������ ����(feature_points_R)�� ref���� ���Ͽ� ����ϸ� ȭ��ǥ�� ����� ��µ��� �ʾƼ� ����� ���� ������ ����
		CvPoint2D32f display_point;
        for( int i = 0 ; i < num_of_pts_R ; ++i){
			// ref�� ����� ��ġ
			display_point.x = feature_points_R[i].x+x_ref;
			display_point.y = feature_points_R[i].y+y_ref;
			// ȭ�� ��谡 �Ѿ�� �ʰ� ����
            if(display_point.x>=width)	display_point.x=width-1;
            else if(display_point.x<0)		display_point.x=1;
            if(display_point.y>=height)	display_point.y=height-1;
            else if(display_point.y<0)		display_point.y=1;
            if(display_point.x>=width)	display_point.x=width-1;
            else if(display_point.x<0)		display_point.x=1;
            if(display_point.y>height)	display_point.y=height-1;
            else if(display_point.y<0)		display_point.y=1;
			cvCircle( result_view_R, cvPointFrom32f(display_point), 1, CV_RGB(0,255,0), -1, 4,0);
		}
     
        //���� �� ó�� ���� ���ݿ��� ����
        cvCopy(src_grey_R, pre_src_grey_R);
        cvCopy(pyramid_R, prev_pyramid_R);
        //���� ã�� �� ��ǥ ����
        for(int i=0; i<num_of_pts_R; ++i)
            pre_feature_points_R[i] = feature_points_R[i];
        pre_num_of_pts_R = num_of_pts_R;
        finish=clock();
        duration=(double)(finish-start)/CLOCKS_PER_SEC;
        sprintf(s_output_result,"time : %f",duration );    //�켱 sprintf�� ���ڿ� ����
        cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, 0.5, 0.5, 0, 1);  //�̷� ���� ����.
        cvPutText(result_view_R, s_output_result ,cvPoint(width-150,height-20),&font,cvScalar(0,255,0));   //cvPoint�� ���� ���� ��ġ ����(uv)
		cvShowImage( "Optical Flow", result_view_R );
        pressed_key = cvWaitKey(30);
        if(pressed_key == 'q'){	break;}
		else if(pressed_key == 'a'){	x_ref += 50;}
		else if(pressed_key == 'z'){	x_ref -= 50;}
		else if(pressed_key == 's'){	y_ref += 50;}
		else if(pressed_key == 'x'){	y_ref -= 50;}
    }
    cvDestroyAllWindows();  
    cvReleaseCapture(&capture_R);
    if(src_grey_R)
        cvReleaseImage(&src_grey_R);
    if(pre_src_color_R)
        cvReleaseImage(&pre_src_color_R);
    if(result_view_R)
        cvReleaseImage(&result_view_R);
    //goodfeature to track 
    if(eig_R)
        cvReleaseImage( &eig_R );
    if(temp_R)
        cvReleaseImage( &temp_R );
    if(status_R)
        delete [] status_R;

}
*/