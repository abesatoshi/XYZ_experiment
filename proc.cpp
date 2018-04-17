#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>

#define TEST_FLICKER 1
#define TEST_COLOR 2

struct condition {
	int r;
	int t;
	std::string origin;
	int count;
	double result;
};

const int Ncolor = 6;
const int Ntheta = 2;
const int Nr1[Ncolor][Ntheta] = // n = 61
{
	{ 4, 7 },
	{ 7, 7 },
	{ 4, 4 },
	{ 6, 4 },
	{ 4, 4 },
	{ 4, 6 },
};
const int Nr2[Ncolor][Ntheta] = // n = 62
{
	{ 4, 7 },
	{ 8, 7 },
	{ 6, 6 },
	{ 4, 4 },
	{ 4, 4 },
	{ 4, 4 },
};

int main(int argc, char** argv) {

	if (argc < 3) {
		printf("usage: .exe file type(1:flicker, 2:color)\n");
		exit(1);
	}
	int type = atoi(argv[2]);

	const int Nitr = 3;
	int Ncond = 0;
	for (int i = 0; i < Ncolor; i++)
		for (int j = 0; j < Ntheta; j++)
			Ncond += (type == TEST_FLICKER) ? Nr1[i][j] : Nr2[i][j];
	struct condition *conds = (struct condition*)calloc(Ncond, sizeof(struct condition));

	FILE *Fin = fopen(argv[1], "r");
	FILE *Fout[12] = {
		fopen("GRAY_0.txt", "w"),
		fopen("GRAY_90.txt", "w"),
		fopen("LIGHTGRAY_0.txt", "w"),
		fopen("LIGHTGRAY_90.txt", "w"),
		fopen("DARKGRAY_0.txt", "w"),
		fopen("DARKGRAY_90.txt", "w"),
		fopen("RED_0.txt", "w"),
		fopen("RED_90.txt", "w"),
		fopen("GREEN_0.txt", "w"),
		fopen("GREEN_90.txt", "w"),
		fopen("BLUE_0.txt", "w"),
		fopen("BLUE_90.txt", "w")
	};

	if (Fin == NULL) exit(1);
	for (int i = 0; i < 12; i++) {
		if (Fout[i] == NULL) exit(1);
	}

	char buf[200];
	fgets(buf, 200, Fin);
	
	while (fgets(buf, 200, Fin) != NULL) {
		char *tp;
		tp = strtok(buf, ",");
		std::string origin(tp);
		tp = strtok(NULL, ",");
		int r = atoi(tp);
		tp = strtok(NULL, ",");
		int t = atoi(tp);
		tp = strtok(NULL, ",");
		int ans = atoi(tp);
		tp = strtok(NULL, ",");
		int id = atoi(tp);

		printf("%s, %d, %d, %d, %d\n", origin.c_str(), r, t, ans, id);

		if (id == -1) continue;
		if (conds[id].count == 0) {
			struct condition cond;
			cond.origin = origin;
			cond.r = r;
			cond.t = t;
			cond.count = 0;
			cond.result = 0.0;			
			conds[id] = cond;
		}
		conds[id].result = (conds[id].result * conds[id].count + (double)ans) / (double)(conds[id].count + 1);
		conds[id].count++;
	}

	for (int i = 0; i < Ncond; i++) {
		struct condition c = conds[i];
		if (c.count != Nitr) {
			printf("%d : duplicated %d times %s r=%d t=%d\n",
				i, c.count,
				c.origin.c_str(), c.r, c.t);
			exit(1);
		}
		printf("%d %s r=%d t=%d\n\n",
			i, c.origin.c_str(), c.r, c.t);
	
		int outId;
		if (c.origin == "GRAY" && c.t == 0)			outId = 0;
		if (c.origin == "GRAY" && c.t == 90)		outId = 1;
		if (c.origin == "LIGHTGRAY" && c.t == 0)	outId = 2;
		if (c.origin == "LIGHTGRAY" && c.t == 90)	outId = 3;
		if (c.origin == "DARKGRAY" && c.t == 0)		outId = 4;
		if (c.origin == "DARKGRAY" && c.t == 90)	outId = 5;
		if (c.origin == "RED" && c.t == 0)			outId = 6;
		if (c.origin == "RED" && c.t == 90)			outId = 7;
		if (c.origin == "GREEN" && c.t == 0)		outId = 8;
		if (c.origin == "GREEN" && c.t == 90)		outId = 9;
		if (c.origin == "BLUE" && c.t == 0)			outId = 10;
		if (c.origin == "BLUE" && c.t == 90)		outId = 11;

		fprintf(Fout[outId],
			"%d, %f\n",
			c.r, c.result);
	}
}