#include <stdlib.h>
#include <iostream>
#include <vector>
using namespace std;

const int BLOCK_SIZE = 2;	//���̿��СΪ2K
const int BLOCK_NUM = 500;	//���̿�����Ϊ1-500
const int MAXNUM_FILES = 5000;	//�����ļ���С

//���̿�ṹ��
struct block {
	int no;
	int size;
	int next;
};

class fat_entry {
public:
	string file_name;
	int no;
	vector<int> block_allocated;

	fat_entry(const string& file_name, int no, vector<int> blocks) {
		this->file_name.assign(file_name);
		this->no = no;
		this->block_allocated = blocks;
	}

	//���ظ�ֵ������
	fat_entry& operator=(const fat_entry& fat) {
		this->file_name = fat.file_name;
		this->no = fat.no;
		this->block_allocated = fat.block_allocated;
		return *this;
	}
};

struct disk_controller {
	int free_ptr = 0;
	block blocks[BLOCK_NUM + 1];
	vector<fat_entry> fat_table;
	int fat_ptr;	//�ļ������ָ��
	int remain;		//ʣ����п�����
}controller;

void init() {
	controller.free_ptr = 1;	//��ǰ���п�ָ��
	for (int i = 1; i <= BLOCK_NUM; i++) {
		controller.blocks[i].next = i + 1;	//��������˳����
		controller.blocks[i].no = i;
		controller.blocks[i].size = BLOCK_SIZE;
	}
	controller.blocks[BLOCK_NUM].next = 0;	
	controller.remain = BLOCK_NUM;			//��ǰ��ʣ500�����п�
	controller.fat_ptr = 0;					//��ǰ�ļ�����
}

vector<int> create_blocks(int allocated_block_num) {
	vector<int> allocated_blocks;
	int  allocated_ptr = controller.free_ptr;	//ָ��Ҫ������Ŀ�
	int free_ptr = controller.free_ptr;			//������п�ָ��

	for (int i = 0; i < allocated_block_num; i++) {
		free_ptr = controller.blocks[free_ptr].next;	//free_ptrָ��Ҫ����Ŀ����ڵ���һ��
		controller.blocks[allocated_ptr].next = 0;	//�ı�ÿ��ָ�򣬲���ָ�����ڵ���һ��
		allocated_blocks.push_back(controller.blocks[allocated_ptr].no);	//��vector����ĩβ����ÿ�
		allocated_ptr = free_ptr;
	}
	controller.free_ptr = free_ptr;	//���п�ָ��ָ����һ���п�
	controller.remain -= allocated_block_num;	//����ʣ����п�����
	return allocated_blocks;
}

//�����ļ������ļ���С
bool create_file(string file_name, double size) {

	//����ļ���Ҫ�Ŀ��С����ʣ����п��������򴴽��ļ�ʧ��
	//if (size / BLOCK_SIZE > controller.remain) return false;

	//������Ҫ������ٿ� �����BLOCK_SIZE�ı�����ϵ������Ҫ+1��������Ҫ�����һ����
	int allocated_block_num = size / BLOCK_SIZE;
	allocated_block_num += (size - allocated_block_num * BLOCK_SIZE);

	//����ļ���Ҫ�Ŀ��С����ʣ����п��������򴴽��ļ�ʧ��
	if (allocated_block_num > controller.remain) return false;

	//��vector������ű�����Ŀ�
	vector<int> allocated_blocks = create_blocks(allocated_block_num + 1);	//�׿���Ϊ����
	int index_block = allocated_blocks[0];	//��¼�÷�������׿�ĵ�ַ
	allocated_blocks.erase(allocated_blocks.begin());	//ɾ���÷�����е��׿�

	//��������¼�µ��ļ���
	fat_entry fat = fat_entry(file_name, index_block, allocated_blocks);
	controller.fat_table.push_back(fat);
	controller.fat_ptr++;
	return true;
}
//�ͷ��ļ���
int free_blocks(vector<int> blocks) {
	for (int i = 0; i < blocks.size(); i++) {
		controller.blocks[blocks[i]].next = controller.free_ptr;
		controller.free_ptr = blocks[i];
	}
	return 0;
}

//ɾ���ļ����ͷſ��п�
bool delete_file(string file_name) {
	fat_entry fat("", 0, vector<int>());
	for (int ptr = 0; ptr < controller.fat_ptr; ptr++) {
		if (file_name == controller.fat_table[ptr].file_name) {
			fat = controller.fat_table[ptr];
			controller.fat_table.erase(controller.fat_table.begin() + ptr);
			controller.fat_ptr--;
			break;
		}
	}

	if (fat.no == 0) return false;

	fat.block_allocated.push_back(fat.no);
	free_blocks(fat.block_allocated);
	return true;
}

//����ļ������״̬
void display_fat() {
	for (int i = 0; i < controller.fat_ptr; i++) {
		cout << "file_name" << controller.fat_table[i].file_name << "\t\t" << "index_block:" << controller.fat_table[i].no << "\t\t";
		cout << "blocks:";
		for (int j = 0; j < controller.fat_table[i].block_allocated.size(); j++) {
			cout << controller.fat_table[i].block_allocated[j] << " ";
		}
		cout << endl;
	}
}

void display_free() {
	for (int i = controller.free_ptr; i; i = controller.blocks[i].next) {
		cout << controller.blocks[i].no << " ";
	}
	cout << endl;
}

int main() {
	init();	//��ʼ��

	//�������50���ļ�
	for (int i = 0; i < 50; i++) {
		int r = rand() % 9 + 2;	//r��ʾ�ļ���С����ΧΪ2-11K
		char name[10];
		sprintf(name, "%d.txt", i);
		create_file(name, r);
	}

	//����ļ������
	cout << "FILE ALLACATION TABLE:" << endl;
	display_fat();

	//ɾ�������ļ�
	for (int i = 1; i <= 50; i += 2) {
		char name[10];
		sprintf(name, "%d.txt", i);
		delete_file(name);
	}

	//����ļ������
	cout << "FILE ALLACATION TABLE:" << endl;
	display_fat();

	//����5���ļ�
	create_file("A.txt", 7);
	create_file("B.txt", 5);
	create_file("C.txt", 2);
	create_file("D.txt", 9);
	create_file("E.txt", 3.5);

	//����ļ������
	cout << "FILE ALLACATION TABLE:" << endl;
	display_fat();

	//������п�״̬
	cout << "FREE BLOCKS" << endl;
	display_free();

	char ch;
	cin >> ch;

	//system("pause");
	return 0;
}