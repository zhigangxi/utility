#pragma once
#include <cmath>
struct vector3d 
{
	vector3d()
	{
		x_ = 0.f;
		y_ = 0.f;
		z_ = 0.f;
	}
	vector3d(float x,float y,float z)
		:x_(x),y_(y),z_(z)
	{}
	float x_;
	float y_;
	float z_;
};
struct Point 
{
	float x_,y_;
	Point():x_(0),y_(0){}
	Point(float x,float y):x_(x),y_(y){}
	Point(const Point& p){ x_ = p.x_; y_ = p.y_; }
	Point& operator = (const Point& p){ x_ = p.x_; y_ = p.y_; return *this;}
};
struct Rectangle
{
	float left_;
	float top_;
	float right_;
	float bottom_;
	Rectangle(){}
	Rectangle(float l,float t,float r,float b)
	{left_ = l;top_ = t;right_ = r;bottom_ = b;}
	Rectangle(const Rectangle& other){ left_ = other.left_; top_ = other.top_; right_ = other.right_; bottom_ = other.bottom_;}
	Rectangle& operator = (const Rectangle& other){ left_ = other.left_; top_ = other.top_; right_ = other.right_; bottom_ = other.bottom_;return *this;}
};

struct Size 
{
	float w_;
	float h_;
	Size():w_(0.0),h_(0.0){}
	Size(float w,float h):w_(w),h_(h){}
};

struct Circle 
{
	Point center_;
	float radius_;
	Circle():radius_(0.0){}
};
