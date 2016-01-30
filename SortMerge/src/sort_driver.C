#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <iostream.h>
#include <assert.h>
#include <stdio.h>

#include "scan.h"
#include "sort.h"
#include "db.h"
#include "buf.h"
#include "minirel.h"
#include "heapfile.h"
#include "new_error.h"

#define	DBSIZE 		500
#define NUM_RECORDS	94
#define	REC_LEN1	256
#define	REC_LEN2	32
#define SORTPGNUM	 4


int MINIBASE_RESTART_FLAG = 0;

void test1();
void test2();

int	keySize;
TupleOrder sortOrder;

char*	data1[NUM_RECORDS] = {
"raghu", "xbao", "cychan", "leela", "ketola", "soma", "ulloa", "dhanoa", "dsilva",
"kurniawa", "dissoswa", "waic", "susanc", "kinc", "marc", "scottc", "yuc", "ireland",
"rathgebe", "joyce", "daode", "yuvadee", "he", "huxtable", "muerle", "flechtne",
"thiodore", "jhowe", "frankief", "yiching", "xiaoming", "jsong", "yung", "muthiah",
"bloch", "binh", "dai", "hai", "handi", "shi", "sonthi", "evgueni", "chung-pi",
"chui", "siddiqui", "mak", "tak", "sungk", "randal", "barthel", "newell", "schiesl",
"neuman", "heitzman", "wan", "gunawan", "djensen", "juei-wen", "josephin", "harimin",
"xin", "zmudzin", "feldmann", "joon", "wawrzon", "yi-chun", "wenchao", "seo",
"karsono", "dwiyono", "ginther", "keeler", "peter", "lukas", "edwards", "mirwais",
"schleis", "haris", "meyers", "azat", "shun-kit", "robert", "markert", "wlau",
"honghu", "guangshu", "chingju", "bradw", "andyw", "gray", "vharvey", "awny",
"savoy", "meltz" };

char*	data2[NUM_RECORDS] = {
"andyw", "awny", "azat", "barthel", "binh", "bloch", "bradw", "chingju",
"chui", "chung-pi", "cychan", "dai", "daode", "dhanoa", "dissoswa", "djensen",
"dsilva", "dwiyono", "edwards", "evgueni", "feldmann", "flechtne", "frankief",
"ginther", "gray", "guangshu", "gunawan", "hai", "handi", "harimin", "haris",
"he", "heitzman", "honghu", "huxtable", "ireland", "jhowe", "joon", "josephin",
"joyce", "jsong", "juei-wen", "karsono", "keeler", "ketola", "kinc", "kurniawa",
"leela", "lukas", "mak", "marc", "markert", "meltz", "meyers", "mirwais", "muerle",
"muthiah", "neuman", "newell", "peter", "raghu", "randal", "rathgebe", "robert",
"savoy", "schiesl", "schleis", "scottc", "seo", "shi", "shun-kit", "siddiqui",
"soma", "sonthi", "sungk", "susanc", "tak", "thiodore", "ulloa", "vharvey", "waic",
"wan", "wawrzon", "wenchao", "wlau", "xbao", "xiaoming", "xin", "yi-chun", "yiching",
"yuc", "yung", "yuvadee", "zmudzin" };

//-------------------------------------------------------------
// usage
//-------------------------------------------------------------
void usage(char *name)
{
    cerr << "Usage: " << name << " test_number ...";
    cerr << endl;
    exit(1);
}

//-------------------------------------------------------------
// parse
//-------------------------------------------------------------
char** parse(int argc, char** argv)
{
    if (argc == 1)
       usage(argv[0]);
    if (argv[1][0] == '-')
       usage(argv[0]);

    return ++argv;
}


//-------------------------------------------------------------
// main
//-------------------------------------------------------------
main(int argc, char **argv)
{
  Status  st;

  char real_logname[50];
  char real_dbname[55];

  sprintf(real_logname, "/bin/rm -rf sortlog");
  sprintf(real_dbname, "/bin/rm -rf SORTDRIVER");
  system(real_logname);
  system(real_dbname);

  minibase_globals = new SystemDefs(st,"SORTDRIVER",
                                  "sortlog", 1000,500,200, "Clock");

  if (st != OK) {
     minibase_errors.show_errors();
     exit(1);
  }

   void (*tests[])() = {
      test1, test2
   };

   int tests_n = sizeof(tests) / sizeof(void (*)());

   char** test_num = parse(argc, argv);


   for ( ; *test_num; test_num++) {
      int i = atoi(*test_num);
      if (i < 1 || i > tests_n)
	 usage(argv[0]);

      tests[i-1]();
   }

   cout << "--------------------- Test Done ----------------------\n";

   delete minibase_globals;
   system(real_logname);
   system(real_dbname);

   return 0;
}

//-------------------------------------------------------------
// test1
//-------------------------------------------------------------
void test1()
{
	struct R1 {
		char	key [32];
		char	filler[224];
	} rec; 

	AttrType 	attrType[] = { attrString, attrString };
	short		attrSize[] = { 32, 224 };

	// Create unsorted data file "test1.in"
	Status		s;
	RID		rid;
	HeapFile	f("test1.in",s);
	assert(s == OK);
	for (int i=0; i<NUM_RECORDS; i++)
	{
		strcpy(rec.key, data1[i]);
		s = f.insertRecord((char*)&rec,REC_LEN1,rid);
		assert(s == OK);
	}

	// Sort "test1.in" into "test1.out"
	keySize = attrSize[0];
	sortOrder = Ascending;
	Sort 		sort("test1.in","test1.out",2,attrType,attrSize,0,Ascending,SORTPGNUM,s);
	f.deleteFile();
	if (s != OK)
	{
		minibase_errors.show_errors();
		minibase_errors.clear_errors();
		return;
	}

	// check if "test1.out" is sorted correctly
	HeapFile	f2("test1.out",s);
	if (s != OK)
	{
		cout << "Test1 -- OOPS! test1.out not created\n";
		return;
	}
	Scan*	scan = f2.openScan(s);
	assert(s == OK);
	int len;
	int count = 0;
	for (s = scan->getNext(rid, (char *) &rec, len); 
		(s == OK) && (count < NUM_RECORDS);
		s = scan->getNext(rid, (char *) &rec, len)) 
	{
		if (strcmp(rec.key,data2[count]) != 0)
		{
			cout << "Test1 -- OOPS! test1.out not sorted\n";
			s = FAIL;
		}
		count++;
	}
	if (count != NUM_RECORDS)
		cout << "Test1 -- OOPS! Sorting Failed\n";
	else
		cout << "Test1 -- Sorting OK\n";
	f2.deleteFile();
}

//-------------------------------------------------------------
// test2
//-------------------------------------------------------------
void test2()
{
	struct R1 {
		char	key [32];
	} rec; 

	AttrType 	attrType[] = { attrString };
	short		attrSize[] = { 32 };

	// Create unsorted data file "test2.in"
	Status		s;
	RID		rid;
	HeapFile	f("test2.in",s);
	assert(s == OK);
	for (int i=0; i<NUM_RECORDS; i++)
	{
		strcpy(rec.key, data2[i]);
		s = f.insertRecord((char*)&rec,REC_LEN2,rid);
		assert(s == OK);
	}

	// Sort "test2.in" into "test2.out"
	keySize = attrSize[0];
	sortOrder = Ascending;
	Sort 		sort("test2.in","test2.out",1,attrType,attrSize,0,Ascending,SORTPGNUM,s);
	f.deleteFile();
	if (s != OK)
	{
		minibase_errors.show_errors();
		minibase_errors.clear_errors();
		return;
	}

	// check if "test2.out" is sorted correctly
	HeapFile	f2("test2.out",s);
	if (s != OK)
	{
		cout << "Test2 -- OOPS! test2.out not created\n";
		return;
	}
	Scan*	scan = f2.openScan(s);
	assert(s == OK);
	int len;
	int count = 0;
	for (s = scan->getNext(rid, (char *) &rec, len); 
		(s == OK) && (count < NUM_RECORDS);
		s = scan->getNext(rid, (char *) &rec, len)) 
	{
		if (strcmp(rec.key,data2[count]) != 0)
		{
			cout << "Test2 -- OOPS! test2.out not sorted\n";
			s = FAIL;
		}
		count++;
	}
	if (count != NUM_RECORDS)
		cout << "Test2 -- OOPS! Sorting Failed\n";
	else
		cout << "Test2 -- Sorting OK\n";
	f2.deleteFile();
}

