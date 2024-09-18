#pragma once
#include <vector>
#include <list>
struct source {
	int x;
	int y;
	int time;
};

class sim2D
{
private:
	std::vector <std::vector<double>> HUpdateCoeffs;
	std::vector <std::vector<double>> EUpdateCoeffs;
protected:
	std::vector<std::vector<double>> Hy;
	std::vector<std::vector<double>> Hx;
	std::vector<std::vector<double>> Ez;
	std::vector<std::vector<double>> epsR;
	std::vector<std::vector<double>> muR;
	double impedance;
	double Sc;

	std::list<source> sources;

	int q;

	int M;
	int N;

	void initialiseArrays();
	void recalculateCoeffs();

	void updateH();
	void updateE();
	void source();

public:
	sim2D(int width, int height, double impermeability);
	double getEField(int m, int n) {return Ez[m][n];};
	double getMag(int m, int n);
	double getHyField(int m, int n) { return (m < M - 1) ? Hy[m][n] * impedance : 0; };
	double getHxField(int m, int n) { return (n < N - 1) ? Hx[m][n] * impedance: 0; };
	void step();
	void addSource(int x, int y) {sources.push_back({x,y,0});};
};

