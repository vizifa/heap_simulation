#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAX 11
#define MAX_SIZE 1024
#define ADD_LEN 4
#define NAME_LEN 20

typedef struct node_tag{
	char* address;
	int size;
	char* name;
	int extra_bit;
	struct node_tag* next;
}Node;

typedef struct list_tag{
	Node* lptr;
}AL;

typedef struct free_list_tag{
	Node* size[MAX];
}FL;

//create block for free list with add and size
Node* create_node_fl(char* add, int size){
	Node* ptr = (Node*)malloc(sizeof(Node));
	ptr->address = add;
	ptr->extra_bit = -1;
	ptr->size = size;
	ptr->name = NULL;
	ptr->next = NULL;
	
	return ptr;
}

//initialize allocated list to null and free list to max_size
void initialize(AL* allctd_list, FL* free_list){
	allctd_list->lptr = NULL;
	for(int i=0; i<MAX; i++){
		free_list->size[i] = NULL;
	}
	
	free_list->size[MAX-1] = create_node_fl("000", MAX_SIZE);
	printf("Initialisation completed\n");
}

//print data of node
void print(Node* ptr){
	printf("\nName: %s", ptr->name);
	printf("\nSize: %d", ptr->size);
	printf("\nAddress: %s", ptr->address);
	printf("\nExtra Bit: %d\n", ptr->extra_bit);
}

/*Algorithm to allocate:
	-find nearest size(power of 2)
	-find free block of this size
	-remove free block from free list
	-add block to allocated list
*/

//creeate block for allocated list with existing node from free list and name
Node* create_node_al(Node* head, char* name){
	Node* ptr = (Node*)malloc(sizeof(Node));
	ptr->address = head->address;
	ptr->extra_bit = 1;
	ptr->name = name;
	ptr->size = head->size;
	ptr->next = NULL;
	
	free(head);
	
	return ptr;
}

//convert hex address to decimal
int convert_to_decimal(char* hex){
	int i=0, decimal = 0;
	while(hex[i] != '\0'){
        decimal *= 16;
		if(hex[i] >= '1' && hex[i] <= '9'){
            decimal += (hex[i] - 48);
        }
        else if(hex[i] >= 'A' && hex[i] <= 'F'){
            decimal += (hex[i] - 55);
        }
        i++;
	}
	return decimal;
}

//convert decimal address to hex
char* convert_to_hex(int num){
	long q, r, j=0;
    char* hex = malloc(ADD_LEN*sizeof(char));
 
    q = num;
    while (q != 0){
        r = q % 16;
        if (r < 10)
        	hex[j] = 48 + r;
            
        else
            hex[j] = 55 + r;
        q /= 16;
        j++; 
    }
    strrev(hex);
    return hex;
}

//find the address of the next block if a block is split
char* next_address(char* add, int size){
	int address = convert_to_decimal(add);
	address += size;
	char* next = convert_to_hex(address);
	return next;
}

//nearest power of 2
int find_nearest_power(int size){
	int p = ceil(log2(size));
	return p;
}

//remove block from free list
void remove_node_fl(Node* ptr, Node* prev, FL* free_list){
	int block_size = find_nearest_power(ptr->size);
	if(prev == NULL){
		free_list->size[block_size] = NULL; 
	}
	else{
		prev->next = prev->next;
	}
}

//add block to free list
void add_node_fl(Node* nptr, FL* free_list){
	int block_size = find_nearest_power(nptr->size);
	if(free_list->size[block_size] == NULL){
		free_list->size[block_size] = nptr;
	}
	else{
		Node* ptr = free_list->size[block_size];
		while(ptr->next != NULL){
			ptr = ptr->next;
		}
		ptr->next = nptr;
		nptr->next = NULL;
	}
}

//find free block according to size provided
Node* find_free_block(int size, FL* free_list){
	int block_size = find_nearest_power(size);
	Node* return_node = NULL;
	
	if(size<=MAX_SIZE){
		//if free node of size is available return that
		if(free_list->size[block_size] != NULL){
			Node* ptr = free_list->size[block_size];
			Node* prev = NULL;
			while(ptr->next!=NULL){
				prev = ptr;
				ptr = ptr->next;
			}
			return_node = ptr;
			remove_node_fl(ptr, prev, free_list);
		}
		
		//else find free block of size*2 and split
		else{
			Node* nptr = find_free_block(size*2, free_list);
			if(nptr!=NULL){
				char* add = next_address(nptr->address, size);
				nptr->size = size;
				add_node_fl(nptr, free_list);
				return_node = create_node_fl(add, size);
			}
		}
	}
	return return_node;
}

//add a block to allocated list
void add_node_al(Node* nptr, AL* allctd_list){
	Node* ptr = allctd_list->lptr;
	if(ptr == NULL){
		allctd_list->lptr = nptr;
	}
	else{
		while(ptr->next != NULL){
			ptr = ptr->next;
		}
		ptr->next = nptr;
	}
}

//check if block exists in allocated list with the name
int has_node(Node** prev, AL* allctd_list, char* name){
	int ans = 0;
	Node* ptr = allctd_list->lptr;
	if(allctd_list->lptr != NULL){
		//block to be found is the first block, ans = 2, prev = first block
		if(strcmp(allctd_list->lptr->name, name) == 0){
			ans = 2;
			*prev = allctd_list->lptr;
		}
		else{
			while(ptr->next!=NULL && strcmp((ptr->next)->name, name) == 0){
				ptr = ptr->next;
			}
			//if block exists but is not first block, ans = 1, prev stores the previos block
			if(ptr->next != NULL){
				*prev = ptr;
				ans = 1;
			}
			//else ans = 0, prev=NULL
			else{
				*prev = NULL;
			}
		}	
	}
	return ans;
}

//allocate memory
void allocate(int size, char* name, AL* allctd_list, FL* free_list){
	Node* prev;
	int exists = has_node(&prev, allctd_list, name);
	//if node of same name exists error
	if(exists){
		printf("Name exists. Please enter different name.\n");
	}
	else{
		//free block of nearest power of 2 is found
		int a = find_nearest_power(size);
		int allocated_size = pow(2, a);
		Node* free_block = find_free_block(allocated_size, free_list);
		
		//if free block is NULL, error
		if(free_block == NULL){
			printf("Block of this size is not available.\n");
		}
		//else added to allocated list 
		else{
			Node* allctd_block = create_node_al(free_block, name);
			allctd_block->size = size;
			add_node_al(allctd_block, allctd_list);
		}
	}	
}

/*steps for deallocation:
check if node exists
remove from allocated list 
add to free list 
if possible merge
*/

//remove node form allocated list 
void remove_al(Node* nptr, Node* prev, AL* allctd_list){
	if(nptr != NULL){
		if(nptr == allctd_list->lptr){
			allctd_list->lptr = nptr->next;
		}
		else{
			prev->next = nptr->next;
		}
		nptr->next = NULL;
	}
}

// add nptr to free list and merge if 2 memory next to each other exists
//nptr = newly frred node
void merge(Node* nptr, FL* free_list){
	int block_size = find_nearest_power(nptr->size);
	int size = pow(2, block_size);
	nptr->name = NULL;
	nptr->size = size;
	free(nptr->name);
	
	//if free list of size is empty
	if(free_list->size[block_size] == NULL){
		free_list->size[block_size] = nptr;
	}
	
	else{
		int flag = 0;
		Node* ptr = free_list->size[block_size];
		Node* prev = NULL;
		while(flag == 0){
			//address of nptr after node considered, ptr
			if(strcmp(next_address(ptr->address, size), nptr->address) == 0){
				remove_node_fl(ptr, prev, free_list);
				ptr->size *= 2;
				merge(ptr, free_list);
				free(nptr);
				flag = 1;
			}
			//address of ptr after address of nptr
			else if(strcmp(next_address(nptr->address, size), ptr->address) == 0){
				remove_node_fl(ptr, prev, free_list);
				ptr->size *= 2;
				ptr->address = nptr->address;
				merge(ptr, free_list);
				free(nptr);
				flag = 1;
			}
			//list ends without above conditions
			else if(ptr->next == NULL){
				ptr->next = nptr;
				nptr->next = NULL;
				flag = 1;
			}
			else{
				prev = ptr;
				ptr = ptr->next;
			}
		}
	}
}

//deallocate memory
void deallocate(char* name, AL* allctd_list, FL* free_list){
	Node* prev = NULL;
	int exists = has_node(&prev, allctd_list, name);
	//if block exists in allocated ist
	if(exists){
		Node* nptr = NULL;
		if(exists == 1){
			nptr = prev->next;	
		}
		else{
			nptr = prev; 
		}
		if(nptr->extra_bit != 0){
			nptr->extra_bit = -1;
			remove_al(nptr, prev, allctd_list);
			merge(nptr, free_list);
		}
		else{
			printf("Memory doesn\'t exist. Please enter valid name.\n");
		}
	}
	else{
		printf("Memory doesn\'t exist. Please enter valid name.\n");
	}
}

//main malloc function
void Malloc(AL* allctd_list, FL* free_list){
	printf("Enter Name: ");
	char* name = (char*)malloc(NAME_LEN*sizeof(char));
	scanf("%[^\n]%*c", name);
	printf("Enter Size: ");
	int size;
	scanf("%d%*c", &size);
	allocate(size, name, allctd_list, free_list);	
}

//main free function
void Free(AL* allctd_list, FL* free_list){
	printf("Enter Name: ");
	char* name = (char*)malloc(NAME_LEN*sizeof(char));
	scanf("%[^\n]%*c", name);
	
	deallocate(name, allctd_list, free_list);	
}

//display allocated list
void view_al(AL* allctd_list){
	Node* ptr = allctd_list->lptr;
	if(ptr == NULL){
		printf("Allocated list is empty.\n");
	}
	else{
		int flag = 0;
		while(ptr != NULL){
			if(ptr->extra_bit != 0){
				print(ptr);
				flag = 1;
			}
			ptr = ptr->next;
		}
		if(flag == 0){
			printf("Allocated list is empty.\n");
		}
	}
}

//display free list
void view_fl(FL* free_list){
	int flag = 0, i;
	for(i=0; i<MAX; i++){
		Node* ptr = free_list->size[i];
		if(ptr != NULL){
			flag = 1;
			while(ptr != NULL){
				print(ptr);
				ptr = ptr->next;
			}
		}
	}
	if(flag == 0){
		printf("Free list is empty.\n");
	}
}

//display one node
void display_node(AL* allctd_list){
	printf("Enter Name: ");
	char* name = (char*)malloc(NAME_LEN*sizeof(char));
	scanf("%[^\n]%*c", name);
	
	Node* prev = NULL;
	int exists = has_node(&prev, allctd_list, name);
	if(exists){
		Node* nptr = NULL;
		if(exists == 1){
			nptr = prev->next;	
		}
		else{
			nptr = prev; 
		}
		if(nptr->extra_bit != 0){
			print(nptr);
		}
		else{
			printf("Node does not exist. Please enter correct name.\n");
		}
	}
	else{
		printf("Node does not exist. Please enter correct name.\n");
	}
	free(name);
}

//create memory that cannot be referenced 
void create_garbage(char* name, AL* allctd_list, FL* free_list){
	Node* prev = NULL;
	int exists = has_node(&prev, allctd_list, name);
	//if block exists in allocated ist
	if(exists){
		Node* nptr = NULL;
		if(exists == 1){
			nptr = prev->next;	
		}
		else{
			nptr = prev; 
		}
		//denoting deletion without deallocation
		if(nptr->extra_bit != 0){
			nptr->extra_bit = 0;
		}
		else{
			printf("Memory doesn\'t exist. Please enter valid name.\n");
		}		
	}
	else{
		printf("Memory doesn\'t exist. Please enter valid name.\n");
	}
}

//main delete without deallocation function
void Delete(AL* allctd_list, FL* free_list){
	printf("Enter Name: ");
	char* name = (char*)malloc(NAME_LEN*sizeof(char));
	scanf("%[^\n]%*c", name);
	
	create_garbage(name, allctd_list, free_list);	
}

//garbage collector using referece counting method
void gc(AL* allctd_list, FL* free_list){
	Node* ptr = allctd_list->lptr;
	Node* prev = NULL;
	while(ptr != NULL){
		if(ptr->extra_bit == 0){
			ptr->extra_bit = -1;
			remove_al(ptr, prev, allctd_list);
			merge(ptr, free_list);
			
			prev = ptr;
			ptr = ptr->next;
		}
	}
	printf("Gabage collected.\n");
}

int main(){
	AL allctd_list;
	FL free_list;
	initialize(&allctd_list, &free_list);
	
	char menu; 
	int end = 0;
	
	//menu driven program
	while(end == 0){
		printf("\nEnter:\n1 to allocate memory\n2 to deallocate memory\n3 to view free list\n4 to view allocated list\n5 to view node\n6 to delete memory without deallocating\n7 to run garbage collector\nAny other key to exit.\n");
		scanf("%c%*c", &menu);
		switch(menu){
			case '1': Malloc(&allctd_list, &free_list);
					break;
			case '2': Free(&allctd_list, &free_list);
					break;		
			case '3': view_fl(&free_list);
					break;
			case '4': view_al(&allctd_list);
					break;
			case '5': display_node(&allctd_list);
					break;
			case '6': Delete(&allctd_list, &free_list);
					break;
			case '7': gc(&allctd_list, &free_list);
					break;
			default: end = 1;
		}
		
	}
	
	return 0;
}
