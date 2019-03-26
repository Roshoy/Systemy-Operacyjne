struct mem_block_arr{
    char** arr;
    unsigned int max_size;
    unsigned int first_null;
};

void create(unsigned int size);

int add_mem_block();

void set_next_null();

void remove_mem_block(unsigned int i);

void set_curr_dir(char* new_dir);

void set_curr_file(char* new_file);

void set_tmp_file(char* new_tmp_file);

void set_command();

void search(char* arg_dir, char* arg_file, char* new_tmp_file);

char* get_mem_block(unsigned int i);

void clear_mem();