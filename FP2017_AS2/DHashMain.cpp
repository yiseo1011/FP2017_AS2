#pragma warning(disable: 4996)

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>

#define MIN_ORDER 2
#define KEY_LENGTH 9
#define BUCKET_SIZE 128

using namespace std;
unsigned int LIST_SIZE;

class StElement {
	char name[20];
	unsigned int studentID;
	float score;
	unsigned int advisorID;
public:
	// set class elements
	void setName(char* _name) { strcpy( name, _name ); }
	void setSID(unsigned int _studentID) { studentID = _studentID; }
	void setScore(float _score) { score = _score; };
	void setAID(unsigned int _advisorID) { advisorID = _advisorID; }

	// get class elements
	char* getName() { return name; }
	unsigned int getSID() { return studentID; }
	float getScore() { return score; }
	unsigned int getAID() { return advisorID; }

	// functions
	void setElement(char* , unsigned int , float , unsigned int );
};
void StElement::setElement(char* _name, unsigned int _studentID, float _score, unsigned int _advisorID) {
	setName(_name);
	setSID(_studentID);
	setScore(_score);
	setAID(_advisorID);
}
// 제곱을 해주는 함수
int pow_2(int p){
	int i;
	int x = 1;
	for (i = 0; i<p; i++)
		x *= 2;
	return x;
}
// pseudokey를 만들어서 return 한다.
// KEY 길이만큼 제곱 후 mod 연산을 통해 하위 KEY_LENGTH 키만큼 잘라낸다
int makePseudokey(int key){
	return key % pow_2(KEY_LENGTH);
}
//  상위 n bit를 돌려준다. 
int foreget(int k, int n){
	return k / pow_2(KEY_LENGTH - n);
}
typedef struct Leaf{
	int header;
	int count;  // 저장된 record의 갯수
	StElement** pRecord; // 저장된 record의 주소를 가리키는 pointer의 배열
} leaf;
typedef struct Directory{
	int header;
	int divCount;  // directory의 header와 같은 header를 가진 leaf pair의 갯수
	leaf** entry;
} directory;
StElement* retrieval(int key, directory* dir){
	int i;
	int pseudokey = makePseudokey(key);    // pseudokey 생성
	int index = foreget(pseudokey, dir->header);  // entry index
	leaf* bucket = dir->entry[index];
	for (i = 0; i<bucket->count; i++)     // 해당 leaf에서 찾고자하는 key를 찾는다. 
	{
		StElement* ptr = dir->entry[index]->pRecord[i];
		if (ptr->getSID() == key)
			return ptr;        // 찾으면 record의 주소를 리턴
	}
	return NULL;         // 없으면 NULL을 리턴
}

int insertRecord(StElement* rec, directory* dir){
	int i, ind;
	int key = rec->getSID();        // 삽입되는 record의 key
	int pseudokey = makePseudokey(key);    // pseudokey
	int index = foreget(pseudokey, dir->header); // entry의 번호를 찾는다. 
	leaf* bucket = dir->entry[index];    // 삽입하고자 하는 leaf

	// bucket을 확인해서 삽입하고자 하는 키가 이미 존재하는지 확인
	for (i = 0; i<bucket->count; i++){
		if (bucket->pRecord[i]->getSID() == key){
			printf("\n\nThe key is already exist!!!\n");
			return 0;  // 비정상 종료
		}
	}

	// bucket에 저장할 공간이 남아있다면 빈칸에 저장한다. 
	if (bucket->count < BUCKET_SIZE){
		bucket->pRecord[bucket->count] = rec;
		bucket->count++;
		return 1;   // 정상종료
	}

	// bucket이 다 찼다면 overflow 처리
	while (1){
		int n;
		leaf* newBucket;  // 새로 만들 bucket;
						  //////  d < t+1인경우 => 먼저 디렉토리를 두배 늘린다 
		if (dir->header < bucket->header + 1){
			int numEntry;    // 새로 만들어질 entry의 수
			leaf** newEntry;
			dir->header++;
			numEntry = pow_2(dir->header);
			newEntry = (leaf**)malloc(sizeof(leaf*)*numEntry);
			for (i = 0; i<numEntry / 2; i++)  // entry를 늘어난 만큼 분배
				newEntry[i * 2] = newEntry[i * 2 + 1] = dir->entry[i];
			dir->divCount = 0;
			free(dir->entry);
			dir->entry = newEntry;
		}
		// 이제 overflow가 생긴 leaf를 split한다. 
		// 새로운 bucket 생성
		newBucket = (leaf*)malloc(sizeof(leaf));
		newBucket->header = bucket->header;
		newBucket->count = 0;
		newBucket->pRecord = (StElement**)malloc(sizeof(StElement*) * BUCKET_SIZE);

		// bucket내의 record 분배
		bucket->count = 0;
		for (i = 0; i<BUCKET_SIZE; i++)
		{
			if (foreget(makePseudokey(bucket->pRecord[i]->getSID()), bucket->header + 1) % 2 == 0)
			{
				bucket->pRecord[bucket->count] = bucket->pRecord[i];
				bucket->count++;
			}
			else
			{
				newBucket->pRecord[newBucket->count] = bucket->pRecord[i];
				newBucket->count++;
			}
		}
		bucket->header++;
		newBucket->header++;
		if (bucket->header == dir->header)
			dir->divCount++;

		// entry -> leaf 로의 pointer 조절
		n = pow_2(dir->header - bucket->header + 1);      // 나누어질 entry 수
		ind = foreget(makePseudokey(key), bucket->header - 1) * n;   // 나누어질 entry중 첫번째 entry의 index

		for (i = 0; i<n / 2; i++, ind++)        // 두개의 bucket으로 나눔
			dir->entry[ind] = bucket;
		for (i = 0; i<n / 2; i++, ind++)
			dir->entry[ind] = newBucket;

		// 이제 다시 삽입될 노드를 저장한다. 
		index = foreget(pseudokey, dir->header);
		bucket = dir->entry[index];
		if (bucket->count < BUCKET_SIZE) // 저장할 곳의 bucket이 full이 아니면 저장
		{
			bucket->pRecord[bucket->count] = rec;
			bucket->count++;
			return 1;   // 정상종료
		}
		// 저장할 곳이 full이 아니면 2의 처음으로 돌아가서 다시 directory를 늘린다. 
	}
}
void printDB(directory* dir, ofstream& os){
	int i, j;
	leaf* cBucket;
	leaf* pBucket = NULL;
	for (i = 0; i<pow_2(dir->header); i++){
		int c = i;
		/*
		for (k = pow_2(dir->header - 1); k>0; k /= 2){
			// os << c / k;
			c = c%k;
		}
		*/
		os << c;
		cBucket = dir->entry[i];
		if (cBucket != pBucket){
			// os << cBucket->header << "\t";
			os << "\t" << cBucket->count << "\t" << endl;
			for (j = 0; j<cBucket->count; j++){
				/*
					Data structure :
					char name[20];
					unsigned int studentID;
					float score;
					unsigned int advisorID;
				*/
				os << cBucket->pRecord[j]->getName() << ", ";
				os << cBucket->pRecord[j]->getSID() << ", ";
				os << cBucket->pRecord[j]->getScore() << ", ";
				os << cBucket->pRecord[j]->getAID() << endl;
			}
		}
		else {
			os << "\t" << "P" << "\t" << endl;
		}
		pBucket = cBucket;
	}
}

void printTable(vector<StElement> origin, ofstream& os){
	FILE *fp;
	fp = fopen("Students.hash", "wb");
	// os.open("Students.hash");
	int getInt;
	int getPseudo;
	for (int i = 0; i < LIST_SIZE; i++) {
		StElement* rec = &origin[i];
		getInt = rec->getSID();
		fwrite(&getInt, sizeof(int), 1, fp);
		fwrite(" ", sizeof(" "), 1, fp);
		getPseudo = makePseudokey(rec->getSID());
		fwrite(&getPseudo, sizeof(int), 1, fp);
		fwrite("\n", sizeof("\n"), 1, fp);
		// os << rec->getSID() << " " << makePseudokey(rec->getSID()) << endl;
	}
	// os.close();
}
StElement getToken(char* convStr) {
	StElement bufElement;
	char* tokList;

	tokList = strtok(convStr, ",");
	bufElement.setName(tokList);

	tokList = strtok(NULL, ",");
	bufElement.setSID(atoi(tokList));

	tokList = strtok(NULL, ",");
	bufElement.setScore(atof(tokList));

	tokList = strtok(NULL, ",");
	bufElement.setAID(atoi(tokList));

	return bufElement;
}

int main() {

	ifstream is;
	ofstream os;
	int target = 0;
	is.open("sampleData.csv");
	os.open("Students.DB");
	if (is.is_open() == false){
		cout << "File open Failed" << endl; return 1;
	}

	char* convStr;
	string bufStr;
	StElement bufElement;
	vector<StElement> oData;
	vector<unsigned int> hashList;

	// get LIST_SIZE
	getline(is, bufStr);
	convStr = new char[sizeof(bufStr)];
	
	strcpy(convStr, bufStr.c_str());
	convStr = strtok(convStr, ",");
	LIST_SIZE = atoi(convStr);

	for (int i = 0; i < LIST_SIZE; i++) {
		getline(is, bufStr);
		strcpy(convStr, bufStr.c_str());
		bufElement = getToken(convStr);
		oData.push_back(bufElement);
	}
	
	directory* dir;
	dir = (directory*)malloc(sizeof(directory));
	dir->header = MIN_ORDER;
	dir->divCount = pow_2(MIN_ORDER - 1);
	dir->entry = (leaf**)malloc(sizeof(leaf*) * pow_2(MIN_ORDER));
	for (int i = 0; i<pow_2(MIN_ORDER); i++)	{
		dir->entry[i] = (leaf*)malloc(sizeof(leaf));
		dir->entry[i]->header = 2;
		dir->entry[i]->count = 0;
		dir->entry[i]->pRecord = (StElement**)malloc(sizeof(StElement*)*BUCKET_SIZE);
	}

	for(int i = 0; i < LIST_SIZE; i++){
		StElement* rec = &oData[i];
		insertRecord(rec, dir);
		/* printf("\n***  key : %d, name : %s 인 record를 저장, pseudokey : %d ***\n",
			rec->getSID(), rec->getName(), makePseudokey(rec->getSID())); */
	}
	printDB(dir, os);
	os.close();
	printTable(oData, os);
	is.close();
	return 0;
}
