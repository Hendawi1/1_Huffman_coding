#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>



typedef struct tree_el{
	char value;
	int freq;
	struct tree_el *right;
	struct tree_el *left;
} tree_el;



typedef struct Node{
	tree_el *value;
	struct Node *next;
} Node;


bool add_to_freq_sorted_arr(Node **head, tree_el *new_el){
	if (!new_el) return false;

	Node* temp = *head; // Use a temporary pointer to iterate without changing the head
	Node* x = (Node*)malloc(sizeof(Node));
	x->value = new_el;
	x->next = NULL;

	if (temp == NULL){
		*head = x;
		return true;
	}
	else if ((*head)->value->freq >= new_el->freq){
		x->next = *head;
		*head = x;
		temp = *head;
	}
	else{ 
		while (temp->next && temp->next->value->freq < new_el->freq)
			temp = temp->next;
		x->next = temp->next;
		temp->next = x;
		return true;
	}
}

//vonvert every symbol to "tree element" and add it to freq-sorted array (priority queue):
Node *symbols_to_tree_el_conv(int freq[]){
	Node *head = NULL;
	
	for (int i=0; i < 256; i++)
		if (freq[i]){
			tree_el* new_el = (tree_el*)malloc(sizeof(tree_el));
			new_el -> value = i;
			new_el -> freq = freq[i];
			new_el -> right = new_el -> left = NULL;
			// add it to freq-sorted array
			add_to_freq_sorted_arr(&head, new_el);
		}
	return head;
}



void freq_calc(int freq[], FILE *input){
       	char c;
       	for (; (c=fgetc(input)) != EOF;) freq[c]++;
       	freq['\n'] = 0;
}


void freq_print(int freq[]){
        for (int i=0; i < 256; i++) if (freq[i]) printf("\"%c\"\t:\t%d\n", i, freq[i]);
}


void print_list(Node* head){
	while(head){
		printf("\"%c\" (freq = %d)\n", (head) -> value -> value,  (head) -> value -> freq);
		head = head -> next;
	}
	printf("\n\n");
	//return true;
}


tree_el *build_huffman_tree(Node **head){

	while ((*head)->next){
		//merge 1-st and 2-nd elements:
		tree_el* new_el = (tree_el*)malloc(sizeof(tree_el));
		new_el -> freq = (*head)->value->freq + (*head)->next->value->freq;
		new_el -> left = (*head)->value;
		new_el -> right = (*head)->next->value;
		
		
		// remove them from sorted-freq list:
		*head = (*head)->next->next;
		
		
		// add merged element to freq-sorted array
		add_to_freq_sorted_arr(head, new_el);
	}
	return (*head)->value;
}


void print_tree(tree_el* root, char code[], char **huff, int index) {
	if (root == NULL) return;
	
	// If the current node is a leaf node
	if (root->left == NULL && root->right == NULL) {
		code[index] = '\0'; // Null-terminate the code
		huff[root->value] = (char*)malloc(strlen(code) + 1);
		strcpy(huff[root->value], code);
		printf("\"%c\": %s\n", root->value, huff[root->value]);
	}
	else {
		// Move to the left child and add '0' to the code
		code[index] = '0';
		print_tree(root->left, code, huff, index + 1);

		// Move to the right child and add '1' to the code
		code[index] = '1';
		print_tree(root->right, code, huff, index + 1);
	}
}




// حساب طول البايتات من البتتات من اجل تخصيص ذاكرة للمتجه المنطقي
size_t calc_bytes_from_bits(size_t bits){
	return ((bits -1) / 8) + 1 ;
}



unsigned char* string_to_bool_conv(char* str, size_t bits){
	if(str && bits > 0){        
		size_t bytes = calc_bytes_from_bits(bits);
		unsigned char* bv = (unsigned char*)calloc(bytes, sizeof(unsigned char));
		if(!bv) return NULL;

		unsigned char mask = 1;
		for(; bytes > 0; bytes--)
			for(int k = 0; k < 8 && bits > 0; k++){
				if(str[bits-1] != '0') bv[bytes-1] = bv[bytes-1] | mask << k;
				bits--;
			}
		return bv;
	}
	return NULL;
}


int encode(char **huff, FILE *input, FILE *encoded){
	rewind(input);
       	char c;
       	int len = 0;
       	//char encoded_as_string[];
       	char* encoded_as_string = (char *)calloc((len+1), sizeof(char));
       	for (; (c=fgetc(input)) != EOF;)
       		if (huff[c] != NULL){
       			strcat(encoded_as_string, huff[c]);
       			len+=strlen(huff[c]);
       			encoded_as_string = (char*)realloc(encoded_as_string, sizeof(char) * (len + 1));
		}
	printf("%s\n", encoded_as_string);
	//printf("\nlen = %d\n", len);
	unsigned char* encoded_as_bin = string_to_bool_conv(encoded_as_string, len);
	fwrite(encoded_as_bin,sizeof(encoded_as_string),1,encoded);
	return len;
}


char* convert_bv_to_string(unsigned char* bv, int bits) {
	if(bv && bits > 0){
		size_t bytes = calc_bytes_from_bits(bits);
		char* str = (char*)calloc(bits + 1, sizeof(char));
		if(!str) return NULL;

		int idx = bits /*- 1*/;
		unsigned char mask = 1;
		for(int i = bytes - 1; i >= 0; i--)
			for(int j = 0; j < 8 && idx /*>= 0*/; j++){
				if(bv[i] & mask << j) str[idx-1] = '1';
				else str[idx-1] = '0';
				idx--;
			}
		str[bits] = '\0';
		return str;
	}
	return NULL;
}


void decode(tree_el* tree, FILE *encoded, FILE *decoded, int len){
	rewind(encoded);
	size_t bytes = calc_bytes_from_bits(len);
	unsigned char *encoded_as_bin = (unsigned char*)calloc(bytes, sizeof(char));
	fread(encoded_as_bin,bytes,1,encoded);
	char *encoded_as_string = convert_bv_to_string(encoded_as_bin, len);	
	tree_el* root = tree;
	for (int i=0; encoded_as_string[i]; i++)
		if (encoded_as_string[i]=='0')
			if (root->left->left == NULL && root->left->right == NULL){
				printf("%c", root->left->value);
				fprintf(decoded, "%c", root->left->value);
				root = tree;
			}
			else root = root->left;
		else if (encoded_as_string[i]=='1')
			if (root->right->left == NULL && root->right->right == NULL){
				printf("%c", root->right->value);
				fprintf(decoded, "%c", root->right->value);
				root = tree;
			}
			else root = root->right;
}


int main(){

	FILE *input = fopen("input.txt", "r");
	if (input == NULL) {printf("\nError openning input file!\n"); return 1;}
	int freq[256] = { 0 };
	char code[256];
	char *huff[256];

	printf("[freq]:\n");
	freq_calc(freq, input);
	freq_print(freq);
	
	printf("\n\n[freq-sorted list]:\n");
	Node *freq_sorted_arr = symbols_to_tree_el_conv(freq);
	print_list(freq_sorted_arr);
	
	printf("\n[huff code]:\n");
	tree_el *tree = build_huffman_tree(&freq_sorted_arr);
	print_tree(tree, code, huff, 0);
	
	FILE *encoded = fopen("encoded.bin", "wb");
	if (encoded == NULL) {printf("\nError openning encoded file!\n"); return 1;}
	printf("\n\n[encoded]:\n");
	int len = encode(huff, input, encoded);
	fclose(encoded);
	
	printf("\n\n[decoded]:\n");
	encoded = fopen("encoded.bin", "rb");
	FILE *decoded = fopen("decoded.txt", "w");
	if (decoded == NULL) {printf("\nError openning decoded file!\n"); return 1;}
	decode(tree, encoded, decoded, len);
	
}
