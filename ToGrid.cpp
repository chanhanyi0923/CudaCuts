#include <vector>
#include <limits>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


class Point
{
public:
	double x, y;
	Point() : x(0), y(0) {}
	Point(double x, double y) : x(x), y(y) {}
};


class Grid
{
public:
	bool is_s, is_t;
	size_t left, right, up, down;
	//double left_w, right_w, up_w, down_w; // weights
	Grid() : left(0), right(0), up(0), down(0),
		//left_w(std::numeric_limits<double>::max()),
		//right_w(std::numeric_limits<double>::max()),
		//up_w(std::numeric_limits<double>::max()),
		//down_w(std::numeric_limits<double>::max()),
		is_s(false), is_t(false) {}
};


const char * nodeFile = "data/Nodes.csv";
const char * tripFile = "data/simpleOD4.csv";
const size_t POINT_SIZE = 105000;
const size_t TRIPS_SIZE = 1000;
const double GRID_WIDTH = 0.001;
const size_t GRID_SIZE = 1000;

double x_min, x_max, y_min, y_max;
Point points[POINT_SIZE];
size_t trips_size = 0;
std::vector<size_t> trips[TRIPS_SIZE];
Grid grid[GRID_SIZE][GRID_SIZE];

void ReadNodes()
{
	using std::fstream;

	fstream fin;
	fin.open(nodeFile, fstream::in);

	int index;
	double x, y;
	std::string str, str2, str3;
	while (std::getline(fin, str)) {
		std::stringstream ss(str);
		{
			std::getline(ss, str2, ',');
			std::stringstream ss2(str2);
			ss2 >> index;
			index--;
		}
		{
			std::getline(ss, str2, ',');
			std::stringstream ss2(str2);
			ss2 >> x;
		}
		{
			std::getline(ss, str2, ',');
			std::stringstream ss2(str2);
			ss2 >> y;
		}
		points[index] = Point(x, y);
	}
	fin.close();
}


void ReadTrips()
{
	using std::fstream;

	fstream fin;
	fin.open(tripFile, fstream::in);

	std::string buffer, buffer2;

	trips_size = 0; // count of trips

	while (std::getline(fin, buffer)) {
		std::stringstream ss(buffer);
		for (int num = 0; ss >> num; ) {

			trips[trips_size].push_back(num - 1);
			Point p = points[num - 1];

			if (p.x < 5 || p.y < 5) {
				throw "input trip error!";
				//std::cout << num - 1 << std::endl;
			}

			x_min = std::min(x_min, p.x);
			y_min = std::min(y_min, p.y);
			x_max = std::max(x_max, p.x);
			y_max = std::max(y_max, p.y);
			num = 0;
		}
		trips_size ++;
	}

	fin.close();

}


size_t getGridX(double x)
{
	return floor( (x - x_min + 0.5 * GRID_WIDTH) / GRID_WIDTH );
}


size_t getGridY(double y)
{
	return floor( (y - y_min + 0.5 * GRID_WIDTH) / GRID_WIDTH );
}


std::vector< std::pair<int, int> > drawline(int x0, int y0, int x1, int y1)
{
	std::vector< std::pair<int, int> > result;
	bool reverse = false, swap_xy = false;
	if (std::abs(y1 - y0) > std::abs(x1 - x0)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		swap_xy = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
		reverse = true;
	}


	int dx, dy, p, x, y;

	dx = x1 - x0;
	dy = std::abs(y1 - y0);

	x = x0;
	y = y0;

	p = dx;
	int ystep = y1 > y0 ? 1 : -1;

	while (x <= x1) {
		result.push_back(swap_xy ? std::make_pair(y, x) : std::make_pair(x, y));
		p = p - 2 * dy;
		if (p < 0) {
			y = y + ystep;
			p = p + 2 * dx;
		}
		x = x + 1;
	}

	if (reverse) {
		std::reverse(result.begin(), result.end());
	}

	return result;
}


int weightToInteger(double value)
{
	const int multiply = 10000;
	return value * multiply;
}


double ww(int v)
{
	if (v == 0) return 0;
	return 1.0 / v;
}



int main()
{
	x_min = y_min = std::numeric_limits<double>::max();
	x_max = y_max = std::numeric_limits<double>::min();
	ReadNodes();
	ReadTrips();

	//std::cout << x_min << " " << x_max << std::endl;
	//std::cout << y_min << " " << y_max << std::endl;


	size_t x_num = 1 + ceil( (x_max - x_min + GRID_WIDTH) / GRID_WIDTH );
	size_t y_num = 1 + ceil( (y_max - y_min + GRID_WIDTH) / GRID_WIDTH );

	std::cout << x_num << " " << y_num << std::endl;


	for (size_t _ = 0; _ < trips_size; _++) {
		const auto & trip = trips[_];
		for (size_t i = 1; i < trip.size(); i++) {
			const auto & pa = points[trip[i - 1]];
			const auto & pb = points[trip[i]];
			const size_t pa_x_id = getGridX(pa.x);
			const size_t pa_y_id = getGridY(pa.y);
			const size_t pb_x_id = getGridX(pb.x);
			const size_t pb_y_id = getGridY(pb.y);

			// set od
			if (i == 1) {
				grid[pa_x_id][pa_y_id].is_s = true;
			}
			if (i == trip.size() - 1) {
				grid[pb_x_id][pb_y_id].is_t = true;
			}
			//

			if (pa_x_id == pb_x_id && pa_y_id == pb_y_id) {
				// same position
				continue;
			} else {
				std::vector< std::pair<int, int> > linePoints;
				if (pa_x_id == pb_x_id) {
					int ystep = pa_y_id < pb_y_id ? 1 : -1;
					for (int i = pa_y_id; i != pb_y_id + ystep; i += ystep) {
						linePoints.push_back(std::make_pair(pa_x_id, i));
					}
				} else if (pa_y_id == pb_y_id) {
					int xstep = pa_x_id < pb_x_id ? 1 : -1;
					for (int i = pa_x_id; i != pb_x_id + xstep; i += xstep) {
						linePoints.push_back(std::make_pair(i, pa_y_id));
					}
				} else {
					linePoints = drawline(pa_x_id, pa_y_id, pb_x_id, pb_y_id);
				}
				for (size_t i = 1; i < linePoints.size(); i++) {
					int x0 = linePoints[i - 1].first;
					int y0 = linePoints[i - 1].second;
					int x1 = linePoints[i].first;
					int y1 = linePoints[i].second;
					Grid & g = grid[x0][y0];
					//
					if (x0 == x1) {
						if (y0 == y1) {
							throw "x0 == x1 && y0 == y1 when drawing line";
						}
						else if (y0 < y1) {
							g.up++;
							//g.up_w = 1.0 / g.up;
						}
						else if (y0 > y1) {
							g.down++;
							//g.down_w = 1.0 / g.down;
						}
					} else if (x0 < x1) {
						// y0 == y1
						g.right++;
						//g.right_w = 1.0 / g.right;

						Grid & g2 = grid[x0 + 1][y0];

						if (y0 < y1) {
							g2.up++;
							//g2.up_w = 1.0 / g2.up;
						}
						else if (y0 > y1) {
							g2.down++;
							//g2.down_w = 1.0 / g2.down;
						}
					} else if (x0 > x1) {
						// y0 == y1
						g.left++;
						//g.left_w = 1.0 / g.left;
						Grid & g2 = grid[x0 - 1][y0];

						if (y0 < y1) {
							g2.up++;
							//g2.up_w = 1.0 / g2.up;
						}
						else if (y0 > y1) {
							g2.down++;
							//g2.down_w = 1.0 / g2.down;
						}
					}
				}
			}
		}
	}

	std::fstream fout;
	fout.open("data/sample4.txt", std::fstream::out);


	for (int i = 0; i < x_num; i++) {
		for (int j = 0; j < y_num; j++) {
			if (grid[i][j].is_s && grid[i][j].is_t) {
				std::cout << i << " " << j << std::endl;
				throw "a grid is s and t";
			}
			//fout << "0 ";
		}
		//fout << std::endl;
	}
	//fout << std::endl;

	//fout << y_num << " " << x_num << " 2" << std::endl;


	fout << x_num << " " << y_num << std::endl;
	
	int cnt = 0;
	for (int i = 0; i < x_num; i++) {
		for (int j = 0; j < y_num; j++) {
			//fout << (cnt ++) << ":  ";
			//if (grid[i][j].is_s) fout << "s: " << i << " - " << j << " ";// std::endl;
			//if (grid[i][j].is_t) fout << "t: " << i << " - " << j << " ";// std::endl;
			fout << weightToInteger(grid[i][j].is_s ? 10 : 0) << " ";
			fout << weightToInteger(grid[i][j].is_t ? 10 : 0) << " ";
			fout << weightToInteger(ww(grid[i][j].left)) << " ";
			fout << weightToInteger(ww(grid[i][j].right)) << " ";
			fout << weightToInteger(ww(grid[i][j].up)) << " ";
			fout << weightToInteger(ww(grid[i][j].down)) << std::endl;
		}
		//fout << std::endl;
	}
	fout.close();
	//while (1);

	// for debug html
//#define DEBUG_HTML
#ifdef DEBUG_HTML
	std::cout << "<table>";
	for (int i = 0; i < x_num; i++) {
		std::cout << "<tr>";
		for (int j = 0; j < y_num; j++) {
			int v = grid[i][j].left + grid[i][j].right + grid[i][j].up + grid[i][j].down;
			//bool cond = (i == 14 && j == 25) || (i == 15 && j == 4);
			//bool cond = grid[i][j].is_s;
			if (grid[i][j].is_s) {
				std::cout << "<td class=\"red\">&nbsp</td>";
			} else if (grid[i][j].is_t) {
					std::cout << "<td class=\"green\">&nbsp</td>";
			} else if (v > 0) {
				std::cout << "<td class=\"blue\">&nbsp</td>";
			}
			else {
				std::cout << "<td class=\"white\">&nbsp</td>";
			}
		}
		std::cout << "</tr>";
	}
	std::cout << "</table>" << std::endl;
	while (1);
#endif
	//for (int i = 0; i < x_num; i++) {
	//	for (int j = 0; j < y_num; j++) {
	//		fout << y_num << " " << x_num << " 2" << std::endl;
	//	}
	//	fout << std::endl;
	//}
	//fout << std::endl;

	//for (int i = 0; i < x_num; i++) {
	//	for (int j = 0; j < y_num; j++) {
	//		fout << weightToInteger(grid[i][j].is_t ? 1 : 0) << " ";
	//	}
	//	fout << std::endl;
	//}
	//fout << std::endl;

	//for (int i = 0; i < x_num; i++) {
	//	for (int j = 0; j < y_num - 1; j++) {
	//		fout << weightToInteger( 1.0 / (grid[i][j].right + grid[i][j + 1].left) ) << " ";
	//	}
	//	fout << std::endl;
	//}
	//fout << std::endl;

	//for (int i = 0; i < x_num - 1; i++) {
	//	for (int j = 0; j < y_num; j++) {
	//		fout << weightToInteger(1.0 / (grid[i][j].down + grid[i + 1][j].up) ) << " ";
	//	}
	//	fout << std::endl;
	//}
	//fout << std::endl;


	//while (1);
	return 0;
}