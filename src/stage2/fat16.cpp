#include <cstdint>
namespace {
	struct [[gnu::packed]] {
		uint8_t  jmp_boot[3];
		char     oem_name[8];
		uint16_t bytes_per_sector;
		uint8_t  sectors_per_cluster;
		uint16_t reserved_sectors;
		uint8_t  num_fats;
		uint16_t root_entries;
		uint16_t total_sectors_16;
		uint8_t  media_type;
		uint16_t sectors_per_fat;
		uint16_t sectors_per_track;
		uint16_t num_heads;
		uint32_t hidden_sectors;
		uint32_t total_sectors_32;
		
		uint8_t  drive_number;
		uint8_t  reserved;
		uint8_t  boot_signature;
		uint32_t volume_id;
		char     volume_label[11];
		char     file_system_type[8];
	} fat16_boot_sector;

	struct [[gnu::packed]] fat16_dirent {
		char     filename[8];
		char     extension[3];
		uint8_t  attributes;
		uint8_t  reserved;
		uint8_t  creation_time_tenths;
		uint16_t creation_time;
		uint16_t creation_date;
		uint16_t last_access_date;
		uint16_t first_cluster_high;
		uint16_t last_modification_time;
		uint16_t last_modification_date;
		uint16_t first_cluster_low;
		uint32_t file_size;
	};
