#include <fstream>
#include <algorithm>
#include <iostream>
#include <limits>
//#include <queue>


class Data
{
private:
	bool active;
	size_t rowSize, columnSize;
	int *weight; // size = 4 * (number of grids)
	int *s_weight, *t_weight;
	int *height, *capacity; // size = number of grids

	int flow;
	int *bfsTag;

	void PushFromS(int x, int y);
	void PushToT(int x, int y);
	void PushLeft(int x, int y);
	void PushRight(int x, int y);
	void PushUp(int x, int y);
	void PushDown(int x, int y);
	void Push(size_t indexFrom, size_t indexTo, size_t weightIndexFrom, size_t weightIndexTo);
	//void Relabel(int x, int y);

#define DEBUG
#ifdef DEBUG
	void Print();
#endif
	//void GlobalRelabel();
public:

	Data();
	Data(size_t rowSize, size_t columnSize);
	void Read(const char * filename);
	int GetFlow();
	//void BfsFromS();
	void BfsFromT();
};


Data::Data():
	weight(nullptr), height(nullptr), capacity(nullptr), s_weight(nullptr), t_weight(nullptr),
	rowSize(0), columnSize(0), flow(0)
{
}


Data::Data(size_t rowSize, size_t columnSize) :
	rowSize(rowSize), columnSize(columnSize), flow(0)
{
	weight = new int[4 * rowSize * columnSize](); // (): set to zero
	s_weight = new int[rowSize * columnSize]();
	t_weight = new int[rowSize * columnSize]();
	height = new int[rowSize * columnSize]();
	capacity = new int[rowSize * columnSize]();
}


void Data::Read(const char * filename)
{
	using std::fstream;
	fstream fin;
	fin.open(filename, fstream::in);
	fin >> this->rowSize >> this->columnSize;

	this->weight = new int[4 * this->rowSize * this->columnSize](); // (): set to zero
	this->s_weight = new int[this->rowSize * this->columnSize]();
	this->t_weight = new int[this->rowSize * this->columnSize]();
	this->height = new int[this->rowSize * this->columnSize]();
	this->capacity = new int[this->rowSize * this->columnSize]();
	this->bfsTag = new int[this->rowSize * this->columnSize]();

	for (int i = 0; i < this->rowSize * this->columnSize; i++) {
		// order: s, t, left, right, up, down
		fin >> this->s_weight[i] >> this->t_weight[i];
		fin >> this->weight[i * 4] >> this->weight[i * 4 + 1] >> this->weight[i * 4 + 2] >> this->weight[i * 4 + 3];
	}
	fin.close();
}


void Data::Print()
{
	std::cout << "Weight:" << std::endl;
	for (int i = 0; i < this->rowSize; i++) {
		for (int j = 0; j < this->columnSize; j++) {
			size_t idx = (i * this->columnSize + j) * 4;
			std::cout << this->weight[idx] << " ";
			std::cout << this->weight[idx + 1] << " ";
			std::cout << this->weight[idx + 2] << " ";
			std::cout << this->weight[idx + 3] << std::endl;
		}
	}
	std::cout << "Capacity:" << std::endl;
	for (int i = 0; i < this->rowSize; i++) {
		for (int j = 0; j < this->columnSize; j++) {
			size_t idx = (i * this->columnSize + j);
			std::cout << this->capacity[idx] << std::endl;
		}
	}
	std::cout << "Height:" << std::endl;
	for (int i = 0; i < this->rowSize; i++) {
		for (int j = 0; j < this->columnSize; j++) {
			size_t idx = (i * this->columnSize + j);
			std::cout << this->height[idx] << std::endl;
		}
	}
	std::cout << std::endl;
}


int Data::GetFlow()
{
	int count = 0;// debug


	this->active = true;
	while (this->active) {
		//std::cout << (++ count) << std::endl;

		//this->Print();
		//for (int i = 0; i < this->rowSize; i++) {
		//	for (int j = 0; j < this->columnSize; j++) {
		//		PushFromS(i, j);
		//	}
		//}
		this->active = false;

		for (int i = 0; i < this->rowSize; i++) {
			for (int j = 0; j < this->columnSize; j++) {
				PushFromS(i, j);
				PushLeft(i, j);
				PushToT(i, j);
			}
		}

		//this->Print();

		for (int i = 0; i < this->rowSize; i++) {
			for (int j = 0; j < this->columnSize; j++) {
				PushFromS(i, j);
				PushUp(i, j);
				//std::cout << i << " " << j << std::endl;
				//Print();
				PushToT(i, j);
			}
		}

		//this->Print();

		for (int i = 0; i < this->rowSize; i++) {
			for (int j = 0; j < this->columnSize; j++) {
				PushFromS(i, j);
				PushRight(i, j);
				PushToT(i, j);
			}
		}

		//this->Print();

		for (int i = 0; i < this->rowSize; i++) {
			for (int j = 0; j < this->columnSize; j++) {
				PushFromS(i, j);
				PushDown(i, j);
				PushToT(i, j);
			}
		}

		//this->Print();
	}

	return this->flow;
}


void Data::PushFromS(int x, int y)
{
	const size_t index = x * this->columnSize + y;
	if (this->s_weight[index] > 0) {
		this->height[index] = 1;

		//std::cout << x << " " << y << std::endl;
	}
	this->capacity[index] += this->s_weight[index];
	//this->s_weight[index] = 0;
}


void Data::PushToT(int x, int y)
{
	const size_t index = x * this->columnSize + y;
	int value = std::min(this->capacity[index], this->t_weight[index]);

	//if (value > 0) std::cout << x << " " << y << std::endl;

	this->capacity[index] -= value;
	//this->t_weight[index] -= value;

	this->flow += value;
}


void Data::PushLeft(int x, int y)
{
	if (x == 0) {
		return;
	}

	const size_t indexFrom = x * this->columnSize + y;
	const size_t indexTo = (x - 1) * this->columnSize + y;
	const size_t weightIndexFrom = indexFrom * 4; // (x, y).left
	const size_t weightIndexTo = indexTo * 4 + 1; // (x - 1, y).right
	this->Push(indexFrom, indexTo, weightIndexFrom, weightIndexTo);
}


void Data::PushRight(int x, int y)
{
	if (x == this->rowSize - 1) {
		return;
	}

	const size_t indexFrom = x * this->columnSize + y;
	const size_t indexTo = (x + 1) * this->columnSize + y;
	const size_t weightIndexFrom = indexFrom * 4 + 1; // (x, y).right
	const size_t weightIndexTo = indexTo * 4; // (x + 1, y).left
	this->Push(indexFrom, indexTo, weightIndexFrom, weightIndexTo);
}


void Data::PushUp(int x, int y)
{
	if (y == this->columnSize - 1) {
		return;
	}

	const size_t indexFrom = x * this->columnSize + y;
	const size_t indexTo = x * this->columnSize + (y + 1);
	const size_t weightIndexFrom = indexFrom * 4 + 2; // (x, y).up
	const size_t weightIndexTo = indexTo * 4 + 3; // (x, y + 1).down
	this->Push(indexFrom, indexTo, weightIndexFrom, weightIndexTo);
}


void Data::PushDown(int x, int y)
{
	if (y == 0) {
		return;
	}

	const size_t indexFrom = x * this->columnSize + y;
	const size_t indexTo = x * this->columnSize + (y - 1);
	const size_t weightIndexFrom = indexFrom * 4 + 3; // (x, y).down
	const size_t weightIndexTo = indexTo * 4 + 2; // (x, y + 1).up
	this->Push(indexFrom, indexTo, weightIndexFrom, weightIndexTo);
}


void Data::Push(size_t indexFrom, size_t indexTo, size_t weightIndexFrom, size_t weightIndexTo)
{
	if (this->height[indexFrom] != this->height[indexTo] + 1) {
		return;
	}

	int value = std::min(this->weight[weightIndexFrom], this->capacity[indexFrom]);

	this->weight[weightIndexFrom] -= value;
	this->capacity[indexFrom] -= value;
	this->weight[weightIndexTo] += value;
	this->capacity[indexTo] += value;

	this->height[indexTo] = this->capacity[indexTo] > 0 ? 1 : 0;
	if (value > 0) {
		this->active = true;
	}
}

//void Data::Relabel(int x, int y)
//{
//
//}

void Data::BfsFromT()
{
	for (int i = 0; i < this->rowSize; i++) {
		for (int j = 0; j < this->columnSize; j++) {
			if (this->t_weight[i * this->columnSize + j] > 0) {
				this->bfsTag[i * this->columnSize + j] = 1;
			} else {
				this->bfsTag[i * this->columnSize + j] = std::numeric_limits<int>::max();
			}
		}
	}

	int max_k = -1;
	bool check = true;
	for (int k = 1; check; k++) {
		check = false;
		for (int i = 0; i < this->rowSize; i++) {
			for (int j = 0; j < this->columnSize; j++) {
				if (this->bfsTag[i * this->columnSize + j] == k) {
					//
					if (i != 0 && weight[4 * ((i - 1) * this->columnSize + j) + 1] > 0 &&
						this->bfsTag[(i - 1) * this->columnSize + j] > k) {
						this->bfsTag[(i - 1) * this->columnSize + j] = k + 1;
						check = true;
					}

					if (i != this->rowSize && weight[4 * ((i + 1) * this->columnSize + j)] > 0 &&
						this->bfsTag[(i + 1) * this->columnSize + j] > k) {
						this->bfsTag[(i + 1) * this->columnSize + j] = k + 1;
						check = true;
					}

					if (j != this->columnSize - 1 && weight[4 * (i * this->columnSize + j + 1) + 3] > 0 &&
						this->bfsTag[i * this->columnSize + j + 1] > k) {
						this->bfsTag[i * this->columnSize + j + 1] = k + 1;
						check = true;
					}

					if (j != 0 && weight[4 * (i * this->columnSize + j - 1) + 2] > 0 &&
						this->bfsTag[i * this->columnSize + j - 1] > k) {
						this->bfsTag[i * this->columnSize + j - 1] = k + 1;
						check = true;
					}
					//
				}
			}
		}
		//std::cout << "k = " << k << std::endl;
		max_k = k;
	}

	//for (int i = 0; i < this->rowSize * this->columnSize; i++) {
	//	std::cout << this->weight[i * 4] << " ";
	//	std::cout << this->weight[i * 4 + 1] << " ";
	//	std::cout << this->weight[i * 4 + 2] << " ";
	//	std::cout << this->weight[i * 4 + 3] << std::endl;
	//}
	int result[100][100] = { 0 };

	for (int i = 0; i < this->rowSize; i++) {
		for (int j = 0; j < this->columnSize; j++) {
			const size_t index = i * this->columnSize + j;
			int v = this->bfsTag[index];
			if (v <= max_k) {


				if (i != 0 && weight[4 * index] > 0 &&
					this->bfsTag[(i - 1) * this->columnSize + j] > max_k) {
					result[i][j] = 1;
					result[i - 1][j] = 2;
				}

				if (i != this->rowSize && weight[4 * index + 1] > 0 &&
					this->bfsTag[(i + 1) * this->columnSize + j] > max_k) {
					result[i][j] = 1;
					result[i + 1][j] = 2;
				}

				if (j != this->columnSize - 1 && weight[4 * index + 2] > 0 &&
					this->bfsTag[i * this->columnSize + j + 1] > max_k) {
					result[i][j] = 1;
					result[i][j + 1] = 2;
				}

				if (j != 0 && weight[4 * index + 3] > 0 &&
					this->bfsTag[i * this->columnSize + j - 1] > max_k) {
					result[i][j] = 1;
					result[i][j - 1] = 2;
				}


			}
		}
	}

#define HTML_DEBUG
#ifdef HTML_DEBUG
	std::cout << this->rowSize << " " << this->columnSize << std::endl;
	for (int i = 0; i < this->rowSize; i++) {
		std::cout << "<tr>";
		for (int j = 0; j < this->columnSize; j++) {
			int v = this->bfsTag[i * this->columnSize + j];
			//if (v == 6) {
			//	std::cout << "(" << i << " " << j << ")" << std::endl;
			//}
			//std::cout << (v > 10000 ? -1 : v) << " ";

			//if (v == max_k) {
			//	std::cout << "<td class=\"yellow\">&nbsp;</td>";
			if (result[i][j] == 1) {
				std::cout << "<td class=\"yellow\">&nbsp;</td>";
			} else if (result[i][j] == 2) {
				std::cout << "<td class=\"purple\">&nbsp;</td>";
			} else if (this->s_weight[i * this->columnSize + j] > 0) {
				std::cout << "<td class=\"red\">&nbsp;</td>";
			} else if (this->t_weight[i * this->columnSize + j] > 0) {
				std::cout << "<td class=\"green\">&nbsp;</td>";

			} else {
				if (v > max_k) {
					std::cout << "<td>&nbsp;</td>";
				} else {
					std::cout << "<td style=\"background-color: rgb(0, 0," << (5 * v) << ")\">&nbsp;</td>";
				}
			}
		}
		std::cout << "</tr>";
		//std::cout << std::endl;
	}
#endif

	//std::cout << this->rowSize << " " << this->columnSize << std::endl;
	//for (int i = 0; i < this->rowSize; i++) {
	//	for (int j = 0; j < this->columnSize; j++) {
	//		int v = this->bfsTag[i * this->columnSize + j];
	//		if (v == 6) {
	//			std::cout << "(" << i << ", " << j << ")" << std::endl;
	//		}
	//	}
	//	std::cout << std::endl;
	//}
}

void Data::BfsFromT()
{

}


int main()
{
	Data *data = new Data;
	data->Read("data/sample4.txt");
	int flow = data->GetFlow();
	//data->BfsFromT();
	std::cout << "flow = " << flow << std::endl;

	data->BfsFromT();
	while (1);
	return 0;
}