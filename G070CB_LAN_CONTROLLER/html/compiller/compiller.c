//
// Created by Andrei Fedorov on 16.11.2022.
//

#include "compiller.h"

/* секция прототипов */
uint32_t get_address(int argc, char **argv);   /* получение адреса из аргументов */
void print_help(char *s);                       /* вывод справочной информации */
uint32_t get_fileList(fileList **fList, char *dirName); /*чтение списка файлов*/


int main(int argc, char *argv[]) {
    //  читаем адрес
    uint32_t startaddr = get_address(argc, argv);
    if (!startaddr) {
        print_help(argv[0]);
        return 0;
    }

    //  читаем список файлов
    fileList *fList = malloc(sizeof(fileList));
    fList->next = NULL;
    uint32_t nFiles = get_fileList(&fList, argv[1]);
    if (!nFiles) {
        print_help(argv[0]);
        return 0;
    }

    // создаем выходной файл
    FILE *f_out = fopen("out.bin", "wb");
    fwrite(&nFiles, 4, 1, f_out); // кол-во файлов
    fileList *fl_pointer = fList;
    // пишем данные о файлах
    while (fl_pointer != NULL) {
        fwrite(fl_pointer->info, sizeof(fileInfo), 1, f_out);
        fl_pointer = fl_pointer->next;
    }

    //пишем файлы
    char fileName[500];
    strcpy(fileName, argv[1]);
    int namePointer = strlen(fileName);
    fl_pointer = fList;
    uint32_t address = startaddr+4+nFiles*sizeof (fileInfo);
    while (fl_pointer != NULL) {
        uint32_t count = 1;
        strcpy(&(fileName[namePointer]), fl_pointer->info->name);
        char buffer1;
        FILE *f_in = fopen(fileName, "r");
        while ((buffer1 = getc(f_in)) != EOF) {
            fwrite(&buffer1, 1, 1, f_out);
            count++;
        }
        fclose(f_in);
        fwrite("\0", 1, 1, f_out);
        printf("файл :%s, размер %u, адрес %08x\n", fileName, count-1, address);
        fl_pointer->info->address=address;
        address+=count;
        fl_pointer = fl_pointer->next;
           }


    // перезаписываем заголовки
    fseek(f_out,4,SEEK_SET);
    fl_pointer = fList;
    while (fl_pointer != NULL) {
        fwrite(fl_pointer->info, sizeof(fileInfo), 1, f_out);
        fl_pointer = fl_pointer->next;
    }

    fclose(f_out);

    while (fList != NULL) {
        // printf("%s\n", fList->info->name);
        fList = fList->next;
    }

    printf("конец размещения файла: %08x\n",  address);

    return 0;
}


uint32_t get_address(int argc, char **argv) {
    if (argc != 3) return 0;
    char *arg = argv[2];
    uint32_t result = 0;
    while (*arg != 0) {
        result *= 16;
        if (*arg >= '0' && *arg <= '9')
            result += *arg - '0';
        else if (*arg >= 'a' && *arg <= 'f')
            result += *arg - 'a' + 10;
        else if (*arg >= 'A' && *arg <= 'F')
            result += *arg - 'A' + 10;
        else if (*arg != 'x')
            return 0;
        arg++;
    }
    return result;
}

uint32_t get_fileList(fileList **fList, char *dirName) {
    uint32_t result = 0;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(dirName)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == 8 && ent->d_name[0] != '.' && strlen(ent->d_name)<20) {
                (*fList)->info = malloc(sizeof(fileInfo));
                strlcpy((*fList)->info->name, ent->d_name, 20);
                fileList *new_file = malloc(sizeof(fileList));
                new_file->next = *fList;
                *fList = new_file;
                result++;
            };
        }
        closedir(dir);
        fileList *new_file = *fList;
        *fList = (*fList)->next;
        free(new_file);

    } else {
        return 0;
    }
    return result;
}

void print_help(char *s) {
    char *s1 = s;
    while (*s1 != 0) {
        if (*s1 == '/') s = s1 + 1;
        s1++;
    }
    printf("используйте синтаксис: \n%s  dir addr \ndir - каталог с файлами;\naddr - адрес записи блока в контроллер\n",
           s);
    return;
}


