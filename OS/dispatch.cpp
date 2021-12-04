#include <stdlib.h>
#include <iostream>
#include <vector>
using namespace std;

const int BLOCK_SIZE = 2;	//磁盘块大小为2K
const int BLOCK_NUM = 500;	//磁盘块序列为1-500
const int MAXNUM_FILES = 5000;	//最大的文件大小

//磁盘块结构体
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

	//重载赋值操作符
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
	int fat_ptr;	//文件分配表指针
	int remain;		//剩余空闲块数量
}controller;

void init() {
	controller.free_ptr = 1;	//当前空闲块指针
	for (int i = 1; i <= BLOCK_NUM; i++) {
		controller.blocks[i].next = i + 1;	//将各个块顺序存放
		controller.blocks[i].no = i;
		controller.blocks[i].size = BLOCK_SIZE;
	}
	controller.blocks[BLOCK_NUM].next = 0;	
	controller.remain = BLOCK_NUM;			//当前还剩500个空闲块
	controller.fat_ptr = 0;					//当前文件索引
}

vector<int> create_blocks(int allocated_block_num) {
	vector<int> allocated_blocks;
	int  allocated_ptr = controller.free_ptr;	//指向将要被分配的块
	int free_ptr = controller.free_ptr;			//定义空闲块指针

	for (int i = 0; i < allocated_block_num; i++) {
		free_ptr = controller.blocks[free_ptr].next;	//free_ptr指向将要分配的块相邻的下一块
		controller.blocks[allocated_ptr].next = 0;	//改变该块的指向，不再指向相邻的下一块
		allocated_blocks.push_back(controller.blocks[allocated_ptr].no);	//在vector容器末尾加入该块
		allocated_ptr = free_ptr;
	}
	controller.free_ptr = free_ptr;	//空闲块指针指向下一空闲块
	controller.remain -= allocated_block_num;	//更新剩余空闲块数量
	return allocated_blocks;
}

//传入文件名及文件大小
bool create_file(string file_name, double size) {

	//如果文件需要的块大小大于剩余空闲块数量，则创建文件失败
	//if (size / BLOCK_SIZE > controller.remain) return false;

	//计算需要分配多少块 如果是BLOCK_SIZE的倍数关系，则不需要+1，否则需要多分配一个块
	int allocated_block_num = size / BLOCK_SIZE;
	allocated_block_num += (size - allocated_block_num * BLOCK_SIZE);

	//如果文件需要的块大小大于剩余空闲块数量，则创建文件失败
	if (allocated_block_num > controller.remain) return false;

	//用vector容器存放被分配的块
	vector<int> allocated_blocks = create_blocks(allocated_block_num + 1);	//首块作为索引
	int index_block = allocated_blocks[0];	//记录该分配块中首块的地址
	allocated_blocks.erase(allocated_blocks.begin());	//删除该分配块中的首块

	//创建并记录新的文件块
	fat_entry fat = fat_entry(file_name, index_block, allocated_blocks);
	controller.fat_table.push_back(fat);
	controller.fat_ptr++;
	return true;
}
//释放文件块
int free_blocks(vector<int> blocks) {
	for (int i = 0; i < blocks.size(); i++) {
		controller.blocks[blocks[i]].next = controller.free_ptr;
		controller.free_ptr = blocks[i];
	}
	return 0;
}

//删除文件并释放空闲块
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

//输出文件分配表状态
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
	init();	//初始化

	//随机创建50个文件
	for (int i = 0; i < 50; i++) {
		int r = rand() % 9 + 2;	//r表示文件大小，范围为2-11K
		char name[10];
		sprintf(name, "%d.txt", i);
		create_file(name, r);
	}

	//输出文件分配表
	cout << "FILE ALLACATION TABLE:" << endl;
	display_fat();

	//删除奇数文件
	for (int i = 1; i <= 50; i += 2) {
		char name[10];
		sprintf(name, "%d.txt", i);
		delete_file(name);
	}

	//输出文件分配表
	cout << "FILE ALLACATION TABLE:" << endl;
	display_fat();

	//创建5个文件
	create_file("A.txt", 7);
	create_file("B.txt", 5);
	create_file("C.txt", 2);
	create_file("D.txt", 9);
	create_file("E.txt", 3.5);

	//输出文件分配表
	cout << "FILE ALLACATION TABLE:" << endl;
	display_fat();

	//输出空闲块状态
	cout << "FREE BLOCKS" << endl;
	display_free();

	char ch;
	cin >> ch;

	//system("pause");
	return 0;
}