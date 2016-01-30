#include <stdio.h>
#include <stdlib.h>
#include <ios>

enum AttrType {
  attrInteger,
  attrString
};

enum SortOrder {
  Ascending,
  Descending
};

int offset;
int fieldLength;
AttrType attrType;
SortOrder sortOrder;

int tupleCmp(const void *pRec1, const void *pRec2) 
{
  int result;

  char *rec1 = (char *)pRec1;
  char *rec2 = (char *)pRec2;
  
  if (attrType = attrInteger) 
	result = *(int *)(rec1 + offset) - *(int *)(rec2 + offset);
  else
	result = (strncmp(&rec1[offset], &rec2[offset], fieldLength));
	
  if (result < 0)
	if (sortOrder == Ascending)
	  return -1;
	else
	  return 1;
  else 
	if (result > 0)
	  if (sortOrder == Ascending)
		return 1;
	  else 
		return -1;
	else 
	  return 0;
}

int main() {
  struct StudentRec {
	int sid;
	char name[20];
	int age;
  };

  StudentRec stu[10];
  
  for (int i=0; i< 10; i++) {
	stu[i].sid = i;
	sprintf(stu[i].name, "Student %c", i % 5 + 65);
	stu[i].age = 20-i;
  }

  for (int i=0; i< 10; i++) 
	cout << stu[i].sid << "," << stu[i].name << "," << stu[i].age << endl;

  offset = 0;
  fieldLength = sizeof(int);
  sortOrder = Descending;
  attrType = attrInteger;

  qsort(stu, 10, sizeof(StudentRec), tupleCmp);
  cout << "Sort with the sid in descnding order" << endl;
  for (int i=0; i< 10; i++) 
	cout << stu[i].sid << "," << stu[i].name << "," << stu[i].age << endl;


  offset = 0;
  fieldLength = sizeof(int);
  sortOrder = Ascending;
  attrType = attrInteger;
  qsort(stu, 10, sizeof(StudentRec), tupleCmp);
  cout << "Sort with the sid in ascending order" << endl;
  for (int i=0; i< 10; i++) 
	cout << stu[i].sid << "," << stu[i].name << "," << stu[i].age << endl;


  offset = 4;                       // The offset of name attribute
  fieldLength = 20;
  sortOrder = Ascending;
  attrType = attrString;
  qsort(stu, 10, sizeof(StudentRec), tupleCmp);
  cout << "Sort with the name in ascending order" << endl;
  for (int i=0; i< 10; i++) 
	cout << stu[i].sid << "," << stu[i].name << "," << stu[i].age << endl;

  return 0;
}
