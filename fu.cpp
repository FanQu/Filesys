#include "fu.h"
const char filename[12] = "Filesys.vfs";
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
//ע�⣬WriteDir������д��dir_nameû�аѺ��油ȫ'\0' 
//�������ļ�����ϵ�������������һ���ļ��У��˺���ʮ��Σ��
//�˺������������⣬DirsPos(252)�ǲ��Ե� 
//��������qu.cpp 
_dir_block get_dirblock(block_id)//����dirblock 
{
	
}
int find_free_indbmp(){
	int i = 0;
	for (; (i < indbmp_size) && (skbs.inode_bitmap[i] != 0); i++);
	if (i == 4096)
		return -1;
	return i;
}
int find_free_blkbmp(){
	int i = 0;
	for (; (i < blkbmp_size) && (skbs.block_bitmap[i] != 0); i++);
	if (i == 4096)
		return -1;
	return i;
}
int find_free_dir_entry(int inode_id){ 
	int block_id = inodes[inode_id].i_blocks[0];
	//�ж��Ƿ�Ϊ�ļ��� 
	_dir_block block1 = get_dirblock(block_id);//�ҵ�block
	int i = 0;
	for (; (i<16)&&(block->dirs.name[0] != '\0'); i++);
	if (i==16)
		return -1;
	return i;
}
int echo(char path[],int inode_id, char str[]) //δ���� 
/*Ҫ��Ҫ���ǰѵ�ǰinode_id��Ϊȫ�ֱ������½������ж�pathΪ���·��/����·����������ظ�Ŀ¼/��ǰĿ¼ 
���⣬��Ȼfindpath�ȵõ��Ƿ���ڣ��ֵõ�inode������ȥ��error�������ڸ�������ʵ���� 
*/
{
	char path_up[252] = {0};
	int UpDirPos = 0;
	bool UpDir = 0;
	for (int i = 251; i >= 0; i--){
		if(path[i] == '/') {
            UpDir = 1;
            UpDirPos = i;
            break;
        }
	}
	if (UpDir){
		for (int i = UpDirPos - 1; i >= 0; i--){
			path_up[i] = path[i];
		}
		for (int i = UpDirPos; i < 252; i++){
			path_up[i] = '\0';
		}
	}
	int upstr_inode_id = FindPath(path_up[],inode_id);
	if (upstr_inode_id==-1){
		printf("%s No such directory\n",path_up[]);
		return -1;
	}//������ʵ�ظ������ˣ���ɺ�ɿ����޸�FindPath���� 
	int str_inode_id = FindPath(path[],inode_id);
	if (str_inode_id==-1){
		int x = find_free_dir_entry(y);
		if (x==-1){
			printf("%s is full\n",path_up[]);
			return -1;
		}
		//����path�ļ�����str_inode_id����Ϊ������inode_id 
	}
	inodes[str_inode_id].i_blocks[0]
}
