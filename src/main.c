#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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


void generate_header_file(char *input_filepath, const char *output_dir, bool is_image) {
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

    unsigned char *data;
    size_t file_size = 0, bytes_read = 0;
    int    image_w = 0,  image_h = 0;

    if (is_image) {
        // 1. read pixels with stb_image
        char *dot = strrchr(input_filepath, '.');
        bool flip_h = (dot && !strcmp(dot, ".png"));

        stbi_set_flip_vertically_on_load_thread(flip_h);

        int channels = 0;
        data = stbi_load_from_file(in_file, &image_w, &image_h, &channels, 0);

        if (data == NULL) {
            perror("Error reading image!");
            fclose(in_file);
            return;
        }

        file_size = image_w * image_h * channels;
        bytes_read = image_w * image_h * channels;

        // 2. store pixels in the header instead
    } else {
        fseek(in_file, 0, SEEK_END);
        file_size = ftell(in_file);
        fseek(in_file, 0, SEEK_SET);

        data = (unsigned char *)malloc(file_size * sizeof(unsigned char));
        if (data == NULL) {
            perror("Error allocating memory");
            fclose(in_file);
            return;
        }

        bytes_read = fread(data, 1, file_size, in_file);
    }

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
    fprintf(out_file, "#define BINARY_DATA_%s_SIZE %zu\n", input_filename, bytes_read);

    if (is_image) {
        fprintf(out_file, "#define BINARY_DATA_%s_IMAGE_WIDTH %d\n", input_filename, image_w);
        fprintf(out_file, "#define BINARY_DATA_%s_IMAGE_HEIGHT %d\n\n", input_filename, image_h);
    }

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
    bool is_image = false;

    if (argc < 2) {
	puts("Usage: blcc [-o <output_dir>] [--image] <binary_file>...");
	return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_dir = argv[++i];
            continue;
        }

        if (strcmp(argv[i], "--image") == 0 && i + 1 < argc) {
            is_image = true;
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

        generate_header_file(argv[i], output_dir, is_image);
    }

    return EXIT_SUCCESS;
}
