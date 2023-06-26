#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MIN 101


typedef struct shortestPathTreeNode {
	int distance;
	int parent;
} ShortestPathTreeNode;

typedef struct networkLink {
	int node1;
	int node2;
	int cost;
} NetworkLink;


typedef struct routingTableEntry{
	int next,cost;
}RoutingTableEntry;

int node_num;
char message_buffer[1001]; 
NetworkLink links[8000];
int link_num = 0;


RoutingTableEntry** init_table(FILE* t_File);
void print_table(FILE *out_File, RoutingTableEntry **r_table);
void print_message(FILE *m_File, FILE *out_File, RoutingTableEntry** r_table);
void exchange_table(int node_index, RoutingTableEntry **r_table, ShortestPathTreeNode *SPT);
void close_files(FILE* t_File, FILE* m_File, FILE* c_File, FILE* out_File);
void update_table( FILE* m_File, FILE *c_File, FILE * out_File, RoutingTableEntry **r_table);

void update_links(int node_index, int node1, int node2, int cost);

int main(int argc, char *argv[]) {
	if(argc != 4) {
	  	fprintf(stdout, "usage: distvec topologyfile messagesfile changesfile\n");
		return 0;
	}

	FILE *t_File, *m_File, *c_File, *out_File; 
	t_File = fopen(argv[1], "r");
	if (t_File == NULL) {
		fprintf(stdout, "Error: open input file.\n");
	  	return 0;
	}
	m_File = fopen(argv[2], "r");
	if (m_File == NULL) {
		fprintf(stdout, "Error: open input file.\n");
		return 0;

	}
	c_File = fopen(argv[3], "r");
	if (c_File == NULL) {
		fprintf(stdout, "Error: open input file.\n");
		return 0;
	}
	out_File = fopen("output_ls.txt", "wt");
	if (out_File == NULL) {
		fprintf(stderr, "Error: open output file. \n");
		return 0;
	}

	RoutingTableEntry** r_table = init_table(t_File); //create routing table


	print_table(out_File, r_table);
	print_message(m_File, out_File, r_table);
	update_table(m_File, c_File, out_File, r_table);

	
	printf("Complete. Output file written to output_ls.txt\n");

	close_files(t_File, m_File, c_File, out_File);
	return 0;
}

ShortestPathTreeNode* create_SPT(int node_index, int node_num){
	ShortestPathTreeNode *SPT = (ShortestPathTreeNode*)malloc(sizeof(ShortestPathTreeNode)*(node_num));
	for(int i=0; i<node_num; i++){ 
		if(i==node_index){
			SPT[i].distance = 0;
			SPT[i].parent = i; }
		else {
			SPT[i].distance = -999;
			SPT[i].parent = -1; 
		}
	}
	return SPT;
}


RoutingTableEntry** init_table(FILE* t_File){
	fscanf(t_File, "%d", &node_num);
	RoutingTableEntry **r_table = (RoutingTableEntry**)malloc(sizeof(RoutingTableEntry*)*node_num);
	for(int i=0; i<node_num; i++){
		r_table[i] = (RoutingTableEntry*)malloc(sizeof(RoutingTableEntry)*node_num);
		
	}
	for(int i=0; i<node_num; i++){
		for(int j=0; j<node_num; j++){
			if(i==j) {
				r_table[i][j].next = j;
				r_table[i][j].cost = 0;
			}
			else{
				r_table[i][j].next = -1; 
				r_table[i][j].cost = -999; 
			} 
		}
	}

	int node1, node2, cost;
	while(fscanf(t_File, "%d %d %d", &node1, &node2, &cost) != EOF){
		links[link_num].node1 = node1;
		links[link_num].node2 = node2;
		links[link_num].cost = cost;
		link_num++;
	} 

	
	int node_index = 0;
	for(; node_index < node_num; node_index++){
		ShortestPathTreeNode *SPT = create_SPT(node_index, node_num);
		
		for(int i=0; i<link_num; i++){
			if(links[i].node1 == node_index){
				SPT[links[i].node2].distance = links[i].cost;
				SPT[links[i].node2].parent = node_index;
			}
			else if(links[i].node2 == node_index){
				SPT[links[i].node1].distance = links[i].cost;
				SPT[links[i].node1].parent = node_index;
			}
		}

		// (2) Fill table : Exchange
		exchange_table(node_index, r_table, SPT);
		free(SPT);
	}



	return r_table;
}


void print_table(FILE *out_File, RoutingTableEntry **r_table){
	for(int i=0; i<node_num; i++){
		for(int j=0; j<node_num; j++){
			if(r_table[i][j].cost == -999) continue;
			fprintf(out_File, "%d %d %d\n", j, r_table[i][j].next, r_table[i][j].cost);
		}
		fprintf(out_File, "\n");
	}
}


void print_message(FILE *m_File, FILE *out_File, RoutingTableEntry** r_table){
	fseek(m_File, 0, SEEK_SET);
	int src, dest;

	int start = 1;
	while(!feof(m_File)){
		fscanf(m_File, "%d %d ", &src, &dest);
		if(fgets(message_buffer, 1000, m_File) != NULL){
			if(start == 0)
				fprintf(out_File, "\n"); 
			
			message_buffer[strcspn(message_buffer, "\n")] = 0;
			if(r_table[src][dest].cost != -999){
				fprintf(out_File, "from %d to %d cost %d hops ", src, dest, r_table[src][dest].cost);
				while(src!=dest){
					fprintf(out_File, "%d ", src);
					src = r_table[src][dest].next;
				} fprintf(out_File, "message %s", message_buffer);
			}
			else if(r_table[src][dest].cost == -999){
				fprintf(out_File, "from %d to %d cost infinite hops unreachable message %s", src, dest, message_buffer);
			}
		}
		start = 0;
	} 

}

void update_links( int node1, int node2, int cost){
	for(int i=0; i<link_num; i++){
			if(links[i].node1==node1 && links[i].node2==node2){
				links[i].cost = cost;
			}
			else if(links[i].node1==node2 && links[i].node2==node1){
				links[i].cost = cost;
			}
			else {
				links[link_num].node1 = node1;
				links[link_num].node2 = node2;
				links[link_num].cost = cost;
				link_num++;
			}
	}

}



void exchange_table(int node_index, RoutingTableEntry **r_table, ShortestPathTreeNode *SPT){
	int min = MIN;
	int selected_node;
	int selected_list[100];
	selected_list[0] = node_index;
	int selected_num = 1; 


	int continue_outer = 0;
	while (!continue_outer) {
		min = MIN;
		for (int i = 0; i < node_num; i++) {
			if (SPT[i].distance > 0 && SPT[i].distance < min) {
				int continue_inner = 0;
				for (int j = 0; j < selected_num && !continue_inner; j++) {
					if (i == selected_list[j]) {
						continue_inner = 1;
					}
				}
				if (continue_inner) {
					continue;
				}
				min = SPT[i].distance;
				selected_node = i;
			}
		}
		selected_list[selected_num] = selected_node;
		selected_num++;
		if (selected_num == node_num) {
			continue_outer = 1;
		}

		
		for (int i = 0; i < link_num; i++) {
		int node1 = links[i].node1;
		int node2 = links[i].node2;
		int distance1 = SPT[node1].distance;
		int distance2 = SPT[node2].distance;

		if (node1 == selected_node) {
			if (distance2 == -999 || SPT[selected_node].distance + links[i].cost < distance2) {
				int new_distance = SPT[selected_node].distance + links[i].cost;
				if (new_distance < 0) {
					continue;
				}
				SPT[node2].distance = new_distance;
				SPT[node2].parent = selected_node;
			}
		} else if (node2 == selected_node) {
			if (distance1 == -999 || SPT[selected_node].distance + links[i].cost < distance1) {
				int new_distance = SPT[selected_node].distance + links[i].cost;
				if (new_distance < 0) {
					continue;
				}
				SPT[node1].distance = new_distance;
				SPT[node1].parent = selected_node;
			}
		}
		}
		
	}


	int temp_index;
	for (int i = 0; i < node_num; i++) {
    if (i != node_index) {
        if (SPT[i].distance < 0) {
            continue;
        }
        temp_index = i;
        int current_parent = SPT[temp_index].parent;
        while (current_parent != node_index) {
            temp_index = current_parent;
            current_parent = SPT[temp_index].parent;
        }
        r_table[node_index][i].next = temp_index;
        r_table[node_index][i].cost = SPT[i].distance;
    	}
	}

}

void update_table( FILE* m_File, FILE *c_File, FILE * out_File, RoutingTableEntry **r_table){
	//make default table
	for(int i=0; i<node_num; i++){
		for(int j=0; j<node_num; j++){
			if(i==j) {
				r_table[i][j].next = j;
				r_table[i][j].cost = 0;
			}
			else{
				r_table[i][j].next = -1; 
				r_table[i][j].cost = -999; 
			} 
		}
	}
	int node1, node2, cost;
	while(fscanf(c_File, "%d %d %d", &node1, &node2, &cost) != EOF){
		update_links(node1, node2, cost);
		int node_index = 0;
		for( ; node_index < node_num; node_index++){
			ShortestPathTreeNode* SPT = create_SPT(node_index, node_num);
			for(int i=0; i<link_num; i++){
				if(links[i].node1 == node_index){
					SPT[links[i].node2].distance = links[i].cost;
					SPT[links[i].node2].parent = node_index;
				}
				else if(links[i].node2 == node_index){
					SPT[links[i].node1].distance = links[i].cost;
					SPT[links[i].node1].parent = node_index;
				}
			}
			exchange_table(node_index, r_table, SPT);
			free(SPT);
		}
			fprintf(out_File, "\n\n");

			print_table(out_File, r_table);
			print_message(m_File, out_File, r_table);
	} 

}


void close_files(FILE* t_File, FILE* m_File, FILE* c_File, FILE* out_File){
	fclose(t_File);
	fclose(m_File);
	fclose(c_File);
	fclose(out_File);
	
}