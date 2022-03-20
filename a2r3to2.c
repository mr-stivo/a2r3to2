#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#pragma pack(1)

// A2Rv2
struct A2Rv2_header {
    uint32_t version;
    uint32_t header;
} a2r2_header;

struct A2Rv2_info_chunk {
    uint32_t chunk_id;
    uint32_t chunk_size;
    uint8_t info_version;
    char creator[32];
    uint8_t disk_type;
    uint8_t write_protected;
    uint8_t synchronized;
} a2r2_info_chunk;

struct Strm_chunk {
    uint32_t chunk_id;
    uint32_t chunk_size;
} a2r2_strm_chunk;

struct Strm_capture {
    uint8_t location;
    uint8_t capture_type;
    uint32_t data_length;
    uint32_t estimated_loop_point;
} a2r2_strm_capture;

// A2Rv3
struct A2Rv3_header {
    uint32_t version;
    uint32_t header;
} a2r3_header;

struct A2Rv3_info_chunk {
    uint32_t chunk_id;
    uint32_t chunk_size;
    uint8_t info_version;
    char creator[32];
    uint8_t drive_type;
    uint8_t write_protected;
    uint8_t synchronized;
    uint8_t hard_sector_count;
} a2rv3_info_chunk;

struct Rwcp_chunk {
    uint32_t chunk_id;
    uint32_t chunk_size;
    uint8_t rwcp_version;
    uint32_t resolution;
    char _reserved[11];
} a2rv3_rwcp_chunk;

struct Stream_capture {
    uint8_t capture_type;
    uint16_t location;
    uint8_t number_of_index_signals;
    uint32_t index_signal;
    uint32_t capture_data_size;
} a2rv3_stream_capture;



int main(int argc, char* argv[]) {
    FILE *fpi, *fpo;
    uint8_t mark;
    uint32_t pos, size = 0;

    if(argc == 3) {
        fpi = fopen(argv[1], "r");
        fpo = fopen(argv[2], "w");
    } else if(argc == 1) {
        fpi = stdin;
        fpo = stdout;
    }

    if((fpi == NULL) || (fpo == NULL)) {
        fprintf(stderr, "error: %s input.a2r output.a2r\n", argv[0]);
        exit(1);
    }

    fread(&a2r3_header, sizeof(a2r3_header), 1, fpi);
    a2r2_header.version = 0x32523241;
    a2r2_header.header = a2r3_header.header;
    fwrite(&a2r2_header, sizeof(a2r2_header), 1, fpo);

    fread(&a2rv3_info_chunk, sizeof(a2rv3_info_chunk), 1, fpi);
    a2r2_info_chunk.chunk_id = a2rv3_info_chunk.chunk_id;
    a2r2_info_chunk.chunk_size = 36;
    a2r2_info_chunk.info_version = 1;
    strncpy(a2r2_info_chunk.creator, a2rv3_info_chunk.creator, 32);
    a2r2_info_chunk.disk_type = a2rv3_info_chunk.drive_type;
    a2r2_info_chunk.write_protected = a2rv3_info_chunk.write_protected;
    a2r2_info_chunk.synchronized = a2rv3_info_chunk.synchronized;
    fwrite(&a2r2_info_chunk, sizeof(a2r2_info_chunk), 1, fpo);

    fread(&a2rv3_rwcp_chunk, sizeof(a2rv3_rwcp_chunk), 1, fpi);
    
    a2r2_strm_chunk.chunk_id = 0x4D525453;
    a2r2_strm_chunk.chunk_size = 0;
    fwrite(&a2r2_strm_chunk, sizeof(a2r2_strm_chunk), 1, fpo);
    pos = ftell(fpo) - sizeof(uint32_t);

    fread(&mark, sizeof(uint8_t), 1, fpi);
    while(mark == 'C') {
        fread(&a2rv3_stream_capture, sizeof(a2rv3_stream_capture), 1, fpi);

        uint8_t *data = malloc(a2rv3_stream_capture.capture_data_size);
        fread(data, sizeof(uint8_t), a2rv3_stream_capture.capture_data_size, fpi);

        size += a2rv3_stream_capture.capture_data_size;
        size += 10;

        a2r2_strm_capture.location = a2rv3_stream_capture.location;
        a2r2_strm_capture.capture_type = a2rv3_stream_capture.capture_type;
        a2r2_strm_capture.data_length = a2rv3_stream_capture.capture_data_size;
        a2r2_strm_capture.estimated_loop_point = a2rv3_stream_capture.index_signal;

        fwrite(&a2r2_strm_capture, sizeof(a2r2_strm_capture), 1, fpo);
        fwrite(data, sizeof(uint8_t), a2r2_strm_capture.data_length, fpo);

        free(data);

        fread(&mark, sizeof(uint8_t), 1, fpi);
    }

    mark = 0xff;
    fwrite(&mark, sizeof(uint8_t), 1, fpo);

    fseek(fpo, pos, 0);
    fwrite(&pos, sizeof(uint32_t), 1, fpo);

    fclose(fpi);
    fclose(fpo);
}

