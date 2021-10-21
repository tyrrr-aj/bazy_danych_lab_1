#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>


const int n_experiments = 10;
const int n_records = 1000000;
const int searched_id = n_records - 1;

const char* name = "Zora Cock";
const char* description = "Lead singer of band Blackbriar, an exceptional musician!";


float get_time_score(clock_t c1, clock_t c2);


// native

struct Rec {
	int id;
	char name[20];
	char desc[90];
};

struct Rec* prepare_records_native() {
	struct Rec* records = malloc(sizeof(struct Rec) * n_records);
	
	for (int i = 0; i < n_records; i++) {
		records[i].id = i;
		strcpy(records[i].name, name);
		strcpy(records[i].desc, description);
	}
	
	return records;
}

void search_record_native(struct Rec* records) {
	for (; records->id != searched_id; records++);
}

void cleanup_native(struct Rec* records) {
	free(records);
}

void measure_native(float* time_res_prepare, float* time_res_search) {
	clock_t c1, c2;

	c1 = clock();
	struct Rec* records = prepare_records_native();
	c2 = clock();
	*time_res_prepare = get_time_score(c1, c2);
	
	c1 = clock();
	search_record_native(records);
	c2 = clock();
	*time_res_search = get_time_score(c1, c2);

	cleanup_native(records);
}


// SQLite

const char* filename = "data.db";
// const char* filename = ":memory:";

sqlite3* open_sqlite() {
	int rc;
	sqlite3* db;
	
	rc = sqlite3_open(filename, &db);
	if (rc) {
		fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(db));
		exit(1);
	}

	return db;
}

void create_table(sqlite3* db) {
	int rc = sqlite3_exec(db,
		"CREATE TABLE People (id integer PRIMARY KEY AUTOINCREMENT, name varchar(20), desc varchar(90));",
		NULL, NULL, NULL);
	
	if (rc) {
		fprintf(stderr, "Database create table error: %s\n", sqlite3_errmsg(db));
		exit(1);
	}
}

void insert_records(sqlite3* db) {
	int rc;
	sqlite3_stmt* stmt;

	char* sql = "INSERT INTO People(name, desc) VALUES (?1, ?2)";
	rc = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
	if (rc) {
		fprintf(stderr, "Database prepare INSERT statement error: %s\n", sqlite3_errmsg(db));
		exit(1);
	}

	for (int i = 0; i < n_records; i++) {
		sqlite3_bind_text(stmt, 1, name, strlen(name) + 1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, description, strlen(description) + 1, SQLITE_STATIC);

		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE) {
			fprintf(stderr, "Database execute INSERT error: %s\n", sqlite3_errmsg(db));
			exit(1);
		}

		sqlite3_reset(stmt);
	}

	sqlite3_finalize(stmt);
}

void search_record_sqlite(sqlite3* db) {
	int rc;
	sqlite3_stmt* stmt;

	char* sql = "SELECT * FROM People WHERE id = ?1";
	rc = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL);
	if (rc) {
		fprintf(stderr, "Database prepare SELECT statement error: %s\n", sqlite3_errmsg(db));
		exit(1);
	}

	sqlite3_bind_int(stmt, 1, searched_id);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		fprintf(stderr, "Database execute SELECT error: %s\n", sqlite3_errmsg(db));
		exit(1);
	}

	sqlite3_finalize(stmt);
}

void close_sqlite(sqlite3* db) {
	sqlite3_close(db);

	if (remove(filename) != 0) {
		fprintf(stderr, "Deleting sqlite file failed\n");
		exit(1);
	}
}

void measure_sqlite(float* time_res_prepare, float* time_res_search) {
	clock_t c1, c2;

	sqlite3* db = open_sqlite();

	c1 = clock();
	create_table(db);
	insert_records(db);
	c2 = clock();
	*time_res_prepare = get_time_score(c1, c2);
	
	c1 = clock();
	search_record_sqlite(db);
	c2 = clock();
	*time_res_search = get_time_score(c1, c2);

	close_sqlite(db);
}


// measurements

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
	
	for (int i = 0; i < n_experiments; i++) {
		// measure_native(&time_res_prepare[i], &time_res_search[i]);
		measure_sqlite(&time_res_prepare[i], &time_res_search[i]);
		printf("total time: %f\n", time_res_prepare[i] + time_res_search[i]);
	}
	printf("\n");
	
	report_time_score("prepare", time_res_prepare);
	report_time_score("search", time_res_search);
	report_memory_usage();
	return 0;
}
