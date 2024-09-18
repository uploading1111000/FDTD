#include "sim2D.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <SDL3/SDL.h>
using namespace std;
void sim2D::recalculateCoeffs()
{
	int x, y;
	for (x = 0; x < M; x++) {
		for (y = 0; y < N; y++) {
			HUpdateCoeffs[x][y] = Sc / (impedance * muR[x][y]);
			EUpdateCoeffs[x][y] = (impedance / epsR[x][y]) * Sc;
		}
	}

}
void sim2D::initialiseArrays()
{
	Hy = vector<vector<double>>(M - 1, vector<double>(N, 0));
	Hx = vector<vector<double>>(M, vector<double>(N - 1, 0));
	Ez = vector<vector<double>>(M, vector<double>(N, 0));

	muR = vector<vector<double>>(M, vector<double>(N, 1));
	epsR = vector<vector<double>>(M, vector<double>(N, 1));

	int x, y;
	for (x = 0; x < M; x++) {
		for (y = 0; y < N/2; y++) {
			epsR[x][y] = 1.0;
		}
		for (y = N/2; y < N; y++) {
			epsR[x][y] = 9.0;
		}
	}

	HUpdateCoeffs = vector<vector<double>>(M, vector<double>(N));
	EUpdateCoeffs = vector<vector<double>>(M, vector<double>(N));

	recalculateCoeffs();
}

sim2D::sim2D(int width, int height, double impedance)
{
	this->Sc = 1 / sqrt(2.0);

	this->impedance = impedance;
	this->M = width;
	this->N = height;
	initialiseArrays();
	this->q = 0;
}
void sim2D::updateH()
{
	int x, y;
	for (x = 0; x < M; x++) {
		for (y = 0; y < N; y++) {
			if (y < N-1) Hx[x][y] = Hx[x][y] - (Ez[x][y + 1] - Ez[x][y]) * HUpdateCoeffs[x][y];
			if (x < M-1) Hy[x][y] = Hy[x][y] + (Ez[x + 1][y] - Ez[x][y]) * HUpdateCoeffs[x][y];
		}
	}
}
void sim2D::updateE()
{
	int x, y;
	for (x = 1; x < M - 1; x++) {
		for (y = 1; y < N - 1; y++) {
			Ez[x][y] = Ez[x][y] + EUpdateCoeffs[x][y] * (
				(Hy[x][y] - Hy[x - 1][y]) 
				- (Hx[x][y] - Hx[x][y - 1])
			);
		}
	}
}

void sim2D::source()
{
	for (auto s : sources) {
		double arg = M_PI * ((Sc * s.time) / 20 - 1.0);
		arg = arg * arg;
		Ez[s.x][s.y] += 3.0 * (1.0 - 2.0 * arg) * exp(-arg);
	}
}

double sim2D::getMag(int m, int n)
{
	double total = 0.0;
	if (m > 0) total += sqrt(Hy[m - 1][n] * Hy[m - 1][n]) * impedance / 4;
	if (n > 0) total += sqrt(Hx[m][n - 1] * Hx[m][n - 1]) * impedance / 4;
	if (m < M-1) total += sqrt(Hy[m][n] * Hy[m][n]) * impedance / 4; 
	if (n < N-1) total += sqrt(Hx[m][n] * Hx[m][n] * impedance) / 4;
	total += sqrt(Ez[m][n] * Ez[m][n]);
	return total;
}

void sim2D::step()
{
	updateH();
	updateE();
	source();
	q++;
	for (auto i = sources.begin(); i != sources.end(); i++) {
		(*i).time++;
	}
	while (sources.front().time >= 40 and sources.size() > 0) {
		sources.pop_front();
	}
}
