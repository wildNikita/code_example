// StrassenMethod.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <cmath>
#include <omp.h>

int Start_size = 4;
int Parameter = 2;
int Block_size = 64;

struct Matrix {
	int size;
	double* matrix;

	Matrix() {}
	Matrix(int size) : size(size) {
		matrix = new double[size * size];

		for (int i = 0; i < size; ++i) {
			for (int j = 0; j < size; ++j) {
				matrix[i * size + j] = 0;
			}
		}
	}
	Matrix(const Matrix& A, int bottom, int right) {
		size = A.size / 2;
		matrix = new double[size * size];

		for (int i = 0; i < size; ++i) {
			for (int j = 0; j < size; ++j) {
				matrix[i * size + j] = A.matrix[(i + size * bottom) * A.size + j + size * right];
			}
		}
	}
	Matrix(const Matrix& A) {
		size = A.size;
		matrix = new double[size * size];

		for (int i = 0; i < size; ++i) {
			for (int j = 0; j < size; ++j) {
				matrix[i * size + j] = A.matrix[i * A.size + j];
			}
		}
	}

	~Matrix() {
		delete[] matrix;
		matrix = nullptr;
	}

	void Set() {
		for (int i = 0; i < size; ++i) {
			for (int j = 0; j < size; ++j) {
				matrix[i * size + j] = -1 + 2 * ((double)rand() / RAND_MAX);
			}
		}
	}
	Matrix operator =(const Matrix& B) {
		Matrix C(B.size);
		for (int i = 0; i < B.size; ++i) {
			for (int j = 0; j < B.size; ++j) {
				matrix[i * size + j] = B.matrix[i * B.size + j];
			}
		}
		return *this;
	}

	friend const Matrix operator +(const Matrix& A, const Matrix& B);

	friend const Matrix operator -(const Matrix& A, const Matrix& B);

	const Matrix operator *(const Matrix& B) {
		Matrix C(B.size);
		double tmp;
		Matrix res_block(Block_size);
		double *left_block = new double[B.size];

		for (int i_start = 0; i_start < size; i_start += Block_size) {
			for (int j_start = 0; j_start < size; j_start += Block_size) {

				for (int i = i_start; i < i_start + Block_size; i++) {
					for (int j = 0; j < size; ++j) {
						left_block[j] = matrix[i * C.size + j];
					}

					for (int k = 0; k < size; ++k) {
						for (int j = j_start; j < j_start + Block_size; j++) {
							res_block.matrix[(i - i_start) * Block_size + j - j_start] += left_block[k] * B.matrix[k * B.size + j];
						}
					}

				}

				for (int i = i_start; i < i_start + Block_size; i++) {
					for (int j = j_start; j < j_start + Block_size; j++) {
						C.matrix[i * C.size + j] = res_block.matrix[(i - i_start) * Block_size + j - j_start];
						res_block.matrix[(i - i_start) * Block_size + j - j_start] = 0;
					}
				}
			}
		}

		return C;
	}
	bool operator ==(Matrix& B) {
		for (int i = 0; i < B.size; ++i) {
			for (int j = 0; j < B.size; ++j) {
				if (matrix[i * size + j] != B.matrix[i * B.size + j]) {
					return false;
				}
			}
		}
		return true;
	}
	friend std::ostream& operator <<(std::ostream& out, const Matrix& B);

};

std::ostream& operator <<(std::ostream& out, const Matrix& B) {
	for (int i = 0; i < B.size; ++i) {
		for (int j = 0; j < B.size; ++j) {
			std::cout << B.matrix[i * B.size + j] << " ";
		}
		out << std::endl;
	}
	return out;
}

const Matrix operator +(const Matrix& A, const Matrix& B) {
	Matrix C(B.size);
	for (int i = 0; i < B.size; ++i) {
		for (int j = 0; j < B.size; ++j) {
			C.matrix[i * C.size + j] = A.matrix[i * A.size + j] + B.matrix[i * B.size + j];
		}
	}
	return C;
}

const Matrix operator -(const Matrix& A, const Matrix& B) {
	Matrix C(B.size);

	for (int i = 0; i < B.size; ++i) {
		for (int j = 0; j < B.size; ++j) {
			C.matrix[i * C.size + j] = A.matrix[i * A.size + j] - B.matrix[i * B.size + j];
		}
	}
	return C;
}

const Matrix Strassen(Matrix A, Matrix B)
{
	if (A.size > Parameter) {
		Matrix A11(A, 0, 0), A12(A, 0, 1), A21(A, 1, 0), A22(A, 1, 1);
		Matrix B11(B, 0, 0), B12(B, 0, 1), B21(B, 1, 0), B22(B, 1, 1);

		Matrix P1(A.size / 2), P2(A.size / 2), P3(A.size / 2), P4(A.size / 2), P5(A.size / 2), P6(A.size / 2), P7(A.size / 2);

#pragma omp task 
		{
			P1 = Strassen(A11 + A22, B11 + B22);
		}
#pragma omp task 
		{
			P2 = Strassen(A21 + A22, B11);
		}
#pragma omp task 
		{
			P3 = Strassen(A11, B12 - B22);
		}
#pragma omp task 
		{
			P4 = Strassen(A22, B21 - B11);
		}
#pragma omp task 
		{
			P5 = Strassen(A11 + A12, B22);
		}
#pragma omp task 
		{
			P6 = Strassen(A21 - A11, B11 + B12);
		}
#pragma omp task 
		{
			P7 = Strassen(A12 - A22, B21 + B22);
		}
#pragma omp taskwait
		std::cout << P1 << std::endl;

		Matrix C11 = P1 + P4 - P5 + P7,
			C12 = P3 + P5,
			C21 = P2 + P4,
			C22 = P1 - P2 + P3 + P6;

		Matrix C(A.size);

		for (int i = 0; i < C11.size; ++i) {
			for (int j = 0; j < C11.size; ++j) {
				C.matrix[i * C.size + j] = C11.matrix[i * C11.size + j];
				C.matrix[i * C.size + j + C11.size] = C12.matrix[i * C12.size + j];
				C.matrix[(i + C11.size) * C.size + j] = C21.matrix[i * C21.size + j];
				C.matrix[(i + C11.size) * C.size + j + C11.size] = C22.matrix[i * C22.size + j];
			}
		}
		return C;
	}
	else {
		return A * B;
	}
}

int main()
{
	double start_time, finish_time;
	std::cout << "size " << std::endl;
	std::cin >> Start_size;
	std::cout << "Parameter" << std::endl;
	std::cin >> Parameter;
	std::cout << "block size " << std::endl;
	std::cin >> Block_size;

	Matrix A(Start_size), B(Start_size), C(Start_size);
	srand(time(nullptr));
	A.Set();
	B.Set();

	start_time = omp_get_wtime();
	clock_t begin = clock();
#pragma omp Parameterallel
	{
	#pragma omp single
		{
			C = Strassen(A, B);
		}
	}
	clock_t endclock = clock();
	finish_time = omp_get_wtime();

	Matrix Sub = A * B - C;

	double sub_measure = 0.0f;
	for (int i = 0; i < Start_size; i++)
	{
		for (int j = 0; j < Start_size; j++)
		{
			sub_measure += Sub.matrix[i * Start_size + j] * Sub.matrix[i * Start_size + j];
		}
	}

	double c_measure = 0.0f;
	for (int i = 0; i < Start_size; i++)
	{
		for (int j = 0; j < Start_size; j++)
		{
			c_measure += C.matrix[i * Start_size + j] * C.matrix[i * Start_size + j];
		}
	}


	std::cout << "error = " << sqrt(sub_measure / c_measure) << std::endl;

	std::cout << "time: " << finish_time - start_time << std::endl;
	std::cout << "time: " << ((double)endclock - begin) / CLOCKS_PER_SEC << std::endl;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
