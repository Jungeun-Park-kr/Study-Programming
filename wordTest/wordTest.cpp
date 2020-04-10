/* 영어 단어 시험지 프로그램
 * 2020.03.01
 * 입력 : 단어장 파일 이름, 시험칠 단어의 개수(0입력시 모든 단어)
 * 출력 : 성공 유무
 * 결과 : 결과물이 담긴 test.txt파일과 answer.txt파일 생성
 * 프로그램 설명 : 
 *  -영어 단어와 뜻이 적혀있는 단어장 파일(입력)을 읽어와서 원하는 개수만큼 랜덤으로
 *    단어 시험지와 답안지를 생성한다.
 *  -test.txt 파일에는 영어 원어만을 넣어서 시험지를 만들고,
 *  -answer.txt 파일에는 시험지와 동일한 순서로 뜻이 적힌 답안지를 만든다.
 *  -랜덤으로 시험지를 생성할 때, 중복이 되지 않아야 한다.
 * 
*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

int main() {
	int num, n, size,max_size;
	vector<string> all;
	char sen[200];
	bool visited[999];
	memset(visited, false, 999);
	string title,tmp, delimiter="-", token;

	cout << "단어장 파일 이름을 입력하세요 :";
	cin >> title;
	ifstream sheet(title, ios::in);
	if (sheet.is_open()) {
		ofstream test, answer;
		test.open("test.txt");
		answer.open("answer.txt");
		srand((unsigned int)time(NULL));

		cout << "test할 단어의 개수(0입력시 모든 단어 test) : ";
		cin >> n;

		//단어 개수만큼 string 배열
		while (!sheet.eof()) {
			sheet.getline(sen, 200);
			all.push_back(sen);
		}
		max_size = all.size();
		if (n == 0) { //0입력시 모든 단어 사용
			n = max_size;
		}

		while (n--) {
			num = rand() % max_size; //num난수생성
			while (visited[num]) { //중복제거
				num = rand() % max_size;
			}
			visited[num] = true;
			tmp = all[num];

			//영어까지만 test파일로 input
			size_t pos = 0;
			while ((pos = tmp.find(delimiter)) != string::npos) {
				token = tmp.substr(0, pos);
				test << token << endl;
				answer << token << "-"; //영어단어 answer.txt파일로
				tmp.erase(0, pos + delimiter.length());
			}
			//품사와 뜻 answer.txt파일로 output
			answer << tmp << endl;
		}
		cout << "단어 시험지 생성을 성공하였습니다." << endl;
	}
	else {
		cout << "단어장 파일을 불러올 수 없습니다." << endl;
		return -1;
	}
	

	///*파일의 내용 순서대로 시험지와 답안지 생성*/
	//if(sheet.is_open){
	//	while (!sheet.eof()) {
	//		//영어 : 시험지 파일
	//		sheet.getline(sen, 100, '-');
	//		test << sen;
	//		answer<<sen;
	//		test << '\n';
	//		//뜻 : 답안지 파일 
	//		sheet.getline(sen, 100);
	//		answer << sen;
	//		answer << '\n';
	//	}
	//}
	//else {
	//	cout << "파일 열기 에러";
	//	return -1;
	//}

	return 0;
}