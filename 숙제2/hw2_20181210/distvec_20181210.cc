


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int node_num;
char message_buffer[1001]; 



typedef struct routingTableEntry{
	int next,cost;
}RoutingTableEntry;

RoutingTableEntry** init_table(FILE* t_File);
void print_table(FILE *out_File, RoutingTableEntry **r_table);
void print_message(FILE *m_File, FILE *out, RoutingTableEntry** r_table);
void update_table(FILE *t_File, RoutingTableEntry **r_table);
void change_table( RoutingTableEntry **r_table, int node1, int node2, int cost);
void exchange_table(RoutingTableEntry **r_table);
void close_files(FILE* t_File, FILE* m_File, FILE* c_File, FILE* out_File);

int main(int argc, char *argv[]) {
	if(argc != 4) {
	  	fprintf(stdout, "usage: distvec topologyfile messagesfile changesfile\n");
		return 0;
	}

	FILE *t_File, *m_File, *c_File; 
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

	RoutingTableEntry** r_table = init_table(t_File); 


	FILE *out_File;
	out_File = fopen("output_dv.txt", "wt");
	if (out_File == NULL) {
		fprintf(stdout, "Error: open output file. \n");
		return(0); 
	}
	print_table(out_File, r_table);
	print_message(m_File, out_File, r_table);

	int node1, node2, new_cost;
	while(fscanf(c_File, "%d %d %d", &node1, &node2, &new_cost) != EOF){
		free(r_table);
		r_table = init_table(t_File); 

		update_table(t_File, r_table);

		change_table(r_table, node1, node2, new_cost); 
		exchange_table(r_table); 
		fprintf(out_File, "\n\n");
		print_table(out_File, r_table); 

		print_message(m_File, out_File, r_table);

	}

	fprintf(stdout, "Complete. Output file written to output_dv.txt\n");
	close_files(t_File, m_File, c_File, out_File);
	return 0;
}



RoutingTableEntry** init_table(FILE* t_File){
	fscanf(t_File, "%d", &node_num);
	RoutingTableEntry **r_table = (RoutingTableEntry**)malloc(sizeof(RoutingTableEntry*)*node_num);

	for(int i=0; i<node_num; i++){
		r_table[i] = (RoutingTableEntry*)malloc(sizeof(RoutingTableEntry)*node_num);
		
	}

	for(int i=0; i<node_num; i++){
		for(int j = 0; j < node_num; j++){
			if(i == j){
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
	while(!feof(t_File)){
		fscanf(t_File, "%d %d %d", &node1, &node2, &cost);
		change_table(r_table, node1, node2, cost);	
	} 

	


	exchange_table(r_table);


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


void print_message(FILE *m_File, FILE *out, RoutingTableEntry** r_table){
	fseek(m_File, 0, SEEK_SET);
	int src, dest;

	int start = 1;
	while(!feof(m_File)){
		fscanf(m_File, "%d %d ", &src, &dest);
		if(fgets(message_buffer, 1000, m_File) != NULL){
			if(start == 0)
				fprintf(out, "\n"); 
			
			message_buffer[strcspn(message_buffer, "\n")] = 0;
			if(r_table[src][dest].cost != -999){
				fprintf(out, "from %d to %d cost %d hops ", src, dest, r_table[src][dest].cost);
				while(src!=dest){
					fprintf(out, "%d ", src);
					src = r_table[src][dest].next;
				} fprintf(out, "message %s", message_buffer);
			}
			else if(r_table[src][dest].cost == -999){
				fprintf(out, "from %d to %d cost infinite hops unreachable message %s", src, dest, message_buffer);
			}
		}
		start = 0;
	} 

}

void update_table(FILE *t_File, RoutingTableEntry **r_table){
	fseek(t_File, 0, SEEK_SET);
	fscanf(t_File, "%d", &node_num);
	int node1, node2, cost;
	while(!feof(t_File)){
		fscanf(t_File, "%d %d %d", &node1, &node2, &cost);
		r_table[node1][node2].next = node2;
		r_table[node1][node2].cost = cost;

		r_table[node2][node1].next = node1;
		r_table[node2][node1].cost = cost;

		
	} 

}



void change_table( RoutingTableEntry **r_table, int node1, int node2, int cost){
	r_table[node1][node2].next = node2;
	r_table[node1][node2].cost = cost;
	r_table[node2][node1].next = node1;
	r_table[node2][node1].cost = cost;
}

void exchange_table(RoutingTableEntry **r_table){


	int loop_flag = 1;
	while (loop_flag) {
		loop_flag = 0;
		for (int i = 0; i < node_num; i++) {
			for (int j = 0; j < node_num; j++) {
				if (j == r_table[i][j].next && r_table[i][j].cost != 0) {
					for (int k = 0; k < node_num; k++) {
						if (r_table[i][k].cost == -999 || r_table[i][j].cost + r_table[j][k].cost < r_table[i][k].cost) {
							if (r_table[i][j].cost + r_table[j][k].cost >= 0) {
								r_table[i][k].next = j;
								r_table[i][k].cost = r_table[i][j].cost + r_table[j][k].cost;
								loop_flag = 1;
							}
						}
					}
				}
			}
		}
	}
}

void close_files(FILE* t_File, FILE* m_File, FILE* c_File, FILE* out_File){
	fclose(t_File);
	fclose(m_File);
	fclose(c_File);
	fclose(out_File);
	
}