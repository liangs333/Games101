#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    int len = control_points.size();
    if(len != 1) { 
        std::vector<cv::Point2f> nextStageCP;
        for(int i = 0; i < len - 1; ++i) { 
            auto divPoint = control_points[i] + t * (control_points[i + 1] - control_points[i]);
            nextStageCP.push_back(divPoint);
        } 
        return recursive_bezier(nextStageCP, t);
    } 
    else { 
        return control_points[0];
    } 
}

float getDist(float x1, float y1, float x2, float y2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.

    // for(double t = 0.0; t <= 1.0; t += 0.001) { 
    //     auto point = recursive_bezier(control_points, t);
    //     window.at<cv::Vec3b>(point.y, point.x)[1] = 255;
    //     //这里实质上是做了个强制转换————把point(x, y)舍去小数部分到int，然后对int位子的pixel进行着色
    //     //而这个着色是二分类的，0 / 255这样
    // } 

    for(double t = 0.0; t <= 1.0; t += 0.002) { 
        auto point = recursive_bezier(control_points, t);
        int xmi = 0, ymi = 0;
        //和该坐标点相邻的四像素中的xmi 和 ymi
        //显然是(xmi, ymi), (xmi, ymi+1), (xmi+1, ymi), (xmi+1, ymi+1)这四个玩意儿
        if(point.x - (int)point.x < 0.5)
            xmi = (int)point.x - 1;
        else  
            xmi = (int)point.x;
        
        if(point.y - (int)point.y < 0.5)
            ymi = (int)point.y - 1;
        else   
            ymi = (int)point.y;
        int dx[] = {0, 0, 0, -1, -1, -1, 1, 1, 1};
        int dy[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
        for(int k = 0; k < 9; ++k) {
            float fx = xmi + dx[k] + 0.5f, fy = ymi + dy[k] + 0.5f;
            float nDist = getDist(fx, fy, point.x, point.y);
            //实则距离平方，感觉可以直接抓过来当系数，很河狸吧
            nDist = 8 - nDist;
            nDist /= 8;
            nDist = pow(nDist, 2);
            if(k == 1)
                window.at<cv::Vec3b>(ymi + dy[k], xmi + dx[k])[1] = 255;
            else
                window.at<cv::Vec3b>(ymi + dy[k], xmi + dx[k])[1] = std::max(
                    (float)(window.at<cv::Vec3b>(ymi + dy[k], xmi + dx[k])[1]),
                    (float)255 * nDist
                );
            printf("%f %f %f %f      %f         ", fx, fy, point.x, point.y, nDist);
            printf("%f  \n", (float)(window.at<cv::Vec3b>(ymi + dy[k], xmi + dx[k])[1]));
        }
    } 
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
//            naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
