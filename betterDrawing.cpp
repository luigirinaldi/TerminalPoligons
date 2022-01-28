#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>

const char SYMBOLS[] = {'*','\\','/','|','_',' '}; 

struct Point{
	double x;
	double y;

	Point(){
		x = 0;
		y = 0;
	}

	Point(double in_x, double in_y){
		x = in_x;
		y = in_y;
	}

	bool operator==(const Point& p) const
	{
		return (x == p.x && y == p.y);
	}
};

// Overload ostream operator to print Point
std::ostream& operator<<(std::ostream& os, const Point& p) { 
	os << "(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")";
	return os;
}

struct Canvas{
	int width;
	int height;
	Point corner;
	// pointer to two dimensional arrays
	std::vector<std::vector<char>> canvas;
	
	Canvas(const int& w, const int& h, const Point c){
		// Width is twice the height to account for spacing in console
		width = 2*w;
		height = h;
		corner = c;

		//std::cout << "Initializing canvas with: " << height << " " << width << corner << std::endl;
		
		std::vector<char> tmp_arr;
		for(int y = 0; y <= height; y++){
			for(int x = 0; x <= width; x++){
				tmp_arr.push_back(' ');
			}
			canvas.push_back(tmp_arr);
			tmp_arr = std::vector<char>();
		}
	}

	void drawP(Point p, char symbol = '*'){
		// Width is twice the height to account for spacing in console
		int i = int(corner.y - p.y);
		int j = int(2 * (p.x - corner.x));
		canvas[i][j] = symbol;
	}

	void print(){
		for(int y = 0; y <= height; y++){
			for(int x = 0; x <= width; x++){
				std::cout << canvas[y][x];
			}
			std::cout << '\n';
		}
	}
};

struct Line{
	Point P0;
	Point P1;

	Line(Point p0, Point p1){
		P0 = p0;
		P1 = p1;
	}

	Line(){
		P0 = Point(0,0);
		P1 = Point(0,1);
	}

	char getSlopeChar(Point p1, Point p2){
		p1.x = int(p1.x);
		p1.y = int(p1.y);
		p2.x = int(p2.x);
		p2.y = int(p2.y);
		if(p1.x == p2.x) return '|';
		if(p1.y == p2.y) return '-';
		double slope = (p1.y - p2.y)/(p1.x - p2.y);
		if(slope > 0) return '/';
		else return '\\'; 
	}

	// function draws crude line on canvas
	void draw(Canvas& canvas){
		// how much the x changes as the calculations are made, 0.5 since for every y there are 2 x
		const double deltaX = 0.5;
		// Temp point variables in case they need to be swapped
		Point p1 = P1;
		Point p0 = P0;

		double dx = p1.x - p0.x;
		double dy = p1.y - p0.y;
		if(std::abs(dx) > std::abs(dy)){
			// Line is more horizontal than vertical
			// if P0 > P1 swap them
			if (p0.x > p1.x){
				Point tmp = p0;
				p0 = p1;
				p1 = tmp;
			}

			double y = p0.y;
			double slope = dy/dx;

			Point curr_p;
			Point next_p;

			for(double x = p0.x; x <= p1.x; x += deltaX){
				curr_p = Point(x,y);
				y = y + slope * deltaX;
				next_p = Point(x + deltaX,y);

				canvas.drawP(curr_p,getSlopeChar(curr_p,next_p));
			}
			std::cout << std::endl;
		} else {
			// Line is more vertical than horizontal
			// if P0 > P1 swap them
			if (p0.y > p1.y){
				Point tmp = p0;
				p0 = p1;
				p1 = tmp;
			}

			double x = p0.x;
			double slope = (dx/dy);

			
			std::cout << slope << std::endl;
			Point new_p;

			int change_x;
			char new_char;
			for(double y = p0.y; y <= p1.y; y+= 1){
				if(x != p0.x){
					change_x = int(x) - int(new_p.x);
					if(slope == 0){new_char = SYMBOLS[3];}
					else if(slope < 0){new_char = SYMBOLS[1];}
					else {new_char = SYMBOLS[2];}
				}
				new_p.x = x;
				new_p.y = y;				
				std::cout << new_p << " ";
				canvas.drawP(new_p, new_char);
				x = slope + x;
			}
			std::cout << std::endl;
		}
	}
};


struct Triangle{
	// initialize vector of size 3
	std::vector<Point> vertices = std::vector<Point>(3);
};

// Overload ostream operator to print Triangle
std::ostream& operator<<(std::ostream& os, const Triangle& t) { 
	for (int i = 0; i < t.vertices.size(); i++){
		os << "V" << i << ": " << t.vertices[i] << (i == 2 ? "" : "\n");
	}
	return os;
}

double distance_points(Point p1, Point p2){
	return std::sqrt(std::pow((p1.x - p2.x), 2) + std::pow((p1.y - p2.y), 2));
}

double triangle_perimeter(Triangle t){
	double perimeter = 0;
	for (int i = 0; i < t.vertices.size(); i++){
		perimeter += i == 2 ? distance_points(t.vertices[i], t.vertices[0]) : distance_points(t.vertices[i], t.vertices[i+1]);
	}
	return perimeter;
}


int main(){

	std::ifstream infile;
	infile.open("triangle.txt");

	if(!infile.is_open()){
		std::cout << "error opening file" << std::endl;
		return EXIT_FAILURE;
	}

	Triangle triangle;
	int point_counter = 0;

	while(infile >> triangle.vertices[point_counter].x >> triangle.vertices[point_counter].y){
		point_counter++;
		if (point_counter > 3){
			std::cout << "error, too many points" << std::endl;
			return EXIT_FAILURE;
		}
	}
	std::cout << triangle << std::endl;

	// Make canvas with size 20 and corner and -10, 10
	Canvas c = Canvas(20, 20, Point(-10, 10));

	// make triangle lines and add them to canvas
	for(int i = 0; i < 3; i++){
		Point P1 = triangle.vertices[i];
		Point P2 = triangle.vertices[i == 2 ? 0 : i+1];

		std::cout << P1 << P2 << std::endl;
		Line l = Line(P1,P2);
		l.draw(c);
	}
/*
	for(int i = 0; i < 3; i++){
		c.drawP(triangle.vertices[i],'$');
	}
*/
	//c.drawP(P0,'$');
	//c.drawP(P1,'$');

	// print canvas
	c.print();
 
}