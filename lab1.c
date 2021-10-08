#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>


const int n_experiments = 10;


void prepare_records_native() {

}

void search_record_native() {

}



float get_time_score(clock_t c1, clock_t c2) {
	return ((float) c2 - (float) c1) / CLOCKS_PER_SEC;
}


void report_time_score(char* title, float* measurements) {
	float sum = 0.0;
	for (int i = 0; i < n_experiments; i++) {
		sum += measurements[i];
	}
	float result = sum / n_experiments;
	
	printf("Avg time (%s): %f\n", title, result);
}

void report_memory_usage() {
	struct rusage mem;
	getrusage(RUSAGE_SELF, &mem);
	printf("Max mem usage[kB]: %ld\n", mem.ru_maxrss);	
}


int main() {	
	float time_res_prepare[n_experiments];
	float time_res_search[n_experiments];
	clock_t c1, c2;
	
	for (int i = 0; i < n_experiments; i++) {
		c1 = clock();
		// prepare_records_native();
		printf("dziala\n");
		c2 = clock();
		time_res_prepare[i] = get_time_score(c1, c2);
	}
	
	report_time_score("prepare", time_res_prepare);
	// report_time_score("search", time_res_search);
	report_memory_usage();
	return 0;
}
