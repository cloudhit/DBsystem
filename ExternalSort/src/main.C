#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <iostream>
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
#define NUM_RECORDS	108
#define	REC_LEN1	256
#define	REC_LEN2	32
#define SORTPGNUM	4

int MINIBASE_RESTART_FLAG = 0;

void test1();
void test2();
void test3();
void test4();
void test5();
void test6();

int	keySize;
TupleOrder sortOrder;
int keyPos;

/*
int tupleCmp (const void* t1, const void* t2 )
{
  int diff = strncmp( (char*)t1,(char*)t2, keySize );

  if ( sortOrder == Ascending )
  {
    if ( diff > 0 ) return 1; 
    if ( diff < 0 ) return -1;
    if ( diff == 0 ) return 0;
  }
  
  else
  {
    if ( diff < 0 ) return 1;
    if ( diff > 0 ) return -1;
    if ( diff == 0 ) return 0;
  }
}
*/

char*	data1[NUM_RECORDS] = {
"weilin", "selarson", "joewono", "schellen", 
"mcelroy", "mjhans", "mchao", "ki", 
"chi-chen", "greenlan", "baiming", "wu-sheng", 
"tchan", "soumya", "rebecca", "xinjun", 
"xiaobing", "prawit", "zmiller", "fzhao", 
"jeffreyw", "dbs", "siew", "gavin", 
"yoomi", "bradleyb", "manikuti", "shushuai", 
"jmelski", "mei", "satish", "czhang", 
"whitney", "lfan", "reynaldo", "chihfeng", 
"cheng-fu", "brianh", "kyo-jin", "kit-yee", 
"handy", "qun", "pomeroy", "noto", 
"liu", "honerman", "kimberly", "kangyu", 
"arcady", "lin", "hongju", "salwa", 
"nengwu", "liang", "gan", "susan", 
"daffinru", "chia-hsi", "bi-shing", "iigor", 
"hog", "taodb", "hangying", "masse", 
"qiongluo", "area", "jamin", "llam", 
"kelley", "kjohnson", "hinkfuss", "arora", 
"stenglei", "du", "dingwei", "ramirez", 
"milo", "sromalsk", "andersen", "quinnj", 
"rueden", "sanj", "jhu", "chengh", 
"da-jun", "rustandi", "jchen", "jinsong", 
"rushan", "jiazhen", "jaideep", "gang", 
"miu", "mei-guei", "zebell", "weiss", 
"da-tong", "dzimm", "nwang", "kuong", 
"mlk", "schoenof", "adnan", "yunrui", 
"tae-youn", "qouyang", "wanwimol", "bauman"
};

char*	data2[NUM_RECORDS] = {
"adnan", "andersen", "arcady", "area", 
"arora", "baiming", "bauman", "bi-shing", 
"bradleyb", "brianh", "cheng-fu", "chengh", 
"chi-chen", "chia-hsi", "chihfeng", "czhang", 
"da-jun", "da-tong", "daffinru", "dbs", 
"dingwei", "du", "dzimm", "fzhao", 
"gan", "gang", "gavin", "greenlan", 
"handy", "hangying", "hinkfuss", "hog", 
"honerman", "hongju", "iigor", "jaideep", 
"jamin", "jchen", "jeffreyw", "jhu", 
"jiazhen", "jinsong", "jmelski", "joewono", 
"kangyu", "kelley", "ki", "kimberly", 
"kit-yee", "kjohnson", "kuong", "kyo-jin", 
"lfan", "liang", "lin", "liu", 
"llam", "manikuti", "masse", "mcelroy", 
"mchao", "mei", "mei-guei", "milo", 
"miu", "mjhans", "mlk", "nengwu", 
"noto", "nwang", "pomeroy", "prawit", 
"qiongluo", "qouyang", "quinnj", "qun", 
"ramirez", "rebecca", "reynaldo", "rueden", 
"rushan", "rustandi", "salwa", "sanj", 
"satish", "schellen", "schoenof", "selarson", 
"shushuai", "siew", "soumya", "sromalsk", 
"stenglei", "susan", "tae-youn", "taodb", 
"tchan", "wanwimol", "weilin", "weiss", 
"whitney", "wu-sheng", "xiaobing", "xinjun", 
"yoomi", "yunrui", "zebell", "zmiller"
};

char *data3[NUM_RECORDS] = {
"zmiller", "zebell", "yunrui", "yoomi", 
"xinjun", "xiaobing", "wu-sheng", "whitney", 
"weiss", "weilin", "wanwimol", "tchan", 
"taodb", "tae-youn", "susan", "stenglei", 
"sromalsk", "soumya", "siew", "shushuai", 
"selarson", "schoenof", "schellen", "satish", 
"sanj", "salwa", "rustandi", "rushan", 
"rueden", "reynaldo", "rebecca", "ramirez", 
"qun", "quinnj", "qouyang", "qiongluo", 
"prawit", "pomeroy", "nwang", "noto", 
"nengwu", "mlk", "mjhans", "miu", 
"milo", "mei-guei", "mei", "mchao", 
"mcelroy", "masse", "manikuti", "llam", 
"liu", "lin", "liang", "lfan", 
"kyo-jin", "kuong", "kjohnson", "kit-yee", 
"kimberly", "ki", "kelley", "kangyu", 
"joewono", "jmelski", "jinsong", "jiazhen", 
"jhu", "jeffreyw", "jchen", "jamin", 
"jaideep", "iigor", "hongju", "honerman", 
"hog", "hinkfuss", "hangying", "handy", 
"greenlan", "gavin", "gang", "gan", 
"fzhao", "dzimm", "du", "dingwei", 
"dbs", "daffinru", "da-tong", "da-jun", 
"czhang", "chihfeng", "chia-hsi", "chi-chen", 
"chengh", "cheng-fu", "brianh", "bradleyb", 
"bi-shing", "bauman", "baiming", "arora", 
"area", "arcady", "andersen", "adnan"
};

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
int main(int argc, char **argv)
{
  Status  st;


  void (*tests[])() = {
	test1, test2, test3, test4, test5, test6
  };
  
  int tests_n = sizeof(tests) / sizeof(void (*)());

  for (int i=0; i < tests_n; i++) {
	char real_logname[50];
	char real_dbname[55];
	
	sprintf(real_logname, "/bin/rm -rf /tmp/sortlog");
	sprintf(real_dbname, "/bin/rm -rf /tmp/SORTDRIVER");
	system(real_logname);
	system(real_dbname);
	
	minibase_globals = new SystemDefs(st,"/tmp/SORTDRIVER",
									  "/tmp/sortlog", 1000,500,200, "Clock");
	
	if (st != OK) {
	  minibase_errors.show_errors();
	  exit(1);
	}
	tests[i]();
	
	delete minibase_globals;
	system(real_logname);
	system(real_dbname);
  }

  cout << "--------------------- Test Done ----------------------\n";
    
  return 0;
}

//-------------------------------------------------------------
// test1
//   Test simple case, only need one pass to sort
//-------------------------------------------------------------
void test1() {
  Status s;
  RID rid;
  HeapFile f("test1.in", s);
  assert(s == OK);

  AttrType attrType[] = {attrString};
  short attrSize[1];
  attrSize[0] = sizeof(char);
  
  for (int i=0; i < 100; i++) {
	char c = i;
	s = f.insertRecord(&c, sizeof(char), rid);
	assert(s == OK);
  }

  Sort sort("test1.in", "test1.out", 1, 
			attrType, attrSize, 0,
			Ascending, SORTPGNUM, s);
  if (s != OK) {
	cout << "Error : Error return from Sort" << endl;
	minibase_errors.show_errors();
	minibase_errors.clear_errors();
	return;
  }

  s = f.deleteFile();
  assert(s == OK);

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
  char res;
  for (s = scan->getNext(rid, &res, len); 
	   s == OK;
	   s = scan->getNext(rid, &res, len)) 
	{
	  if (res != count)
		{
		  cout << "Test1 -- OOPS! test1.out not sorted\n";
		  s = FAIL;
		}
	  count++;
	}

  if (count != 100)
	cout << "Test1 -- OOPS! Sorting Failed " << count << endl;
  else
	cout << "Test1 -- Sorting OK\n";
  f2.deleteFile();
}
  
//-------------------------------------------------------------
// test2
//   Test sort over a empty file
//-------------------------------------------------------------
void test2() {
  Status s;
  RID rid;
  HeapFile f("test2.in", s);
  assert(s == OK);

  AttrType attrType[] = {attrString};
  short attrSize[1];
  attrSize[0] = sizeof(char);
  
  Sort sort("test2.in", "test2.out", 1, 
			attrType, attrSize, 0,
			Ascending, SORTPGNUM, s);
  if (s != OK) {
	cout << "Error : Error return from Sort" << endl;
	minibase_errors.show_errors();
	minibase_errors.clear_errors();
	return;
  }

  s = f.deleteFile();
  assert(s == OK);

  // check if "test1.out" is sorted correctly
  HeapFile	f2("test2.out",s);
  if (s != OK)
	{
	  cout << "Test2 -- OOPS! test1.out not created\n";
	  return;
	}
  Scan*	scan = f2.openScan(s);
  assert(s == OK);


  char res;
  int len;
  s = scan->getNext(rid, &res, len); 
  
  if (s != DONE)
	cout << "Test2 -- OOPS! Sorting Failed."<< endl;
  else
	cout << "Test2 -- Sorting OK\n";
  f2.deleteFile();  
}

//-------------------------------------------------------------
// test3
// Test case with large record size and multiple passes
//-------------------------------------------------------------
void test3()
{
	struct R1 {
		char	filler[800];
		char	key [32];
	} rec; 

	AttrType 	attrType[] = { attrString, attrString };
	short		attrSize[] = { 800, 32 };

	// Create unsorted data file "test3.in"
	Status		s;
	RID		rid;
	HeapFile	f("test3.in",s);
	assert(s == OK);
	for (int i=0; i<NUM_RECORDS; i++)
	{
		strcpy(rec.key, data1[i]);
		s = f.insertRecord((char*)&rec, sizeof(rec), rid);
		assert(s == OK);
	}

	// Sort "test1.in" into "test1.out"
	keySize = attrSize[0];
	sortOrder = Ascending;
	Sort 		sort("test3.in","test3.out",2,
					 attrType,attrSize,1,
					 Ascending,SORTPGNUM,s);
	f.deleteFile();
	if (s != OK)
	{
		minibase_errors.show_errors();
		minibase_errors.clear_errors();
		return;
	}

	// check if "test3.out" is sorted correctly
	HeapFile	f2("test3.out",s);
	if (s != OK)
	{
		cout << "Test3 -- OOPS! test3.out not created\n";
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
		  cout << "Test3 -- OOPS! test1.out not sorted" << endl;
			s = FAIL;
		}
		count++;
	}
	if (count != NUM_RECORDS)
		cout << "Test3 -- OOPS! Sorting Failed\n";
	else
		cout << "Test3 -- Sorting OK\n";
	f2.deleteFile();
}

//-------------------------------------------------------------
// test4
//-------------------------------------------------------------
void test4()
{
	struct R1 {
		char	key [32];
	} rec; 

	AttrType 	attrType[] = { attrString };
	short		attrSize[] = { 32 };

	// Create unsorted data file "test2.in"
	Status		s;
	RID		rid;
	HeapFile	f("test4.in",s);
	assert(s == OK);
	for (int i=0; i<NUM_RECORDS; i++)
	{
		strcpy(rec.key, data2[i]);
		s = f.insertRecord((char*)&rec,REC_LEN2,rid);
		assert(s == OK);
	}

	// Sort "test4.in" into "test4.out"
	keySize = attrSize[0];
	sortOrder = Ascending;
	Sort 		sort("test4.in","test4.out",1,attrType,attrSize,0,Ascending,SORTPGNUM,s);
	f.deleteFile();
	if (s != OK)
	{
		minibase_errors.show_errors();
		minibase_errors.clear_errors();
		return;
	}

	// check if "test2.out" is sorted correctly
	HeapFile	f2("test4.out",s);
	if (s != OK)
	{
		cout << "Test4 -- OOPS! test4.out not created\n";
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
			cout << "Test4 -- OOPS! test4.out not sorted\n";
			s = FAIL;
		}
		count++;
	}
	if (count != NUM_RECORDS)
		cout << "Test4 -- OOPS! Sorting Failed\n";
	else
		cout << "Test4 -- Sorting OK\n";
	f2.deleteFile();
}

//-------------------------------------------------------------
// test5
//-------------------------------------------------------------
void test5()
{
	struct R1 {
		char	key [32];
		char	filler[800];
	} rec; 

	AttrType 	attrType[] = { attrString, attrString };
	short		attrSize[] = { 32, 800 };

	// Create unsorted data file "test3.in"
	Status		s;
	RID		rid;
	HeapFile	f("test5.in",s);
	assert(s == OK);
	for (int i=0; i<NUM_RECORDS; i++)
	{
		strcpy(rec.key, data1[i]);
		s = f.insertRecord((char*)&rec,REC_LEN1,rid);
		assert(s == OK);
	}

	// Sort "test5.in" into "test3.out"
	keySize = attrSize[0];
	sortOrder = Descending;
	Sort 		sort("test5.in","test5.out",2,attrType,attrSize,0,Descending,SORTPGNUM,s);
	f.deleteFile();
	if (s != OK)
	{
		minibase_errors.show_errors();
		minibase_errors.clear_errors();
		return;
	}

	// check if "test5.out" is sorted correctly
	HeapFile	f2("test5.out",s);
	if (s != OK)
	{
		cout << "Test5 -- OOPS! test5.out not created\n";
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
		if (strcmp(rec.key,data3[count]) != 0)
		{
			cout << "Test5 -- OOPS! test5.out not sorted\n";
			s = FAIL;
		}
		count++;
	}
	if (count != NUM_RECORDS)
		cout << "Test5 -- OOPS! Sorting Failed\n";
	else
		cout << "Test5 -- Sorting OK\n";
	f2.deleteFile();
}


//-------------------------------------------------------------
// test6
//-------------------------------------------------------------
void test6()
{
	struct R1 {
		char	key [32];
	} rec; 

	AttrType 	attrType[] = { attrString };
	short		attrSize[] = { 32 };

	// Create unsorted data file "test4.in"
	Status		s;
	RID		rid;
	HeapFile	f("test6.in",s);
	assert(s == OK);
	for (int i=0; i<NUM_RECORDS; i++)
	{
		strcpy(rec.key, data3[i]);
		s = f.insertRecord((char*)&rec,REC_LEN2,rid);
		assert(s == OK);
	}

	// Sort "test4.in" into "test4.out"
	keySize = attrSize[0];
	sortOrder = Descending;
	Sort 		sort("test6.in","test6.out",1,attrType,attrSize,0,Descending,SORTPGNUM,s);
	f.deleteFile();
	if (s != OK)
	{
		minibase_errors.show_errors();
		minibase_errors.clear_errors();
		return;
	}

	// check if "test6.out" is sorted correctly
	HeapFile	f2("test6.out",s);
	if (s != OK)
	{
		cout << "Test6 -- OOPS! test4.out not created\n";
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
		if (strcmp(rec.key,data3[count]) != 0)
		{
			cout << "Test6 -- OOPS! test6.out not sorted\n";
			s = FAIL;
		}
		count++;
	}
	if (count != NUM_RECORDS)
		cout << "Test6 -- OOPS! Sorting Failed\n";
	else
		cout << "Test6 -- Sorting OK\n";
	f2.deleteFile();
}


