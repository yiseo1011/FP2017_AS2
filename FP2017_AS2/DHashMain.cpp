#pragma warning(disable: 4996)

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "md5.h"

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
class Bucket {
	// bucket address table -> vector<Bucket>
	vector<StElement> curBucket;

public:

};

void StElement::setElement(char* _name, unsigned int _studentID, float _score, unsigned int _advisorID) {
	setName(_name);
	setSID(_studentID);
	setScore(_score);
	setAID(_advisorID);
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
string md5(const string strMd5) {
	md5_state_t state;
	md5_byte_t digest[16];
	char hex_output[16 * 2 + 1];
	int di;
	
	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)strMd5.c_str(), strMd5.length());
	md5_finish(&state, digest);
	for (di = 0; di < 16; ++di)
		sprintf(hex_output + di * 2, "%02x", digest[di]);

	return hex_output;
}
string GetBinaryFromHex(string sHex){
	string sReturn = "";
	for (int i = 0; i < sHex.length(); ++i){
		switch (sHex[i]){
			case '0': sReturn.append("0000"); break;
			case '1': sReturn.append("0001"); break;
			case '2': sReturn.append("0010"); break;
			case '3': sReturn.append("0011"); break;
			case '4': sReturn.append("0100"); break;
			case '5': sReturn.append("0101"); break;
			case '6': sReturn.append("0110"); break;
			case '7': sReturn.append("0111"); break;
			case '8': sReturn.append("1000"); break;
			case '9': sReturn.append("1001"); break;
			case 'a': sReturn.append("1010"); break;
			case 'b': sReturn.append("1011"); break;
			case 'c': sReturn.append("1100"); break;
			case 'd': sReturn.append("1101"); break;
			case 'e': sReturn.append("1110"); break;
			case 'f': sReturn.append("1111"); break;
		}
	}
	return sReturn;
}
vector<string> dataToHash(vector<StElement> origin) {
	string convHash;
	vector<string> myhash;
	vector<StElement>::iterator itor;

	for (itor = origin.begin(); itor != origin.end(); itor++) {
		to_string(itor->getSID());
		convHash = md5(convHash);
		convHash = GetBinaryFromHex(convHash);
		myhash.push_back(convHash);
		cout << convHash << endl;
	}

	return myhash;
}

int main() {
	
	ifstream is;
	is.open("sampleData.csv");
	if (is.is_open() == false){
		cout << "File open Failed" << endl; return 1;
	}

	char* convStr;
	string bufStr;
	StElement bufElement;
	vector<StElement> oData;
	vector<string> ohash;

	// get LIST_SIZE
	getline(is, bufStr);
	// fatal error : heap corruption
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

	ohash = dataToHash(oData);
	is.close();
	return 0;
}