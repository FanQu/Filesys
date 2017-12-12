#include "fu.h"
#include "qu.cpp"
/*const char filename[12] = "Filesys.vfs";
const int inode_size = 32, datablk_size = 4096, dir_size = 256;
const int indbmp_size = 4096, blkbmp_size = 4096, inodes_size = inode_size * 4096;
long DataBlkPos(int x); //���ص�x��data_block�ڴ����ļ��е�λ��
long InodesPos(int x); //���ص�x��inode�ڴ����ļ��е�λ��
long DirsPos(int x); //���ص�x���ļ�Ŀ¼�����λ��
int UpdateInode(int x);//���µ�x��inode��Ϣ 
int UpdateBlkBmp(int x);//���µ�x��������block��Ϣ 
int FindSonPath(char sonpath[],int inode_id); 
int FindPath(char path[], int inode_id);//����path[]��inode_id·����-1 
void WriteDir(const char *dir_name, int dir_id, int inode_id);//��inode_id�ļ����еĵ�dir_id��λ�ý����ļ�����ϵdir_name
int UpdateIndBmp(int x);
int UpdateBlkBmp(int x);
int UpdateInode(int x);
int GetPathInode(char path[], int type_judge = 0); */
//ע�⣬WriteDir������д��dir_nameû�аѺ��油ȫ'\0' 
//��������qu.cpp 

//ǰ������$str������4095���ַ���������'\0'����Խλ��

_dir_block get_dirblock(int inode_id)//����dirblock�����棬ÿ��ʹ��dirblock֮ǰ����Ҫ�ж��Ƿ�Ϊ�ļ��� 
{
	FILE *vfs = fopen(filename, "rb+");
	_dir_block block;
	int block_id = inodes[inode_id].i_blocks[0];
	int Position = DataBlkPos(block_id);
	for (int i = 0; i < 16; i++){
		fseek(vfs, Position + i * dir_size, SEEK_SET);
		fread(block.dirs[i].name, 252, 1, vfs);
		fread(&block.dirs[i].inode_id,sizeof(block.dirs[i].inode_id), 1, vfs);
	}
	fclose(vfs);
	return block;
}
_file_block get_fileblock(int inode_id)//����fileblock�����棬ÿ��ʹ��fileblock֮ǰ����Ҫ�ж��Ƿ�Ϊ�ļ� 
{
	_file_block fileblock;
	FILE *vfs = fopen(filename, "rb+");
	int block_id = inodes[inode_id].i_blocks[0];
	int Position = DataBlkPos(block_id);
	fseek(vfs, Position, SEEK_SET);
	fread(fileblock.data ,sizeof(fileblock.data), 1, vfs);
	fclose(vfs);
	return fileblock;
}
void write_fileblock_into_file(char str[4096],int block_id)//��block_id����дstr�����棬ÿ��ʹ��ǰ�豣֤���ļ� 
{
	FILE *vfs = fopen(filename, "rb+");
	int Position = DataBlkPos(block_id);
	fseek(vfs, Position, SEEK_SET);
	fwrite(str ,sizeof(str), 1, vfs);
	fwrite('\0' ,sizeof(char), datablk_size-sizeof(str), vfs);
	fclose(vfs);
	return;
}
int find_free_indbmp(){
	int i = 0;
	for (; (i < indbmp_size) && (sbks.inode_bitmap[i] != 0); i++);
	if (i == 4096){
		printf("All inodes are used\n");
		return -1;
	}
	return i;
}
int find_free_blkbmp(){
	int i = 0;
	for (; (i < blkbmp_size) && (sbks.block_bitmap[i] != 0); i++);
	if (i == 4096){
		printf("All blocks are used\n");
		return -1;
	}
	return i;
}
int find_free_dir_entry(int inode_id, char path[]){
	if (inodes[inode_id].i_mode == 1){
		printf("%s is not a directory\n",path);
	}
	int block_id = inodes[inode_id].i_blocks[0]; 
	_dir_block block1 = get_dirblock(inode_id);//�ҵ�block
	int i = 0;
	for (; (i<16)&&(block1.dirs[i].name[0] != '\0'); i++);
	if (i==16){
		printf("%s is full\n",path);
		return -1;
	}
	return i;
}
int find_position_dir_entry(int path_inode_id){//δ��path�жϣ����������ж�path������uppathΪ�ļ���
	int uppath_inode_id = inodes[path_inode_id].fat_id;
	_dir_block block1 = get_dirblock(uppath_inode_id);//�ҵ�block
	int i = 0;
	for (; (i < 16) && (block1.dirs[i].inode_id == path_inode_id); i++);
	if (i==16){
		return -1;
	}
	return i;
}

int echo(char path[], char str[])//��str��д��path·�����ļ� 
{
	char path_up[252] = {0};//��һ��Ŀ¼
	char str_name[252] = {0};//�ļ���
	int UpDirPos = 0;
	bool UpDir = 0;
	for (int i = 251; i >= 0; i--){
		if(path[i] == '/') {
			UpDir = 1;
			UpDirPos = i;
			break;
		}
	}
	if (UpDir){//����ļ������ϼ��ļ���·��
		for (int i = UpDirPos - 1; i >= 0; i--){
			path_up[i] = path[i];
			str_name[251-i] = '\0';
		}
		for (int i = UpDirPos; i < 252; i++){
			path_up[i] = '\0';
			str_name[i-UpDirPos] = path[i];
		}
	}

	else {//�������
		printf("%s No such file or directory\n", path_up);
		return -1;
	}
	int upstr_inode_id = GetPathInode(path_up);
	if (upstr_inode_id<0){
		printf("%s No such file or directory\n", path_up);
		return -1;
	}
	if (inodes[upstr_inode_id].i_mode == 0){
		printf("%s is not directory\n",path_up);
		return -1;
	}

	int str_inode_id = FindPath(path, upstr_inode_id);//�������ڣ��򴴽�������Ϣ
	if (str_inode_id<0){
		int str_position = find_free_dir_entry(upstr_inode_id, path_up);
		if (str_position < 0){
			return -1;
		} 
		str_inode_id = find_free_indbmp();
		if (str_inode_id<0){
			return -1;
		}
		int str_block_id = find_free_blkbmp();
		if (str_block_id<0){
			return -1;
		}//���϶�Ӧ���̿ռ����������

		char path_name[252] = {0};
		WriteDir(str_name, str_position, upstr_inode_id, str_inode_id);
		inodes[str_inode_id] = _inode(str_inode_id, 1, sizeof(str), upstr_inode_id, str_block_id);
		//�������inode/block����Ϊ���Ǳ������ǿյ� 
		sbks.inode_bitmap[str_inode_id] = 1;
		sbks.block_bitmap[str_block_id] = 1;
		UpdateIndBmp(str_inode_id);
		UpdateBlkBmp(str_block_id);
		UpdateInode(str_inode_id);//����������Ϣ
	}

	char full_str[4096];
	int i = 0;
	for (;str[i] != '\0';i++){
		full_str[i] = str[i];
	}
	for (;i<4096;i++){
		full_str[i] = '\0';
	}//��չstr����׼����

	write_fileblock_into_file(full_str,inodes[str_inode_id].i_blocks[0]);//д��str
	return 0;
}
 
int cat(char path[])//��ȡpath·�����ļ�
{
	int str_inode_id = GetPathInode(path); 
	if (str_inode_id<0){
		printf("%s No such file or directory\n",path);
		return -1;
	}//·��������
	if (inodes[str_inode_id].i_mode == 0) {
		printf("%s is not a file\n", path);
		return -1;
	}//·��Ϊ�ļ���
	printf("%s\n",get_fileblock(str_inode_id).data);
	return 0;
}
int delete_file(int path_inode_id)//ɾ��ĳinode_id���ļ����Ѽ��ж�path�Ƿ�Ϊ�ļ��У��������ж�path������uppathΪ�ļ���
{
	if (inodes[path_inode_id].i_mode == 0) {
		return -1;
	}//·��Ϊ�ļ���

	sbks.inode_bitmap[path_inode_id] = 0;
	UpdateIndBmp(path_inode_id);//����path��Ӧ��inode_bitmap

	int path_block_id = inodes[path_inode_id].i_blocks[0];
	sbks.inode_bitmap[path_block_id] = 0;
	UpdateBlkBmp(path_inode_id);//����path��Ӧ��block_bitmap

	char str[252] = { 0 };
	for (int i = 0; i < 252; i++) {
		str[i] = '\0';
	}
	write_fileblock_into_file(str, path_block_id);//ɾ��path��block

	int uppath_inode_id = inodes[path_block_id].fat_id;
	int dir_id = find_position_dir_entry(path_inode_id);
	WriteDir(str, dir_id, 0, uppath_inode_id);//ɾ��path��Ӧ��dir_entry

	inodes[path_inode_id] = _inode(0, 0, 0, 0, 0);
	UpdateInode(path_inode_id);//ɾ��path��inode
	return 0;
}

int delete_directory(int path_inode_id)//ɾ��ĳinode_id���ļ���path_inode_id<0�������ڣ�����-2�������ļ��������ļ��У�����-1
{
	if (path_inode_id < 0) {
		return -2;
	}
	if (inodes[path_inode_id].i_mode == 1) {
		return -1;
	}

	_dir_block dirblock = get_dirblock(path_inode_id);
	//ɾ�����ļ��к����ļ�
	for (int i = 3; i < 16; i++) {
		
		char* down_name = dirblock.dirs[i].name;
		bool NotEmpty = false;
		for (int j = 0; j < 252; j++) {
			if (down_name[j] != 0) {
				NotEmpty = true;
				break;
			}
		}
		if (NotEmpty) {
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
		str[i] = '\0';
	}//current here 12/12 11:38
	int uppath_inode_id = inodes[path_inode_id].fat_id;
	int path_position = find_position_dir_entry(path_inode_id);
	if (path_position < 0) {//����������������ܳ��֣�debug��Ϻ�Ҫɾȥ
		printf("Unexpected mistake happened");
		return -1;
	}
	if (path_position >= 0) {
		WriteDir(str, path_position, 0, uppath_inode_id);
	}
	WriteDir(str, 1, 0, path_inode_id);
	WriteDir(str, 0, 0, path_inode_id);//ɾ������Ŀ¼���"." ".." �Լ���һ���ļ���Ŀ¼���"./name"

	sbks.inode_bitmap[path_inode_id] = 0;
	UpdateIndBmp(path_inode_id);
	int path_block_id = inodes[path_inode_id].i_blocks[0];
	sbks.inode_bitmap[path_block_id] = 0;
	UpdateIndBmp(path_block_id);//���superblockռ��״̬

	inodes[path_inode_id] = _inode(0, 0, 0, 0, 0);
	UpdateInode(path_inode_id);//ɾ������inode��Ϣ

	return 0;
}



int rm(char path[])//ɾ��path·�����ļ� 
{
	int path_inode_id = GetPathInode(path);
	if (path_inode_id < 0) {
		printf("%s No such file or directory\n", path);
		return -1;
	}
	int i = delete_file(path_inode_id);//������Ȼpath���ڣ�uppath���������ļ���
	if (i < 0) {
		printf("%s is not a file\n", path);
	}
	return i;
}

int rmdir(char path[])//ɾ��path·�����ļ���
{
	int path_inode_id = GetPathInode(path);
	int i = delete_directory(path_inode_id);
	if (i < 0) {
		if (i == -2)
			printf("%s No such file or directory\n", path);
		if (i == -1)
			printf("%s is not a directory\n", path);
		return -1;
	}
	return i;
}
