
#include <sstream>
#include <string>
#include <iostream>
#include <opencv\highgui.h>
#include <opencv\cv.h>

using namespace cv;
using namespace std;
//valores do hsv pra calibrar/selcionar os objetos com determinada cor
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS=50;
int pontos = 0;
int xBola = 0;
int yBola = 0;
int xCesta = 0;
int yCesta = 0;
bool contador = false;

//minimum and maximum object area
const int MIN_OBJECT_AREA = 20*20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH/1.5;
//nome das janelas
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string windowName4 = "Imagem original";
const string trackbarWindowName = "Trackbars";
void on_trackbar( int, void* )
{

}
string intToString(int number){


    std::stringstream ss;
    ss << number;
    return ss.str();
}
void createTrackbars(){
    //criar as trackbars pra poder detectar os objetos na calibration


    namedWindow(trackbarWindowName,0);

    char TrackbarName[50];
    sprintf( TrackbarName, "H_MIN", H_MIN);
    sprintf( TrackbarName, "H_MAX", H_MAX);
    sprintf( TrackbarName, "S_MIN", S_MIN);
    sprintf( TrackbarName, "S_MAX", S_MAX);
    sprintf( TrackbarName, "V_MIN", V_MIN);
    sprintf( TrackbarName, "V_MAX", V_MAX);


    createTrackbar( "H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar );
    createTrackbar( "H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar );
    createTrackbar( "S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar );
    createTrackbar( "S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar );
    createTrackbar( "V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar );
    createTrackbar( "V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar );


}
//desenhar as formas na imagem
void drawObject(int x, int y,Mat &frame, int tipo){

    if(tipo==1){

    circle(frame,Point(x,y),20,Scalar(0,255,0),2);
    if(y-25>0)
    line(frame,Point(x,y),Point(x,y-25),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(x,0),Scalar(0,255,0),2);
    if(y+25<FRAME_HEIGHT)
    line(frame,Point(x,y),Point(x,y+25),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(x,FRAME_HEIGHT),Scalar(0,255,0),2);
    if(x-25>0)
    line(frame,Point(x,y),Point(x-25,y),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(0,y),Scalar(0,255,0),2);
    if(x+25<FRAME_WIDTH)
    line(frame,Point(x,y),Point(x+25,y),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(FRAME_WIDTH,y),Scalar(0,255,0),2);
    circle(frame,Point(x,y),2,Scalar(255,0,255),2);

    }else if(tipo==2){
        line(frame,Point(x,y),Point(x-20,y),Scalar(255,0,0),3);
        line(frame,Point(x,y),Point(x+30,y),Scalar(255,0,0),3);
        line(frame,Point(x,y-2),Point(x-20,y-2),Scalar(255,0,0),3);
        line(frame,Point(x,y-2),Point(x+30,y-2),Scalar(255,0,0),3);
         circle(frame,Point(x+10,y),2,Scalar(255,255,0),2);

        //line(frame,Point(x,y),Point(x,y-25),Scalar(0,255,255),3);
    }

    //putText(frame,intToString(x)+","+intToString(y),Point(x,y+30),1,1,Scalar(0,255,0),2);

}
//aplicar morfologia
void morphOps(Mat &thresh){

    //create structuring element that will be used to "dilate" and "erode" image.
    //the element chosen here is a 3px by 3px rectangle

    Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
    //dilate with larger element so make sure object is nicely visible
    Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

    erode(thresh,thresh,erodeElement);
    erode(thresh,thresh,erodeElement);


    dilate(thresh,thresh,dilateElement);
    dilate(thresh,thresh,dilateElement);



}
void trackFilteredObject(int x, int y, Mat threshold, Mat &cameraFeed, int tipo){

    Mat temp;
     //Point a;
    threshold.copyTo(temp);

    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;

    //find contours of filtered image using openCV findContours function
    findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
    //use moments method to find our filtered object
    double refArea = 0;
    bool objectFound = false;
    if (hierarchy.size() > 0) {

        int numObjects = hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if(numObjects<MAX_NUM_OBJECTS){
            for (int index = 0; index >= 0; index = hierarchy[index][0]) {

                Moments moment = moments((cv::Mat)contours[index]);
                double area = moment.m00;
                /* teste
                if(tipo ==1){
                    std::ostringstream strs;
                    strs << area;
                    std::string str = strs.str();

                     putText(cameraFeed,"Area bola: "+str,Point(20,20),1,1,Scalar(255,0,0),1);
                }else{
                    std::ostringstream strs;
                    strs << area;
                    std::string str = strs.str();
                     putText(cameraFeed,"Area cesta: "+str,Point(20,50),1,1,Scalar(255,0,0),1);
                }*/



                //if the area is less than 20 px by 20px then it is probably just noise
                //if the area is the same as the 3/2 of the image size, probably just a bad filter
                //we only want the object with the largest area so we safe a reference area each
                //iteration and compare it to the area in the next iteration.
                if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea){
                    x = moment.m10/area;
                    y = moment.m01/area;
                    if(tipo==1){
                        xBola= x;
                        yBola=y;

                    }else if(tipo==2){
                        xCesta = x;
                        yCesta = y;
                    }

                    objectFound = true;
                    refArea = area;
                }else objectFound = false;


            }
            //let user know you found an object
            if(objectFound ==true){
                //putText(cameraFeed,"Tracking Object",Point(0,50),2,1,Scalar(0,255,0),2);
                //draw object location on screen
                drawObject(x,y,cameraFeed,tipo);}

        }else putText(cameraFeed,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);

    }

//return a;

}


    float distancia(Point p, Point q) {
        Point diff = p - q;
        return cv::sqrt(diff.x*diff.x + diff.y*diff.y);
    }




int main(int argc, char* argv[])
{
    //some boolean variables for different functionality within this
    //program
    bool trackObjects = false;
    bool useMorphOps = false;
    bool calibrationMode = false;

    //matrizes
    Mat cameraFeed;
    Mat HSV;
    Mat threshold;
    Mat Img;

    if(calibrationMode){
        //create slider bars for HSV filtering
        createTrackbars();

    }

    //x and y values for the location of the object
    int x=0, y=0, xCes=0, yCes=0;

    //video capture
    VideoCapture capture;
    //open capture object at location zero (default location for webcam)
    capture.open(0);
    //set height and width of capture frame
    capture.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);

    while(1){



        //capture.read(cameraFeed);
        if (!capture.read(cameraFeed)) {
            capture.open("teste_4.avi");
            capture.read(cameraFeed);
               std::cout << "Fim do video." << std::endl;

        }
        putText(cameraFeed,"Pontos: "+intToString(pontos),Point(20,40),1,1,Scalar(255,0,0),2);

        cameraFeed.copyTo(Img);

        //modo para calibrar os valores do objeto desejado
        if(calibrationMode==true){

            cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
            inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
            morphOps(threshold);
            imshow(windowName2,threshold);
            imshow(windowName1,HSV);




        }else{
            //detectar a bola
            cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
            //colocar os valores hsv pra bola aki. hsv min / hsv max
            inRange(HSV,Scalar(139,96,0),Scalar(244,256,256),threshold);
            morphOps(threshold);
            trackFilteredObject(x,y,threshold,cameraFeed, 1);

            putText(cameraFeed,"Pos bola: "+intToString(xBola)+","+intToString(yBola),Point(20,60),1,1,Scalar(255,0,0),2);

            //detectar a cesta
            cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
            //colocar os valores hsv pra cesta aki. hsv min / hsv max 83,15,41-250,134,256

            //createTrackbars(); //pra teste
            inRange(HSV,Scalar(83,15,41),Scalar(168,134,256),threshold);
            morphOps(threshold);
            //imshow(windowName2,threshold);// pra teste
            trackFilteredObject(x,y,threshold,cameraFeed, 2);

            putText(cameraFeed,"Pos Cesta: "+intToString(xCesta)+","+intToString(yCesta),Point(20,80),1,1,Scalar(255,0,0),2);

            Point a(xBola, yBola);
            Point b(xCesta+10, yCesta);
            double dt = distancia(a, b);



            if((xBola!=0&&yBola!=0)&&(xCesta!=0&&yCesta!=0)){
                if(dt<=35){

                    putText(cameraFeed,"Objetos muito Proximos: ",Point(60,200),1,1,Scalar(255,0,0),2);
                    if(!contador){
                       pontos++;
                       contador = true;
                    }

                }else{
                    contador = false;
                }

            }


        }

        imshow(windowName,cameraFeed);
        imshow(windowName4,Img);


        waitKey(30);
    }

    return 0;

}
