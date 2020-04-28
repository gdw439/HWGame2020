#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define DeBug 1

#ifdef DeBug
	int PAD = 0;	// 统计节点数目
	int num = 0;
#endif

int CNT = 0;	// 统计循环转账数目

struct node;
struct path;

// 有向图的边
struct path{
    struct node *curr;
    struct path *next;
};

// 平衡二叉树的节点
struct node{
    unsigned int id;
    unsigned short height;
	unsigned short isfork;
    struct node *left;
    struct node *right;
    struct path *next;
};

// 保存找到的环
struct loop{
	int nodes[7];
	int height;
	struct loop* next;
};
// 保存环的链表
struct list{
	struct loop* head;
	struct loop* tail;
};


inline struct node* NewNode(int id){
	struct node* temp = (struct node*)malloc(sizeof(struct node));
	if(temp==NULL){
		printf("No Vaild Memory\n");
		return NULL;
	}

#ifdef DeBug
	PAD++;
#endif

	temp->id = id;
	temp->left = NULL;
	temp->right = NULL;
	temp->height = 1;
	temp->next = NULL;
	temp->isfork = 0;
	return temp;
}

inline struct loop* NewLoop(struct loop *root){
	struct loop *copy = malloc(sizeof(struct loop));
	if(copy==NULL){
		printf("No Vaild Memory\n");
		return NULL;
	}
	memcpy(copy, root, sizeof(struct loop));
	return copy;
}

inline void initlists(struct list *lists){
	for(int i=0; i<8; i++){
		lists[i].head = NULL;
		lists[i].tail = NULL;
	}
}

// 内联函数，提升速度
inline int max(int a, int b){
 	return (a >b ) ? a : b;
}

// 内联函数，提升速度
inline int height(struct node* root){
	if(root==NULL)
		return 0;
 	return root->height;
}

inline int Balance(struct node* root){
	if(root==NULL)
		return 0;
 	return height(root->left) - height(root->right);
}


inline struct node* LeftRotate(struct node* z){
	struct node* y = z->right;
	struct node* t2 = y->left;

	y->left = z;
	z->right = t2;

	z->height = max(height(z->left), height(z->right)) + 1;
	y->height = max(height(y->left), height(y->right)) + 1;

	return y;
}


inline struct node * RightRotate(struct node* z){
	struct node* y = z->left;
	struct node* t3 = y->right;

	y->right = z;
	z->left = t3;

	z->height = max(height(z->left),height(z->right))+1;
	y->height = max(height(y->left),height(y->right))+1;

	return y;
}

void FindLoop(struct node* root, struct list *lists, struct loop *find){
	// 环超过既定长度, 环返回到前边搜索过的节点， 退出
	if(find->height > 7 || root->id < find->nodes[0]){
		return;
	}
	if(root->isfork){
		if(root->id != find->nodes[0])
			return;
		else {
			// 环太小，退出
			if(find->height >= 3){
				// 拷贝一份环
				struct loop *copy = NewLoop(find);
				if(lists[find->height].tail==NULL){
					// 环列表节点为空，初始化
					lists[find->height].tail = copy;
					lists[find->height].head = copy;
				} else {
					// 环列表非空，更新
					lists[find->height].head->next = copy;
					lists[find->height].head = copy;
				}
				if(CNT%1000==0)
					printf("current loop: %d\n", CNT);
				CNT++;
			}
			return;
		}
	} else {
		// 访问占用
		root->isfork = 1;
		// 加入环
		find->nodes[find->height] = root->id;
		find->height++;
		struct path *pp = root->next;
		while(pp != NULL){
			FindLoop(pp->curr, lists, find);
			pp = pp->next;
		}
		// 取消访问占用
		root->isfork = 0;
		find->height--;
	}
}

// 中序遍历
void inorder(struct node* root, struct list *lists){
	if(root==NULL)
		return;

    inorder(root->left, lists);
	printf("%d\n", num++);
	// 查找当前节点的环路
	struct loop *find =NULL, tmp = {{0,0,0,0,0,0,0}, 0, NULL};
	find = &tmp;
	FindLoop(root, lists, find);

	inorder(root->right, lists);
}


struct node* Insert(struct node* root, int id, struct node** seek){
	if(root==NULL){
		*seek = NewNode(id);
		return *seek;
	}

	if(id < root->id){
		root->left = Insert(root->left, id, seek);

	} else if(id > root->id){
		root->right = Insert(root->right, id, seek);

	} else {
		*seek = root;
		return root;
	}

	root->height = max(height(root->left), height(root->right))+1;

	int balance = Balance(root);

	// Left Left Case
	if(balance > 1 && id < root->left->id)
		return RightRotate(root);

	// Right Right Case
	if(balance < -1 && id > root->right->id)
		return LeftRotate(root);

	//Left Right Case
	if(balance > 1 && id > root->left->id){
		root->left = LeftRotate(root->left);
		return RightRotate(root);
	}

	// Right Left Case
	if(balance < -1 && id < root->right->id){
		root->right = RightRotate(root->right);
		return LeftRotate(root);
	}

	return root;
}


struct node* Build(char * filename){
    int a, b, c;
	FILE *fp = NULL;
	struct node* root = NULL;
    
    fp = fopen(filename, "r");
    while(fscanf(fp, "%d,%d,%d", &a, &b, &c) != EOF){
		struct node *A = NULL, *B = NULL;
		root = Insert(root, a, &A);
		root = Insert(root, b, &B);

		struct path* curr = (struct path*)malloc(sizeof(struct path));
		curr->curr = B;
		curr->next = NULL;
		// 每个节点的边按照指向节点的大小插入链表
		if(A->next==NULL){
			A->next = curr;
		} else {
			struct path* point = A->next;
			if(point->curr->id > B->id){
				A->next = curr;
				curr->next = point;
			}else{
				while(point->next != NULL && point->next->curr->id < B->id)
					point = point->next;
				curr->next = point->next;
				point->next = curr;
			}
		}		
	}
    fclose(fp);
	return root;
}


void Save(char * filename, struct list* lists){
    int cnt = 0;
	FILE *fp = NULL;
    fp = fopen(filename, "w");

	fprintf(fp, "%d\n", CNT);
	
	for(int i=3; i<8; i++){
		struct loop *pp = lists[i].tail;
		while(pp != NULL){
			fprintf(fp, "%d", pp->nodes[0]);
			for(int j=1; j<pp->height; j++)
				fprintf(fp, ",%d", pp->nodes[j]);
			fprintf(fp, "\n");
			pp = pp->next;
			cnt++;
		}
	}
    fclose(fp);  
}


int main(void){
	clock_t start, finish;
	double Total_time;
	start = clock();

	char readfile[] = "/data/test_data.txt";
	// char readfile[] = "test.txt";
	char writefile[] = "/projects/student/result.txt";

	// 数组指针
	struct list lists[8]; 

	struct node* root = Build(readfile);

#ifdef DeBug
	printf("Node num : %d\n", PAD);
	printf("AVL height : %d\n", root->height);
#endif

	// 网络建立时间
	// finish = clock();
	// Total_time = (double)(finish - start) / CLOCKS_PER_SEC;
	// printf("\nBuild Time: %lf seconds\n", Total_time);
	
	initlists(lists);
	inorder(root, lists);
	Save(writefile, lists);

	// 总时间
	finish = clock();
	Total_time = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("\nTotal Time: %lf seconds\n", Total_time);
	return 0;
}
