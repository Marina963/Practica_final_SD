struct arg{
    char user[256];
    char op[16];
    char date[32];
    char f_name[256];
};


program INFO{
    version INFOVER{
        void print_info_x(struct arg) = 1;
    } = 1;
} = 99;