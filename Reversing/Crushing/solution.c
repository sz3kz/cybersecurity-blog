#include <stdio.h>
#include <stdlib.h>

#define BYTES_SIZE 8 
#define FILENAME "message.txt.cz"

struct Node {
	struct Node * next_ptr;
	char charcode;
	long position;
};
typedef struct Node Node;


struct Entry {
	struct Entry * next_entry_ptr;
	char character_code;
	long message_index;
};

typedef struct Entry Entry;

void add_node(Node ** head_ptr, char charcode, long position);
int print_all_nodes(Node * head);
long get_max_position(Node * head);
Node * get_ptr_of_position(Node * head, long position);

Entry * get_last_entry_ptr(Entry * first_entry_ptr);
void reconstruct_message(Entry * first_entry_ptr);

int main(){
	FILE * fileptr = fopen(FILENAME,"r");
	if (fileptr == NULL){
		printf("Failed to open file: \"%s\".\n", FILENAME);
		return 1;
	}

	long max_position;
	Node * head = NULL;
	/*
	int nodes_printed;
	Node * position_two;
	nodes_printed = print_all_nodes(head);
	printf("%d nodes printed.\n",nodes_printed);

	add_node(&head, 10, 0);
	nodes_printed = print_all_nodes(head);
	printf("%d nodes printed.\n",nodes_printed);

	add_node(&head, 9, 2);
	nodes_printed = print_all_nodes(head);
	printf("%d nodes printed.\n",nodes_printed);

	add_node(&head, 8, 3);
	nodes_printed = print_all_nodes(head);
	printf("%d nodes printed.\n",nodes_printed);

	max_position = get_max_position(head);
	printf("Max position: %li.\n",max_position);

	position_two = get_ptr_of_position(head, max_position);
	printf("Node of position = %d: (%2x,%3d)\n",max_position,position_two->charcode,position_two->position);
	*/

	int character_code = 0;
	long list_length; 
	long position;
	int charcode = 0;
	while (charcode < 256){
		fread(&list_length, sizeof(long), 1, fileptr);
		while(list_length != 0){
			--list_length;
			fread(&position,sizeof(long), 1, fileptr);
			add_node( & head, charcode, position);
		}
		++charcode;
	}
	// print_all_nodes(head);


	max_position = get_max_position(head);
	Node * pointer;
	char * message = (char *) malloc(sizeof(char) * (max_position + 2));
	for (position = 0; position <= max_position; ++position){
		pointer = get_ptr_of_position( head, position);
		if (pointer == NULL){
			printf("Something went wrong with position = %li\n.",position);
			return -1;
		}
		message[position] = pointer->charcode;
	}
	message[max_position+1] = 0;
	printf("Message:\n\n");
	printf("%s\n",message);
	printf("End\n");
	/*
	long message_index = 0;
	Entry * pointer;
	while (1){
		pointer = get_ptr_to_message_index( &first_entry, message_index);
		if (pointer == NULL){
			break;
		}
		printf("%c",pointer->character_code);
		++message_index;
	}
	*/


	return 0;
}

Node * get_ptr_of_position(Node * head, long position){
	Node * current_node = head;
	if (current_node == NULL){
		return NULL;
	}
	else {
		if (current_node->position == position) return current_node;
	}
	while (current_node->next_ptr != NULL){
		current_node = current_node->next_ptr;
		if (current_node->position == position) return current_node;
	}
	return NULL;
}

long get_max_position(Node * head){
	long max_position = -1;
	Node * current_node = head;

	if (current_node == NULL){
		return max_position;
	}
	else{
		max_position = current_node->position;
	}
	while (current_node->next_ptr != NULL){
		current_node = current_node->next_ptr;
		if (current_node->position > max_position){
			max_position = current_node->position;
		}
	}
	return max_position;
}

void add_node(Node ** head_ptr, char charcode, long position){
	// Adds a new node to the list
	// Optimization -> new node with be the first node in the list after addition
	Node * new_node = (Node *) malloc( sizeof(Node) );
	new_node->charcode = charcode;
	new_node->position = position;
	new_node->next_ptr = *(head_ptr);
	*(head_ptr) = new_node;
}

int print_all_nodes(Node * head){
	Node * current_node = head;
	int node_count = 0;
	if (current_node == NULL){
		return node_count;
	}
	else{
		printf("(0x%02x,%3li)",current_node->charcode,current_node->position);
		++node_count;
	}
	while (current_node->next_ptr != NULL){
		current_node = current_node->next_ptr;
		++node_count;
		printf(" -> (0x%02x,%3li)",current_node->charcode,current_node->position);
	}
	printf("\n");
	return node_count;
}

void print_list(Entry * first_entry_ptr){
	Entry * current_entry_ptr = first_entry_ptr;
	while (1){
		printf("(%x,%li) -> ",current_entry_ptr->character_code, current_entry_ptr->message_index);
		if ( current_entry_ptr->next_entry_ptr == NULL){
			printf("End\n");
			break;
		}
		current_entry_ptr = current_entry_ptr->next_entry_ptr;
	}
}

Entry * get_last_entry_ptr(Entry * first_entry_ptr){
	Entry * current_entry_ptr = first_entry_ptr;
	while( current_entry_ptr->next_entry_ptr != NULL ){
		current_entry_ptr = current_entry_ptr->next_entry_ptr;
	}
	return current_entry_ptr;
}


void add_entry(Entry * first_entry_ptr, char character_code, long message_index){
	Entry * last_entry_ptr = get_last_entry_ptr(first_entry_ptr);
	Entry * new_entry_ptr = (Entry *) malloc( sizeof(Entry) );
	new_entry_ptr->character_code = character_code;
	new_entry_ptr->message_index = message_index;
	new_entry_ptr->next_entry_ptr = NULL;
	last_entry_ptr->next_entry_ptr = new_entry_ptr;
}


Entry * get_ptr_to_message_index( Entry * first_entry_ptr, long message_index){
	Entry * current_entry_ptr = first_entry_ptr->next_entry_ptr;
	while (1){
		// printf("Current entry: (%x,%li)\n",current_entry_ptr->character_code,current_entry_ptr->message_index);
		if (current_entry_ptr->message_index == message_index){
			//printf("Entry of message index found: (%x,%li)\n",current_entry_ptr->character_code,current_entry_ptr->message_index);
			return current_entry_ptr;
		}
		if (current_entry_ptr->next_entry_ptr == NULL){
			return 0;
		}
		current_entry_ptr = current_entry_ptr->next_entry_ptr;
	}
}

