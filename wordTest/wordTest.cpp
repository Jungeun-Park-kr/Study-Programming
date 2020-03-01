/* ���� �ܾ� ������ ���α׷�
 * 2020.03.01
 * �Է� : �ܾ��� ���� �̸�, ����ĥ �ܾ��� ����(0�Է½� ��� �ܾ�)
 * ��� : ���� ����
 * ��� : ������� ��� test.txt���ϰ� answer.txt���� ����
 * ���α׷� ���� : 
 *  -���� �ܾ�� ���� �����ִ� �ܾ��� ����(�Է�)�� �о�ͼ� ���ϴ� ������ŭ ��������
 *    �ܾ� �������� ������� �����Ѵ�.
 *  -test.txt ���Ͽ��� ���� ����� �־ �������� �����,
 *  -answer.txt ���Ͽ��� �������� ������ ������ ���� ���� ������� �����.
 *  -�������� �������� ������ ��, �ߺ��� ���� �ʾƾ� �Ѵ�.
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

	cout << "�ܾ��� ���� �̸��� �Է��ϼ��� :";
	cin >> title;
	ifstream sheet(title, ios::in);
	if (sheet.is_open()) {
		ofstream test, answer;
		test.open("test.txt");
		answer.open("answer.txt");
		srand((unsigned int)time(NULL));

		cout << "test�� �ܾ��� ����(0�Է½� ��� �ܾ� test) : ";
		cin >> n;

		//�ܾ� ������ŭ string �迭
		while (!sheet.eof()) {
			sheet.getline(sen, 200);
			all.push_back(sen);
		}
		max_size = all.size();
		if (n == 0) { //0�Է½� ��� �ܾ� ���
			n = max_size;
		}

		while (n--) {
			num = rand() % max_size; //num��������
			while (visited[num]) { //�ߺ�����
				num = rand() % max_size;
			}
			visited[num] = true;
			tmp = all[num];

			//��������� test���Ϸ� input
			size_t pos = 0;
			while ((pos = tmp.find(delimiter)) != string::npos) {
				token = tmp.substr(0, pos);
				test << token << endl;
				answer << token << "-"; //����ܾ� answer.txt���Ϸ�
				tmp.erase(0, pos + delimiter.length());
			}
			//ǰ��� �� answer.txt���Ϸ� output
			answer << tmp << endl;
		}
		cout << "�ܾ� ������ ������ �����Ͽ����ϴ�." << endl;
	}
	else {
		cout << "�ܾ��� ������ �ҷ��� �� �����ϴ�." << endl;
		return -1;
	}
	

	///*������ ���� ������� �������� ����� ����*/
	//if(sheet.is_open){
	//	while (!sheet.eof()) {
	//		//���� : ������ ����
	//		sheet.getline(sen, 100, '-');
	//		test << sen;
	//		answer<<sen;
	//		test << '\n';
	//		//�� : ����� ���� 
	//		sheet.getline(sen, 100);
	//		answer << sen;
	//		answer << '\n';
	//	}
	//}
	//else {
	//	cout << "���� ���� ����";
	//	return -1;
	//}

	return 0;
}