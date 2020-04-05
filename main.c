#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

// 唯一的全局变量，统计循环转账数目
int CNT = 0;

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
    unsigned int height;
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


inline struct node* NewNode(int id){
	struct node* temp = (struct node*)malloc(sizeof(struct node));
	temp->id = id;
	temp->left = NULL;
	temp->right = NULL;
	temp->height = 1;
	temp->next = NULL;
	return temp;
}

// 内联函数，提升速度
inline int max(int a,int b){
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

	z->height = max(height(z->left),height(z->right))+1;
	y->height = max(height(y->left),height(y->right))+1;

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

void FindLoop(struct node* root, struct loop *list, struct loop find){
	// 环超过既定长度, 环返回到前边搜索过的节点， 退出
	if(find.height > 7 || root->id < find.nodes[0]){
		return;
	}
		
	if(root->id == find.nodes[0] && find.height > 0){
		// 环太小，退出
		if(find.height >= 3){
			// 拷贝一份环
			struct loop *copy = malloc(sizeof(struct loop));
			memcpy(copy, &find, sizeof(find));

			struct loop *pp = &(list[find.height]);
			while(pp->next != NULL){
				pp = pp->next;
			}
			pp->next = copy;
			CNT++;
			//debug
			// for(int i=0; i<copy->height; i++)
			// 	printf("%d", copy->nodes[i]);
			// printf("\n");
		}
		return;
	}
	
	// 避免子环，可以优化查找的位置
	for(int i=0; i<find.height; i++){
		if(find.nodes[i]==root->id)
			return;
	}
	// 加入环
	find.nodes[find.height] = root->id;
	find.height++;
	struct path *pp = root->next;
	while(pp != NULL){
		FindLoop(pp->curr, list, find);
		pp = pp->next;
	}
}

// 中序遍历
void inorder(struct node* root, struct loop *list){
	if(root==NULL)
		return;

    inorder(root->left, list);
	
	// 查找当前节点的环路
	struct loop find = {{0,0,0,0,0,0,0}, 0, NULL};
	FindLoop(root, list, find);
	// printf("%d---",root->id);
	// struct path * point = root->next;
	// while(point != NULL){
	// 	printf("%d ", point->curr->id);
	// 	point = point->next;
	// }
	// printf("\n");

	inorder(root->right, list);
}


struct node* Insert(struct node* root, int id){
	if(root==NULL)
		return NewNode(id);

	if(id < root->id)
		root->left = Insert(root->left, id);

	else if(id > root->id)
		root->right = Insert(root->right, id);

	else
		return root;

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


inline struct node* Exist(struct node* root, int id){
	if (root == NULL){
		return NULL;
	}
	
	if (root->id == id){
		return root;
	}
	else if (id < root->id)	{
		return Exist(root->left, id);
	} 
	else {
		return Exist(root->right, id);
	}
}


struct node* Build(char * filename){
    int a, b, c;
	FILE *fp = NULL;
	struct node* root = NULL;
    
    fp = fopen(filename, "r");
    while(fscanf(fp, "%d,%d,%d", &a, &b, &c) != EOF){
		struct node* A = Exist(root, a);
    	if(A==NULL){
			root = Insert(root, a);
			A = Exist(root, a);
		}

		struct node* B = Exist(root, b);
		if(B==NULL){
			root = Insert(root, b);
			B = Exist(root, b);
		}

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
				struct path *tmp = point->next;
				point->next = curr;
				curr->next = tmp;
			}
		}		
	}
    fclose(fp);
	return root;
}


void Save(char * filename, struct loop* list){
    int cnt = 0;
	FILE *fp = NULL;
    fp = fopen(filename, "w");

	fprintf(fp, "%d\n", CNT);
	
	for(int i=3; i<8; i++){
		struct loop *pp = list[i].next;
		while(pp != NULL){
			fprintf(fp, "%d", pp->nodes[0]);
			for(int j=1; j<pp->height; j++)
				fprintf(fp, ",%d", pp->nodes[j]);
			fprintf(fp, "\n");
			pp = pp->next;
			cnt++;
		}
	}
	// fseek(fp, 0L, SEEK_SET);
	// fseek(fp,0, SEEK_SET);
	// fprintf(fp, "%d\n", cnt);

    fclose(fp);  
}


int main(void){
	clock_t start, finish;
	double Total_time;
	start = clock();

	char readfile[] = "test_data.txt";
	char writefile[] = "demo.txt";

	struct loop list[8] = {
		{{0,0,0,0,0,0,0}, 0, NULL},
		{{0,0,0,0,0,0,0}, 0, NULL},
		{{0,0,0,0,0,0,0}, 0, NULL},
		{{0,0,0,0,0,0,0}, 0, NULL},
		{{0,0,0,0,0,0,0}, 0, NULL},
		{{0,0,0,0,0,0,0}, 0, NULL},
		{{0,0,0,0,0,0,0}, 0, NULL},
		{{0,0,0,0,0,0,0}, 0, NULL}
	};	// 0~2没有用到 

	struct node* root = Build(readfile);

	// 网络建立时间
	finish = clock();
	Total_time = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("\nBuild Time: %lf seconds\n", Total_time);

	inorder(root, list);
	Save(writefile, list);

	// 总时间
	finish = clock();
	Total_time = (double)(finish - start) / CLOCKS_PER_SEC;
	printf("\nTotal Time: %lf seconds\n", Total_time);
	return 0;
}
