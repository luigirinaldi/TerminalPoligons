#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>

struct Point{
    double x;
    double y;

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

void print_triangle(Triangle t){
	// Consts 
	const double INTERVAL = 2;
	const int OFFSET = 5;
	// point for vertex (254), full length horizontal bar (196), full length vertical bar |, 
	// back slash, forward slash
	const char SYMBOLS[] = {'*','\\','/','|','-',' '}; 
	// create new triangle by type casting original one
	Triangle t_draw;
	for(int i = 0; i < 3;i++){
		t_draw.vertices[i].x = (int)t.vertices[i].x;
		t_draw.vertices[i].y = (int)t.vertices[i].y;
	}	

/*
	// scale triangle up until perimeter is greater than 8
	while(triangle_perimeter(t_draw) < 8){
		for(int i = 0; i < 3; i++){
			t_draw.vertices[i].x = (int)(t.vertices[i].x * 10);
			t_draw.vertices[i].y = (int)(t.vertices[i].y * 10);
		}	
	}

	// scale triangle down until perimeter is less than 30
	while(triangle_perimeter(t_draw) > 50){
		for(int i = 0; i < 3; i++){
			t_draw.vertices[i].x = (int)(t.vertices[i].x / 10);
			t_draw.vertices[i].y = (int)(t.vertices[i].y / 10);
		}	
	}
*/
	std::cout << "New triangle: \n" << t_draw << std::endl;

	// sett mins and maxes to extreme values
	/*
	int min_x = std::numeric_limits<int>::max(), min_y = std::numeric_limits<int>::max();
	int max_x = std::numeric_limits<int>::min(), max_y = std::numeric_limits<int>::min();
	*/

	int min_x = 10000, min_y = 10000;
	int max_x = -10000, max_y = -10000;


	int x = 0, y = 0;

	for(int i = 0; i < 3;i++){
		x = t_draw.vertices[i].x;
		y = t_draw.vertices[i].y;

		if(x < min_x){
			min_x = x;
		} 
		if(x > max_x){
			max_x = x;
		}
		if(y < min_y){
			min_y = y;
		} 
		if(y > max_y){
			max_y = y;
		}
	}

	double coeffs[3][5];
	double dx = 0, dy = 0;

	// find equations for the lines of the triangle with following formula:
	// (y1 - y2)x -(x1 - x2)y + (x1 - x2)y1 - (y1 - y2)x1 = 0 
	for(int i = 0; i < 3; i++){
		Point P1 = t_draw.vertices[i];
		Point P2 = t_draw.vertices[i == 2 ? 0 : i+1];
		// calculate coefficients
		dx = P1.x - P2.x;
		dy = P1.y - P2.y;
		coeffs[i][0] = dy;
		coeffs[i][1] = -1 * dx;
		coeffs[i][2] = dx * P1.y - dy * P1.x;
		coeffs[i][3] = dx;
		coeffs[i][4] = dy;
		
		// std::cout << i << " " << P1 << " " << P2 << coeffs[i][0] << " " << coeffs[i][1] << " " << coeffs[i][2] << std::endl;
	}

	//std::cout << min_x << max_x << min_y << max_y << std::endl;

	// define the boundries of the box to draw, adding extra padding on the sides
	// to get x and y coords add min - offset
	// x (j) is double its' normal value since the scaling of the lines is about 2:1 
	// meaning one vertical line is about 2 horizontal ones
	int i_max = ((max_y - min_y) + 2 * OFFSET);
	int j_max = 2*((max_x - min_x) + 2 * OFFSET);

	//std::cout << i_max << " " << j_max << std::endl;

	// matrix of points with true values for points to be printed
	char draw_points[i_max][j_max];

	for(int i = 0; i < i_max; i++){
		for(int j = 0; j < j_max; j++){
			Point p;
			p.x = (double(j)/2) + (min_x - OFFSET) - 1;
			p.y = (-i) + (max_y + OFFSET) - 1;

			//check point is withing triangle area
			if((p.x <= max_x) && (p.x >= min_x) && (p.y <= max_y) && (p.y >= min_y)){
				//check if point is a vertex
				bool found = false;
				for(int k = 0; (k < 3) && !found ;k++){
					found = p == t_draw.vertices[k];
				}
				// if the point is a vertex
				if(found) {draw_points[i][j] = '*';}
				else {
					bool line = false;
					double val = 0;

					for(int k = 0; (k < 3) && !line ;k++){
						val = coeffs[k][0] * p.x + coeffs[k][1] * p.y + coeffs[k][2];
						
						line = val <= INTERVAL && val >= -INTERVAL;
						if(line){
							if(coeffs[k][3] == 0){
								// dx is 0, print vertical line
								draw_points[i][j] = '|';
							} else if(coeffs[k][4] == 0){
								// dy is 0, print horizontal 
								draw_points[i][j] = '-';
							} else if((coeffs[k][4]/coeffs[k][3]) > 0){
								// print forward slash
								draw_points[i][j] = '/';
							} else {
								draw_points[i][j] = '\\';
							}
						} else {
							draw_points[i][j] = ' ';
						}
					}
				}
			} else {
				draw_points[i][j] = ' ';
			}
			//std::cout << p << " " << val << std::endl;
		}
	}

/*
	for(int i = 0; i < i_max; i++){
		for(int j = 0; j < j_max; j++){
			std:: cout << draw_points[i][j] << " ";
		}
		std::cout << "\n";
	}
	*/

	// draw box with vertices inside
	// using box drawing characters: https://en.wikipedia.org/wiki/Box-drawing_character
	// had to use ascii https://theasciicode.com.ar/extended-ascii-code/box-drawing-character-single-line-lower-left-corner-ascii-code-192.html
	// i is y and j is x	

	Point p;
	for(int i = 0; i < i_max; i++){
		for(int j = 0; j < j_max; j++){
			// counter to point conversion			
			p.x = (double(j)/2) + (min_x - OFFSET);
			p.y = (-i) + (max_y + OFFSET);

			// first row, print corners and line 
			/*if(p.y == max_y + OFFSET)
			{
				if(p.x == min_x - OFFSET) {std::cout << char(201);}  			//╔
				else if(p.x == max_x + (OFFSET - 1)) {std::cout << char(187);}		//╗
				else {std::cout <<  char(205);}					//"═"
			} 
			else if(p.y == min_y - (OFFSET - 1))
			{
				if(p.x == min_x - OFFSET) {std::cout << char(200);}  			//"╚"
				else if(p.x == max_x + (OFFSET - 1)) {std::cout << char(188);} 	//"╝"
				else {std::cout <<  char(205);}					//"═"
			}
			else if(p.x == min_x - OFFSET || p.x == max_x + (OFFSET )) {std::cout << char(186);} //"║"
			*/
			
			//else {		
				std::cout << draw_points[i][j];
			
			//}

		}
		std::cout << std::endl;
	}
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
	
	print_triangle(triangle);
 
}