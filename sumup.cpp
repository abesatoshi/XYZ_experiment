#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

void singleProcess(int argc, char** argv, char* origin, char* theta) {
	std::string fileName = std::string(origin) + "_" + std::string(theta);
	FILE *Fin, *Fout, *Fout2;
	char buf[100];
	std::vector<double> flickered;
	Fout = fopen(("./sum/" + fileName + ".txt").c_str(), "w");
	Fout2 = fopen("./sum/threshold.txt", "a");
	if (Fout == NULL || Fout2 == NULL) {
		printf("output file open failed\n");
		return;
	}
	fprintf(Fout, "0, 0.0\n");
	bool isOverHalf = false;
	for (int i = 1; i < argc; i++) {
		Fin = fopen(("./" + std::string(argv[i]) + "/" + fileName + ".txt").c_str(), "r");
		if (Fin == NULL) {
			printf("input file open failed : %s\n", ("./" + std::string(argv[i]) + "/" + fileName + ".txt").c_str());
			return;
		}
		int row = 0;
		double prevVal = 0.0;
		int prevAmp = 0;
		while (fgets(buf, 100, Fin) != NULL) {
			char *tp;
			tp = strtok(buf, ",");
			int amp = atoi(tp);
			tp = strtok(NULL, ",");
			double result = atof(tp);
			if (i == 1) flickered.push_back(result);
			else flickered[row] += result;
			if (i == argc - 1) {
				double val = flickered[row] / (argc - 1.0);
				fprintf(Fout, "%d, %f\n", amp, val);
				if (val > 0.5 && prevVal <= 0.5) {
					double intp = prevAmp +
						(amp - prevAmp) / (val - prevVal) * (0.5 - prevVal);
					fprintf(Fout2, "%s, %f\n", fileName.c_str(), intp);
					isOverHalf = true;
				}

				prevVal = val;
			}
			prevAmp = amp;
			row++;
		}
		fclose(Fin);
	}
	if (!isOverHalf) fprintf(Fout2, "%s, NULL\n", fileName.c_str());
	fclose(Fout);
	fclose(Fout2);

}



int main(int argc, char** argv) {
	
	if (argc < 1) {
		printf("input names\n");
		return 1;
	}
	singleProcess(argc, argv, "GRAY", "0");
	singleProcess(argc, argv, "GRAY", "90");
	singleProcess(argc, argv, "LIGHTGRAY", "0");
	singleProcess(argc, argv, "LIGHTGRAY", "90");
	singleProcess(argc, argv, "DARKGRAY", "0");
	singleProcess(argc, argv, "DARKGRAY", "90");
	singleProcess(argc, argv, "RED", "0");
	singleProcess(argc, argv, "RED", "90");
	singleProcess(argc, argv, "GREEN", "0");
	singleProcess(argc, argv, "GREEN", "90");
	singleProcess(argc, argv, "BLUE", "0");
	singleProcess(argc, argv, "BLUE", "90");

	return 0;
}