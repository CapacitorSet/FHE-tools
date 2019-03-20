#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <fhe-tools.h>
#include <rpc/client.h>

#include "patient.h"

TFHEClientParams_t default_client_params;

constexpr int NUM_PATIENTS = 10;

int main() {
	if (PLAINTEXT)
		puts("Plaintext mode.");
	else
		puts("Ciphertext mode.");
	puts("Initializing TFHE...");
	FILE *secret_key = fopen("secret.key", "rb");
	if (secret_key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	default_client_params = makeTFHEClientParams(secret_key);
	fclose(secret_key);

	puts("Reading data...");
	std::ifstream file("examples/health/data.csv");
	std::string line;

	std::getline(file, line);
	assert(line == "height,weight,age,male");

	auto records = Array<Patient, NUM_PATIENTS>(true, default_client_params);

	for (int i = 0; i < NUM_PATIENTS; i++) {
		std::getline(file, line);
		double height;
		double weight;
		int8_t age;
		char isMale;
		sscanf(line.c_str(), "%lf,%lf,%d,%c\n", &height, &weight, &age, &isMale);
		Patient p(height, weight, age, isMale == '1', default_client_params);
		records.put(p, i);
	}

	puts("Connecting to server...");
	rpc::client client("127.0.0.1", 8000);
	puts("1. Uploading database...");
	client.call("uploadDatabase", records.exportToString());
	puts("");

	puts("2. Counting men...");
	std::string countMStr = client.call("countM").as<std::string>();
	Int8 countM(countMStr, default_client_params);
	printf("Output: %d\n", countM.toInt());
	puts("");

	puts("3. Computing average weight...");
	std::string sumStr = client.call("sumWeight").as<std::string>();
	Fixed<11, 1> sum(sumStr, default_client_params);
	// The division is made client-side for performance reasons.
	printf("Output: %lf\n", sum.toDouble() / 10.0);
	puts("");

	// Pick one patient to use for the weight demo
	Patient sample_patient(default_client_params);
	int i = 0;
	do {
		records.get(sample_patient, i++);
	} while (sample_patient.age.toInt() > 20);
	printf("4. Predicting weight for height=%lf...\n", sample_patient.getHeight());
	using Q7_9 = Fixed<7, 9>;
	Q7_9 height(Patient::scaleHeight(sample_patient.getHeight()), default_client_params);
	std::string weightStr = client.call("predictWeight", height.exportToString()).as<std::string>();
	Q7_9 weight(weightStr, default_client_params);
	printf("Predicted: %lf, actual: %lf\n", weight.toDouble(), sample_patient.weight.toDouble());

	return 0;
}
