#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

using namespace std;

typedef int vertex_t;
typedef int index_t;

//typedef struct packed_edge {
//	  long int v0;
//	  long int v1;
//} packed_edge;

inline off_t fsize(const char *filename) {
    struct stat st; 
    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1; 
}

		
int main(int argc, char** argv){
	int fd,i;
	char* ss_head;
	char* ss;

	size_t file_size = fsize(argv[1]);


	fd=open( argv[1],O_CREAT|O_RDWR,00666 );



	ss_head = (char*)mmap(NULL,file_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);

	size_t head_offset=0;
	while(ss_head[head_offset]=='%'){
		while(ss_head[head_offset]!='\n'){
			head_offset++;
		}
		head_offset++;
	}
	ss = &ss_head[head_offset];
	file_size -= head_offset;

	size_t curr=0;
	size_t next=0;

	//step 1. vert_count,edge_count,
	size_t edge_count=0;
	size_t vert_count;
	vertex_t v_max = 0;
	vertex_t v_min = 999999;//as infinity
	vertex_t a;
	while(next<file_size){
		char* sss=ss+curr;
		a = atoi(sss);

		if(v_max<a){
			v_max = a;
		}
		if(v_min>a){
			v_min = a;
		}

		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;
		}
		curr = next;
		edge_count++;
	}
	edge_count /=2;
	vert_count = v_max - v_min + 1;
	cout<<"edge count: "<<edge_count<<endl;
	cout<<"max vertex id: "<<v_max<<endl;
	cout<<"min vertex id: "<<v_min<<endl;

	cout<<"edge count: "<<edge_count<<endl;
	cout<<"vert count: "<<vert_count<<endl;
	//step 2. each file size
	int fd4 = open( "bw_adjacent.bin",O_CREAT|O_RDWR,00666 );
	ftruncate(fd4, edge_count*sizeof(vertex_t));
	vertex_t* adj = (vertex_t*)mmap(NULL,edge_count*sizeof(vertex_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd4,0);
	
	int fd5 = open( "bw_head.bin",O_CREAT|O_RDWR,00666 );
	ftruncate(fd5, edge_count*sizeof(vertex_t));
	vertex_t* head = (vertex_t*)mmap(NULL,edge_count*sizeof(vertex_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd5,0);

	int fd2 = open( "in_degree.bin",O_CREAT|O_RDWR,00666 );
	ftruncate(fd2, vert_count*sizeof(index_t));
	index_t* degree = (index_t*)mmap(NULL,vert_count*sizeof(index_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd2,0);
	
	int fd3 = open( "bw_begin.bin",O_CREAT|O_RDWR,00666 );
	ftruncate(fd3, (vert_count+1)*sizeof(index_t));
	index_t* begin  = (index_t*)mmap(NULL,(vert_count+1)*sizeof(index_t),PROT_READ|PROT_WRITE,MAP_SHARED,fd3,0);
	
	//step 3. count degree --> extend to indegree & outdegree
	for(int i=0; i<vert_count;i++){
		degree[i]=0;
	}
	vertex_t index;
	vertex_t index_a;
	size_t offset =0;
	curr=0;
	next=0;
	while(offset<edge_count){
		char* sss=ss+curr;
		index = atoi(sss)-v_min;
		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;
		}
		curr = next;

		char* sss1=ss+curr;
		index_a = atoi(sss1)-v_min;
		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;
		}
		curr = next;
//		degree[index]++; /// out degree
		degree[index_a]++;/// in degree
//		cout<<index<<" "<<degree[index]<<endl;

		offset++;
	}
//	exit(-1);
	begin[0]=0;
	begin[vert_count]=edge_count;
	for(size_t i=1; i<vert_count; i++){
		begin[i] = begin[i-1] + degree[i-1];
//		cout<<"begin: " <<begin[i]<<" "<<degree[i]<<endl;
		degree[i-1] = 0; ///
	}
	degree[vert_count-1] = 0;
	//step 4: write adjacent list 
	vertex_t v_id;
	offset =0;
	next = 0;
	curr = 0;
	while(offset<edge_count){
		char* sss=ss+curr;
		index = atoi(sss)-v_min;
		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;
		}
		curr = next;

		char* sss1=ss+curr;
		v_id = atoi(sss1)-v_min;
//		adj[begin[index]+degree[index]] = v_id;
//		head[begin[index]+degree[index]]= index;
		//reverse edge
		adj[begin[v_id]+degree[v_id]] = index;
		head[begin[v_id]+degree[v_id]]= v_id;
		while((ss[next]!=' ')&&(ss[next]!='\n')&&(ss[next]!='\t')){
			next++;
		}
		while((ss[next]==' ')||(ss[next]=='\n')||(ss[next]=='\t')){
			next++;
		}
		curr = next;
		degree[v_id]++;

		offset++;
	}
	
	//step 5
	//print output as a test
	for(size_t i=0; i<=vert_count && i<8; i++){
        for(index_t j=0; j<degree[i]; ++j)
            cout<<i<<"\t"<<begin[i]<<"\t"<<adj[begin[i] + j]<<endl;
    }
//	for(size_t i=0; i<8; i++){
//		cout<<begin[i]<<" "<<degree[i]<<" ";
//		for(index_t j=0; j<degree[i]; j++){
//			cout<<head[begin[i]+j]<<"-"<<adj[begin[i]+j]<<" ";
//		}
//		if(degree[i]>0){
//			cout<<endl;
//		}
	//}
	cout<<begin[vert_count]<<endl;

	for(int i=0; i<edge_count && i<64; i++){
//	for(int i=0; i<64; i++){
		cout<<head[i]<<"\t"<<adj[i]<<endl;
	}


//--------------------------------------------------

	vertex_t *sub_graph=new vertex_t[vert_count+1];
	for(int i=0;i<vert_count;i++){
		sub_graph[i]=0;
	}

	size_t sub_file_size = fsize(argv[2]);

	int sub_fd=open(argv[2],O_CREAT|O_RDWR,00666 );

    char *sub_ss_head;

	sub_ss_head = (char*)mmap(NULL,file_size,PROT_READ|PROT_WRITE,MAP_SHARED,sub_fd,0);

	size_t sub_head_offset=0;
	while(sub_ss_head[head_offset]=='%'){
		while(sub_ss_head[head_offset]!='\n'){
			sub_head_offset++;
		}
		sub_head_offset++;
	}
	char *sub_ss = &sub_ss_head[sub_head_offset];
	sub_file_size -= sub_head_offset;

    size_t sub_curr=0;
    size_t sub_next=0;
    size_t sub_vert_count=0;

    while(sub_next<sub_file_size){
		char* sss=sub_ss+sub_curr;
		a = atoi(sss);
		std::cout<<"sub_graph==1"<<" "<<a<<std::endl;

		while((sub_ss[sub_next]!=' ')&&(sub_ss[sub_next]!='\n')&&(sub_ss[sub_next]!='\t')){
			sub_next++;
		}
		while((sub_ss[sub_next]==' ')||(sub_ss[sub_next]=='\n')||(sub_ss[sub_next]=='\t')){
			sub_next++;
		}
		sub_curr = sub_next;
        sub_graph[a]=1;
        ++sub_vert_count;
	}

   
	index_t* sub_indegree=new index_t[vert_count+1];

	for(int i=0; i<vert_count;i++){
		sub_indegree[i]=0;
	}

    for(size_t vert_id=0;vert_id<vert_count;vert_id++)
    {
        if(sub_graph[vert_id]==1)
        {
            index_t my_beg = begin[vert_id];
            index_t my_end = begin[vert_id+1];
            for(; my_beg<my_end; my_beg++)
            {
                vertex_t nebr=adj[my_beg];
                if(sub_graph[nebr]==1)
                {
                    sub_indegree[vert_id]++;
                }
            }
        }
    }

    int sub_fw_beg_fd = open( "sub_bw_begin.bin",O_CREAT|O_RDWR,00666 );
	ftruncate(sub_fw_beg_fd, (vert_count+1)*sizeof(index_t));
	index_t* sub_fw_begin  = (index_t*)mmap(NULL,(vert_count+1)*sizeof(index_t),PROT_READ|PROT_WRITE,MAP_SHARED,sub_fw_beg_fd,0);

	int sub_fw_adj_fd = open( "sub_bw_adjacent.bin",O_CREAT|O_RDWR,00666 );
	ftruncate(sub_fw_adj_fd, edge_count*sizeof(vertex_t));
	vertex_t* sub_fw_adj = (vertex_t*)mmap(NULL,edge_count*sizeof(vertex_t),PROT_READ|PROT_WRITE,MAP_SHARED,sub_fw_adj_fd,0);



    sub_fw_begin[0]=0;
	for(size_t i=1; i<vert_count; i++){
		sub_fw_begin[i] = sub_fw_begin[i-1] + sub_indegree[i-1];
//		cout<<"begin: " <<begin[i]<<" "<<degree[i]<<endl;
		sub_indegree[i-1] = 0; ///
	}
	sub_indegree[vert_count-1] = 0;

	index_t sub_edge_count=0;
    for(size_t vert_id=0;vert_id<vert_count;vert_id++)
    {
        if(sub_graph[vert_id]==1)
        {
			std::cout<<"vert_id==1: "<<vert_id<<" ";
            index_t my_beg = begin[vert_id];
            index_t my_end = begin[vert_id+1];
            for(; my_beg<my_end; my_beg++)
            {
                vertex_t nebr=adj[my_beg];
                if(sub_graph[nebr]==1)
                {
					cout<<" index: "<<sub_fw_begin[vert_id]+sub_indegree[vert_id]<<" nebr: "<<nebr<<std::endl;
                    sub_fw_adj[sub_fw_begin[vert_id]+sub_indegree[vert_id]]=nebr;
                    ++sub_indegree[vert_id];
					++sub_edge_count;
                }
            }
        }
    }

	sub_fw_begin[vert_count]=sub_edge_count;

	for(size_t vert_id=0;vert_id<vert_count;vert_id++)
    {
            index_t my_beg = sub_fw_begin[vert_id];
            index_t my_end = sub_fw_begin[vert_id+1];
            for(; my_beg<my_end; my_beg++)
            {
                vertex_t nebr=sub_fw_adj[my_beg];
                std::cout<<"vert_id:"<<vert_id<<" nebr"<<nebr<<std::endl;
            }
    }
    

	
	munmap(sub_fw_begin,sizeof(index_t)*vert_count+1);
	close(sub_fw_beg_fd);

	munmap(sub_fw_adj,edge_count*sizeof(vertex_t));
	close(sub_fw_adj_fd);



	munmap( ss,sizeof(char)*file_size );
	munmap( adj,sizeof(vertex_t)*edge_count );
	munmap( head,sizeof(vertex_t)*edge_count );
	munmap( begin,sizeof(index_t)*vert_count+1 );
	munmap( degree,sizeof(index_t)*vert_count );
	close(fd2);
	close(fd3);
	close(fd4);
	close(fd5);
    return 0;
}
