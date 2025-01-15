#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libelf.h>
#include <gelf.h>

unsigned long find_function_address(const char *path_to_bin, const char *function){
    unsigned long res;

    // ELF init
    if (elf_version(EV_CURRENT) == EV_NONE){
        fprintf(stderr, "ELF library initialization failed.\n");
        exit(1);
    }

    // Odpremo bin datoteko
    FILE *bin_file = fopen(path_to_bin,"rb");
    if (!bin_file){
        perror("Failed to open binary");
        exit(1);
    }

    // Init strukture, podamo deskriptor
    Elf *elf = elf_begin(fileno(bin_file), ELF_C_READ, NULL);
    if (!elf) {
        fprintf(stderr, "Failed to read ELF file: %s\n", elf_errmsg(-1));
        fclose(bin_file);
        exit(1);
    }

    // Podamo indeks tabele headerjev
    size_t shstrndx;
    if (elf_getshdrstrndx(elf, &shstrndx) < 0){
        fprintf(stderr, "Failed to get section header string index: %s\n", elf_errmsg(-1));
        elf_end(elf);
        fclose(bin_file);
        exit(1);
    }

    Elf_Scn *section = NULL;
    GElf_Shdr shdr;

    // Iteracija cez sekcije 
    while ((section = elf_nextscn(elf, section)) != NULL){
        // Beremo zaglavje trenutne sekcije
        if (gelf_getshdr(section, &shdr) != &shdr){
            fprintf(stderr, "Failed to read section header\n");
            elf_end(elf);
            fclose(bin_file);
            exit(1);
        }
        // Ali smo v tabeli simbolov? (Staticna || V izvajanju), preberemo simbole
        if (shdr.sh_type == SHT_SYMTAB || shdr.sh_type == SHT_DYNSYM){
            Elf_Data *data = elf_getdata(section, NULL);
            if (!data){
                fprintf(stderr, "Failed to get section data\n");
                elf_end(elf);
                fclose(bin_file);
                exit(1);
            }
            // Beremo simbole, primerjamo z vhodnim nizom, vrnemo naslov pravega simbola
            int symbols = shdr.sh_size / shdr.sh_entsize;
            for (int i = 0; i < symbols; i++){
                GElf_Sym sym;
                if (gelf_getsym(data, i, &sym) != &sym){
                    fprintf(stderr, "Failed to get symbol\n");
                    continue;
                }

                const char *name = elf_strptr(elf, shdr.sh_link, sym.st_name);
                if (name && strcmp(name, function) == 0){
                    printf("Symbol VA: 0x%lx\n", sym.st_value);
                    elf_end(elf);
                    fclose(bin_file);
                    return sym.st_value;
                }
            }
        }
    }

    fprintf(stderr, "Function %s not found in the binary.\n", function);
    elf_end(elf);
    fclose(bin_file);
}

void patch(unsigned char *bufPtr){
    for(size_t i = 0; i < 5; i++){
        bufPtr[i] = 0x90;
    }
}

int main(int argc, char *argv[]){
    if (argc != 3) {
        printf("Run with arguments: %s <binary> <function_name>\n", argv[0]);
        exit(1);
    }

    
    size_t res = (size_t)find_function_address(argv[1], argv[2]);
    //printf("%zu\n", res);

    const char *filename = argv[1];
    FILE *programfile = fopen(filename, "rb");
    if (!programfile){
        perror("Failed to open");
        return EXIT_FAILURE;
    }

    //Določim velikost vhodne datoteke
    fseek(programfile, 0, SEEK_END);
    long size = ftell(programfile);
    rewind(programfile);

    unsigned char *buffer = (unsigned char *)malloc(size);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(programfile);
        exit(1);
    }

    size_t bytes_read = fread(buffer, 1, size, programfile);
    if (bytes_read != size) {
        perror("Error reading file");
        free(buffer);
        fclose(programfile);
        exit(1);
    }
   
    //Precesamo progam v bajtih
    for (size_t i = res; i < (size_t)size; i++) {
        if(buffer[i] == 0xe8){
            patch(&buffer[i]);
            break;
        }
    }


    // Write the modified buffer to a new file "b.out"
    FILE *output_file = fopen("b.out", "wb");
    if (!output_file) {
        perror("Failed to open output file");
        free(buffer);
        fclose(programfile);
        exit(1);
    }

    size_t bytes_written = fwrite(buffer, 1, size, output_file);
    if (bytes_written != size) {
        perror("Error writing to output file");
        free(buffer);
        fclose(programfile);
        fclose(output_file);
        exit(1);
    }

    printf("Patched binary saved to b.out\n");
    
    free(buffer);
    fclose(programfile);
    return 0;
}