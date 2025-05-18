#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_FILENAME_LENGTH 256


void fix_filename(char *str) {
    if (str == NULL) return;

    for (char *c = str; *c != '\0'; ++c) {
        if (!isalnum((unsigned char)*c)) {
            *c = '_';
        }
    }
}


char* extract_filename(const char *path) {
    if (path == NULL || strlen(path) == 0) {
        return NULL;
    }

    const char *base = basename((char *)path);
    char *filename   = (char *)malloc(strlen(base) + 1);

    if (filename == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    strcpy(filename, base);
    return filename;
}


void generate_header_file(char *input_filepath, const char *output_dir) {
    FILE *in_file = fopen(input_filepath, "rb");
    if (in_file == NULL) {
        perror("Error opening input file");
        return;
    }

    char *input_filename = extract_filename(input_filepath);
    if (input_filename == NULL) {
        perror("Error extracting base filename");
        return;
    }

    fseek(in_file, 0, SEEK_END);
    size_t file_size = ftell(in_file);
    fseek(in_file, 0, SEEK_SET);

    unsigned char *data = (unsigned char *)malloc(file_size * sizeof(unsigned char));
    if (data == NULL) {
        perror("Error allocating memory");
        fclose(in_file);
        return;
    }

    size_t bytes_read = fread(data, 1, file_size, in_file);
    if (bytes_read != file_size) {
        perror("Error reading file");
        free(data);
        fclose(in_file);
        return;
    }
    fclose(in_file);

    fix_filename(input_filename);
    char output_filename[MAX_FILENAME_LENGTH];
    snprintf(output_filename, sizeof(output_filename), "%s/%s.h", output_dir, input_filename);

    FILE *out_file = fopen(output_filename, "w");
    if (out_file == NULL) {
        perror("Error opening output file");
        free(data);
        return;
    }

    fprintf(out_file, "#ifndef BINARY_DATA_%s_H\n", input_filename);
    fprintf(out_file, "#define BINARY_DATA_%s_H\n", input_filename);
    fprintf(out_file, "#define BINARY_DATA_%s_SIZE %zu\n\n", input_filename, bytes_read);
    fprintf(out_file, "unsigned char binary_data_%s[] = {\n\t", input_filename);

    for (size_t i = 0; i < bytes_read; ++i) {
        if (i % 16 == 0 && i != 0) {
            fprintf(out_file, "\n\t");
        }

        fprintf(out_file, "0x%02X", data[i]);

        if (i != bytes_read - 1) {
            fprintf(out_file, ", ");
        }
    }

    fprintf(out_file, "\n};\n\n");
    fprintf(out_file, "#endif // BINARY_DATA_%s_H\n", input_filename);

    fclose(out_file);
    free(data);
    free(input_filename);

    printf("Header file '%s' generated successfully.\n", output_filename);
}

int main(int argc, char **argv) {
    const char *output_dir = ".";

    if (argc < 2) {
	puts("Usage: blcc [-o <output_dir>] <binary_file>...");
	return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_dir = argv[++i];
            continue;
        }

        DIR *dir = opendir(output_dir);
        if (dir == NULL) {
            if (mkdir(output_dir, 0755) != 0) {
                perror("Error creating output directory");
                return EXIT_FAILURE;
            }
        } else {
            closedir(dir);
        }

        generate_header_file(argv[i], output_dir);
    }

    return EXIT_SUCCESS;
}
