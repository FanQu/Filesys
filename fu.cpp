#define _CRT_SECURE_NO_WARNINGS
#include "fu.h"

//ע�⣬WriteDir������д��dir_nameû�аѺ��油ȫ'\0' 
//��������qu.cpp 

char path_up[input_buffer_length] = { 0 };

_dir_block get_dirblock(int inode_id)//����dirblock�����棬ʹ��ǰ��Ҫ�ж��Ƿ�ΪĿ¼������Ҫ��֤Ŀ¼��<252 
{
	FILE *vfs = fopen(filename, "rb+");
	_dir_block block;
	int block_id = inodes[inode_id].i_blocks[0];
	long Position = DataBlkPos(block_id);
	for (int i = 0; i < 16; i++){
		fseek(vfs, Position + i * dir_size, SEEK_SET);
		fread(block.dirs[i].name, 252, 1, vfs);
		fread(&block.dirs[i].inode_id,sizeof(block.dirs[i].inode_id), 1, vfs);
	}
	fclose(vfs);
	return block;
}
_file_block get_fileblock(int inode_id)//����fileblock�����棬ʹ��fileblockǰ��Ҫ�ж��Ƿ�Ϊ�ļ�������Ҫ�����ļ�����С<252 
{
	_file_block fileblock;
	FILE *vfs = fopen(filename, "rb+");
	int block_id = inodes[inode_id].i_blocks[0];
	long Position = DataBlkPos(block_id);
	fseek(vfs, Position, SEEK_SET);
	fread(fileblock.data ,sizeof(fileblock.data), 1, vfs);
	fclose(vfs);
	return fileblock;
}

void write_fileblock_into_file(char str[],int block_id)//��block_id����дstr�����棬ÿ��ʹ��ǰ�豣֤���ļ�
{
	FILE *vfs = fopen(filename, "rb+");
	long Position = DataBlkPos(block_id);
	fseek(vfs, Position, SEEK_SET);
	int lenstr = (int)strlen(str);
	if (lenstr > 4096 || lenstr == 0) {
		lenstr = 4096;
	}
    fwrite(str ,sizeof(char), lenstr, vfs);
	char _zero[1] = { '\0' };
	for (int i = 0; i < datablk_size - lenstr; i++) {
		fwrite(_zero, sizeof(char), 1, vfs);
	}
	fclose(vfs);
	return;
}

void FileError(char path[]) {
	cout << path << " " << "is not a file." << endl;
}

void InodeFullError() {
	printf("All inodes are used\n");
}

int find_free_indbmp(){
	int i = 0;
	for (; (i < indbmp_size) && (sbks.inode_bitmap[i] != 0); i++);
	if (i == 4096){
		return -1;
	}
	return i;
}


void BlockFullError() {
	printf("All blocks are used\n");
}

int find_free_blkbmp(){
	int i = 0;
	for (; (i < blkbmp_size) && (sbks.block_bitmap[i] != 0); i++);
	if (i == 4096){
		return -1;
	}
	return i;
}

int find_free_dir_entry(int inode_id, char path[]) {
	if (inodes[inode_id].i_mode == 1){
		return -2;
	}
	int block_id = inodes[inode_id].i_blocks[0]; 
	_dir_block block1 = get_dirblock(inode_id);//�ҵ�block
	int i = 0;
	for (; (i<16)&&(block1.dirs[i].name[0] != '\0'); i++);
	if (i==16){
		return -1;
	}
	return i;
}

int find_position_dir_entry(int path_inode_id){//δ��path�жϣ����������ж�path����
	int uppath_inode_id = inodes[path_inode_id].fat_id;
	_dir_block block1 = get_dirblock(uppath_inode_id);//�ҵ�block
	int i = 0;
	for (; (i < 16) && (block1.dirs[i].inode_id != path_inode_id); i++);
	if (i==16){
		return -1;
	}
	return i;
}

int echo(char path[], char str[])//����path��'\0'��β��str����
{
	int i = DoEcho(path, str);
	if (i == -1) {
		FileError(path);
	}
	else if (i == -2) {
		NameLongError();
	}
	else if (i == -3) {
		PathError(path_up);
	}
	else if (i == -4) {
		DirError(path_up);
	}
	else if (i == 1) {
		FullError();
	}
	else if (i == 2) {
		InodeFullError();
	}
	else if (i == 3) {
		BlockFullError();
	}
	return i;
}

int cut_path_and_path_up(char path[], char str_name[])//echo��������-1�ļ���������0�ɹ�����path���Ϊpath_up��str_name
{
	int tmplen = (int)strlen(path);//·������
	int UpDirPos = -1;//�ϼ�Ŀ¼·����ֹλ�ã���'/'λ��
	bool flag = 0;//�ļ�Ϊ��׼��ʽ"a/b/c"��Ǳ�׼��ʽ"a"(�൱��"./a")ǰ��Ϊ1������0
	for (int i = tmplen; i >= 0; i--) {
		if (path[i] == '/') {
			flag = 1;
			UpDirPos = i;
			break;
		}
	}
	int lenname = tmplen;//str����
	if (flag) {
		lenname = tmplen - UpDirPos - 1;
	}
	if (lenname > 251) {//�ļ�������
		return -1;
	}
	strncpy(str_name, path + UpDirPos + 1, lenname);
	str_name[lenname] = '\0';
	//cout << "str_name: " << str_name << endl;
	if (flag) {
		strncpy(path_up, path, UpDirPos);
		path_up[UpDirPos] = '\0';
		//cout << "path_up: " << path_up << endl;//mark
	}
	else {
		path_up[0] = '.';
		path_up[1] = '\0';
	}

	strncpy(str_name, path + UpDirPos + 1, lenname);
	str_name[lenname] = '\0';
    //cout << "str_name: " << str_name << endl;
    return 0;

}

int DoEcho(char path[], char str[])//echo�ںˣ�-k·������+k�ռ����0�ɹ�
{
	int str_inode_id = GetPathInode(path);

	//��ΪĿ¼������-1
	if (str_inode_id > 0 && inodes[str_inode_id].i_mode == 0) {
		return -1;
	}
	//�������ڣ������ϼ�Ŀ¼�Ƿ����
	if (str_inode_id < 0) {
		char str_name[252];
		//������������-2
		if (cut_path_and_path_up(path, str_name) == -1) {
			return -2;
		}
		int upstr_inode_id = GetPathInode(path_up);
		//�ϼ������ڷ���-3
		if (upstr_inode_id<0) {
			return -3;
		}
		//�ϼ�Ϊ�ļ�����-4
		if (inodes[upstr_inode_id].i_mode == 1) {
			return -4;
		}
		//Ѱ�ҿ���λ��
		int str_position = find_free_dir_entry(upstr_inode_id, path_up);
		if (str_position < 0) {
			return 1;
		}
		str_inode_id = find_free_indbmp();
		if (str_inode_id < 0) {
			return 2;
		}
		int str_block_id = find_free_blkbmp();
		if (str_block_id < 0) {
			return 3;
		}//���϶�Ӧ���̿ռ����������
		 //��ʼ��
		WriteDir(str_name, str_position, upstr_inode_id, str_inode_id);
		inodes[str_inode_id] = _inode(str_inode_id, 1, 0, upstr_inode_id, str_block_id);
		//�������inode/block����Ϊ���Ǳ������ǿյ� 
		sbks.inode_bitmap[str_inode_id] = 1;
		sbks.block_bitmap[str_block_id] = 1;
		UpdateIndBmp(str_inode_id);
		UpdateBlkBmp(str_block_id);
		UpdateInode(str_inode_id);//����������Ϣ
	}
	//��������ļ�һ��������
	char full_str[4096];
	int filesize = 0;
	for (;(str[filesize] != '\0') && filesize < 4096; filesize++){
		full_str[filesize] = str[filesize];
	}
	//cout << filesize << " filesize\n";
	for (int i = filesize; i < 4096; i++){
		full_str[i] = '\0';
	}//��չstr����׼����
	inodes[str_inode_id].i_file_size = filesize;
	UpdateInode(str_inode_id);//����inode�е�fizesize
	write_fileblock_into_file(full_str,inodes[str_inode_id].i_blocks[0]);//д��str
	return 0;
}
 
int cat(char path[]) {//��ȡpath·�����ļ���-2�����ڣ�-1Ŀ¼��0�ɹ�
	char str[4097];
	int i = DoCat(path, str);
	if (i == -2) {
		PathError(path);
	}
	else if (i == -1) {
		FileError(path);
	}
	else {
		printf("%s\n", str);
	}
	return i;
}

int DoCat(char path[],char data[])//cat�ںˣ�����·��path���޸�dataΪpath�����ݣ�����ֵ-2�����ڣ�-1Ŀ¼��0�ɹ�
{
	int str_inode_id = GetPathInode(path); 
	if (str_inode_id<0){
		return -2;
	}//·��������
	if (inodes[str_inode_id].i_mode == 0) {
		return -1;
	}//·��ΪĿ¼
	_file_block block = get_fileblock(str_inode_id);
	int i = 0;
	for (; i < 4096 && block.data[i] != '\0'; i++) {
		data[i] = block.data[i];
	}
	data[i] = '\0';
	return 0;
}

int delete_file(char path[]) {//ɾ��ĳinode_id���ļ���-2�����ڣ�-1�ļ��У�0�ɹ�
	int path_inode_id = GetPathInode(path);
	int i = delete_file(path_inode_id);
	return i;
}

int delete_file(int path_inode_id)//ɾ��ĳinode_id���ļ���-2�����ڣ�-1�ļ��У�0�ɹ�
{
	if (path_inode_id < 0) {
		return -2;
	}//·��������
	if (inodes[path_inode_id].i_mode == 0) {
		return -1;
	}//·��Ϊ�ļ���

	sbks.inode_bitmap[path_inode_id] = 0;
	UpdateIndBmp(path_inode_id);//����path��Ӧ��inode_bitmap

	int path_block_id = inodes[path_inode_id].i_blocks[0];
	sbks.block_bitmap[path_block_id] = 0;
	UpdateBlkBmp(path_inode_id);//����path��Ӧ��block_bitmap

	char str[4096];
	for (int i = 0; i < 4096; i++) {
		str[i] = '\0';
	}
	write_fileblock_into_file(str, path_block_id);//ɾ��path��block

	char file_name[252];
	for (int i = 0; i < 252; i++) {
		file_name[i] = '\0';
	}
	int uppath_inode_id = inodes[path_block_id].fat_id;
	int dir_id = find_position_dir_entry(path_inode_id);
	WriteDir(file_name, dir_id, uppath_inode_id, 0);//ɾ��path��Ӧ��dir_entry

	inodes[path_inode_id] = _inode(0, 0, 0, 0, 0);
	UpdateInode(path_inode_id);//ɾ��path��inode

	return 0;
}

int delete_directory(int path_inode_id)//ɾ��ĳinode_id��Ŀ¼��-2�����ڣ�-1�ļ���0�ɹ�
{
	if (path_inode_id < 0) {
		return -2;
	}
	if (inodes[path_inode_id].i_mode == 1) {
		return -1;
	}
	if (inodes[path_inode_id].i_id == 0) {
		return -3;
	}
	_dir_block dirblock = get_dirblock(path_inode_id);
	//ɾ�����ļ��к����ļ�
	for (int i = 2; i < 16; i++) {
		if (dirblock.dirs[i].name[0] != 0) {
			int down_inode_id = dirblock.dirs[i].inode_id;
			int down_mode = inodes[down_inode_id].i_mode;
			if (down_mode == 1) {
				delete_file(down_inode_id);
			}
			else {
				delete_directory(down_inode_id);
			}
		}
	}

	char str[252] = { 0 };
	for (int i = 0; i < 252; i++) {
		str[i] = 0;
	}
	int uppath_inode_id = inodes[path_inode_id].fat_id;
	int uppath_block_id = inodes[uppath_inode_id].i_blocks[0];
	int path_block_id = inodes[path_inode_id].i_blocks[0];
	int path_position = find_position_dir_entry(path_inode_id);
	if (path_position < 0) {
		printf("Unexpected mistake happened\n");
		return -1;
	}
	if (path_position >= 0) {
        /*for (int i = 0; i < 252; i++) {
			cout << (int)str[i];
		}
        cout << endl;*/
		WriteDir(str, path_position, uppath_inode_id, 0);
	}
    //cout << "path_inode_id " << path_inode_id << endl;
	WriteDir(str, 1, path_block_id, 0);
	WriteDir(str, 0, path_block_id, 0);//ɾ������Ŀ¼���"." ".." �Լ���һ���ļ���Ŀ¼���"./name"

	sbks.inode_bitmap[path_inode_id] = 0;
	UpdateIndBmp(path_inode_id);
    //cout << "path_block_id " << path_block_id << endl;
	sbks.block_bitmap[path_block_id] = 0;
	UpdateBlkBmp(path_block_id);//���superblockռ��״̬

	inodes[path_inode_id] = _inode(0, 0, 0, 0, 0);
	UpdateInode(path_inode_id);//ɾ������inode��Ϣ

	return 0;
}

int rm(char path[])//ɾ��path·�����ļ���-2�����ڣ�-1Ŀ¼��0�ɹ� 
{
	int i = delete_file(path);
	if (i == -2) {
		PathError(path);
	}
	if (i == -1) {
		FileError(path);
	}
	return i;
}

int format()//���̸�ʽ��
{
	cout << "Do you want to format the disk and erase the information?\n"
		<< "If yes, input \"Y\". If no, input other things\n"
		<< ">> ";
	string key1;
	getline(cin, key1);
	if (key1 != "Y") {
		return -1;
	}
	cout << "Do you want to format quicker but maybe not completely, or completely be slower?\n"
		<< "If quicker, input \"Q\". If completely, input \"C\"\n"
		<< "If you don't want to format, input other things\n"
		<< ">> ";
	string key2;
	getline(cin, key2);
	if (key1 == "C") {
		FormatDisk();
		return 1;
	}
	if (key1 != "Q") {
		return -1;
	}
	_dir_block dirblock = get_dirblock(0);
	//ɾ�����ļ��к����ļ�
	for (int i = 2; i < 16; i++) {
		if (dirblock.dirs[i].name[0] != 0) {
			int down_inode_id = dirblock.dirs[i].inode_id;
			int down_mode = inodes[down_inode_id].i_mode;
			if (down_mode == 1) {
				delete_file(down_inode_id);
			}
			else {
				delete_directory(down_inode_id);
			}
		}
	}
	ChangeDir("/");
	return 0;
}
int rmdir(char path[])//ɾ��path·����Ŀ¼��-3Ϊ��Ŀ¼��-2�����ڣ�-1�ļ���0�ɹ�
{
	int path_inode_id = GetPathInode(path);
	int i = delete_directory(path_inode_id);
	if (i == -3) {
		cout << "rmdir: it is dangerous to operate recursively on \"" << path << "\"\n";
	}
	if (i == -2) {
		PathError(path);
	}
	if (i == -1) {
		DirError(path);
	}
	return i;
}

void ModeError() {
	cout << "refresh: mode not found, 0 or 1 only" << endl;
}

int refresh(int i) {
	if (i == 0) {
		for (int j = 0; j < 4096; j++) {
			UpdateBlkBmp(j);
			UpdateIndBmp(j);
			UpdateInode(j);
		}
	}
	else if (i == 1) {
		InitDisk();
	}
	else {
		return 1;
	}
	return 0;
}