#pragma once

#include <stdint.h>

#include <common_attributes.h>

namespace Kernel
{
	class AHCIController;
	class AHCIPort;

	namespace AHCI
	{
		enum class DeviceDetection : uint32_t
		{
			None = 0,
			NoCommunication = 1,
			Present = 3,
			Offline = 4,
		};

		enum class DevicePowerState : uint32_t
		{
			NotPresent = 0,
			Active = 1,
			Partial = 2,
			Slumber = 6,
			DevSleep = 8,
		};

		typedef volatile struct __port_sstatus_t
		{
			DeviceDetection det : 4;        // device detection
			uint32_t spd : 4;               // current interface speed
			DevicePowerState ipm : 4, : 20; // interface power management
		} __packed port_sstatus_t;

		typedef volatile struct __port_command_t
		{
			uint32_t st : 1;       // start
			uint32_t sud : 1;      // spin-up device
			uint32_t pod : 1;      // power on device
			uint32_t clo : 1;      // command list override
			uint32_t fre : 1, : 3; // FIS receive enable
			uint32_t ccd : 5;      // current command slot
			uint32_t mpss : 1;     // mechanical presence switch state
			uint32_t fr : 1;       // FIS receive running
			uint32_t cr : 1;       // command list running
			uint32_t cps : 1;      // cold presence state
			uint32_t pma : 1;      // port multiplier attached
			uint32_t hpcp : 1;     // hot plug capable port
			uint32_t mpsp : 1;     // mechanical presence switch attached to port
			uint32_t cpd : 1;      // cold presence detection
			uint32_t esp : 1;      // external SATA port
			uint32_t fbscp : 1;    // FIS-based switching capable port
			uint32_t apste : 1;    // automatic partial to slumber transition enabled
			uint32_t atapi : 1;    // device is ATAPI
			uint32_t dlae : 1;     // drive LED on ATAPI enable
			uint32_t alpe : 1;     // aggressive link power management enable
			uint32_t asp : 1;      // aggressive slumber/partial
			uint32_t icc : 4;      // interface communication control
		} __packed port_command_t;

		typedef volatile struct __hba_port_t
		{
			uint32_t clb;        // command list base address
			uint32_t clbu;       // command list base address high dword
			uint32_t fb;         // FIS base address
			uint32_t fbu;        // FIS base address high dword
			uint32_t is;         // interrupt status
			uint32_t ie;         // interrupt enable
			port_command_t cmd;        // command and status
			uint32_t rsv0;       // reserved
			uint32_t tfd;        // task file data
			uint32_t sig;        // signature
			port_sstatus_t ssts; // 0SATA status (SCR0:SStatus)
			uint32_t sctl;       // SATA control (SCR2:SControl)
			uint32_t serr;       // SATA error (SCR1:SError)
			uint32_t sact;       // SATA active (SCR3:SActive)
			uint32_t ci;         // command issue
			uint32_t sntf;       // SATA notification (SCR4:SNotification)
			uint32_t fbs;        // FIS-based switch control
			uint32_t rsv1[11];   // reserved
			uint32_t vendor[4];  // vendor specific
		} __packed hba_port_t;

		typedef volatile struct __command_header_t
		{
			uint32_t cfl : 5;    // command FIS length
			uint32_t a : 1;      // ATAPI
			uint32_t w : 1;      // write
			uint32_t p : 1;      // prefetchable
			uint32_t r : 1;      // reset
			uint32_t b : 1;      // BIST
			uint32_t c : 1, : 1; // clear busy on R_OK
			uint32_t pmp : 4;    // port multiplier port
			uint32_t prdtl : 16; // physical region descriptor table length
			uint32_t prdbc;      // physical region descriptor byte count
			uint32_t ctba;       // command table descriptor base address
			uint32_t cbtau;      // cbta high dword
			uint32_t rsv[4];     // reserved
		} __packed command_header_t;

		enum class FisType : uint8_t
		{
			RegisterH2D = 0x27,
			RegisterD2H = 0x34,
			DMAActivate = 0x39,
			DMASetup = 0x41,
			Data = 0x46,
			BISTActivate = 0x58,
			PIOSetup = 0x5f,
			SetDeviceBits = 0xA6,
		};

		typedef volatile struct __h2d_register_fis_t
		{
			FisType type;
			uint8_t pmport : 4, : 3; // port multiplier port
			uint8_t c : 1;           // command bit
			uint8_t command;
			uint8_t featuresl; // features low byte
			uint8_t lba0;
			uint8_t lba1;
			uint8_t lba2;
			uint8_t device;
			uint8_t lba3;
			uint8_t lba4;
			uint8_t lba5;
			uint8_t featuresh; // features high byte
			uint16_t count;
			uint8_t icc; // isochronous command completion
			uint8_t control;
			uint32_t auxiliary;
		} __packed h2d_register_fis_t;

		typedef volatile struct __d2h_register_fis_t
		{
			FisType type;
			uint8_t pmport : 4, : 2; // port multiplier port
			uint8_t i : 1, : 1;      // interrupt bit
			uint8_t status;
			uint8_t error;
			uint8_t lba0;
			uint8_t lba1;
			uint8_t lba2;
			uint8_t device;
			uint8_t lba3;
			uint8_t lba4;
			uint8_t lba5;
			uint8_t rsv1;
			uint16_t count;
			uint16_t rsv2;
			uint32_t rsv3;
		} __packed d2h_register_fis_t;

		typedef volatile struct __data_fis_t
		{
			FisType type;
			uint8_t pmport : 4, : 4; // port multiplier port
			uint16_t rsv;
			uint32_t data[1]; // payload 1 ~ N
		} __packed data_fis_t;

		typedef volatile struct __dma_setup_fis_t
		{
			FisType type;
			uint8_t pmport : 4, : 1; // port multiplier port
			uint8_t d : 1;           // data transfer direction
			uint8_t i : 1;           // interrupt bit
			uint8_t a : 1;           // auto activate
			uint16_t rsv1;
			uint64_t DMABufferId;
			uint32_t rsv2;
			uint32_t DMABufferOffset;
			uint32_t DMATransferCount;
			uint32_t rsv3;
		} __packed dma_setup_fis_t;

		typedef volatile struct __pio_setup_fis_t
		{
			FisType type;
			uint8_t pmport : 4, : 1; // port multiplier port
			uint8_t d : 1;           // data transfer direction
			uint8_t i : 1, : 1;      // interrupt bit
			uint8_t status;
			uint8_t error;
			uint8_t lba0;
			uint8_t lba1;
			uint8_t lba2;
			uint8_t device;
			uint8_t lba3;
			uint8_t lba4;
			uint8_t lba5;
			uint8_t rsv1;
			uint16_t count;
			uint8_t rsv2;
			uint8_t e_status; // new status value
			uint16_t tc;      // transfer count
			uint16_t rsv3;
		} __packed pio_setup_fis_t;

		typedef volatile struct __received_fis_t
		{
			dma_setup_fis_t dsfis;
			uint32_t rsv1;

			pio_setup_fis_t psfis;
			uint32_t rsv2[3];

			d2h_register_fis_t rfis;
			uint32_t rsv3;

			uint32_t ufis[16]; // undefined FIS

			uint32_t rsv4[24];
		} __packed received_fis_t;

		typedef volatile struct __ata_identify_block_t
		{
			struct
			{
				uint16_t rsv1 : 2;
				uint16_t incomplete_response : 1;
				uint16_t rsv2 : 3;
				uint16_t fixed_device : 1;
				uint16_t removable_media : 1;
				uint16_t rsv3 : 7;
				uint16_t device_type : 1;
			} __packed general_configuration;
			uint16_t rsv1;
			uint16_t specific_configuration;
			uint16_t rsv2[7];
			uint8_t serial_num[20];
			uint16_t rsv3[3];
			uint8_t firmware_rev[8];
			uint8_t model_num[40];
			uint8_t max_block_transfer;
			uint8_t rsv4;
			uint16_t trusted_computing_supported : 1, : 15;
			struct
			{
				uint32_t long_physical_sector_alignment : 2, : 6;
				uint32_t dma_supported : 1;
				uint32_t lba_supported : 1;
				uint32_t IORDY_disable : 1;
				uint32_t IORDY_supported : 1, : 1;
				uint32_t stand_by_timer_supported : 1, : 18;
			} __packed capabilities;
			uint32_t rsv5;
			uint8_t translation_fields_valid : 3, : 5;
			uint8_t free_fall_control_sensitivity;
			uint16_t rsv6[5];
			uint8_t current_multi_sector_setting;
			uint8_t multi_sector_setting_valid : 1, : 3;
			uint8_t sanitize_feature_set_supported : 1;
			uint8_t crypto_scramble_ext_supported : 1;
			uint8_t overwrite_ext_supported : 1;
			uint8_t block_erase_ext_supported : 1;
			uint32_t user_addressable_sectors;
			uint16_t rsv7;
			uint8_t multiword_dma_supported;
			uint8_t multiword_dma_activated;
			uint8_t advanced_pio_modes_supported;
			uint8_t rsv8;
			uint16_t minimum_mw_dma_Xfer_time;
			uint16_t recommended_mw_dma_Xfer_time;
			uint16_t minimum_pio_Xfer_time;
			uint16_t minimum_pio_Xfer_time_iordy;
			struct
			{
				uint16_t zoned_capabilities : 2;
				uint16_t non_volatile_write_cache : 1;
				uint16_t extended_user_addressable_sectors : 1;
				uint16_t device_encrypts_all_user_data : 1;
				uint16_t trimmed_ranges_return_zero : 1;
				uint16_t optional_28bit_commands : 1, : 1;
				uint16_t download_microcode : 1, : 1;
				uint16_t write_buffer_dma : 1;
				uint16_t read_buffer_dma : 1, : 1;
				uint16_t long_physical_alignment_error_reporting : 1;
				uint16_t deterministic_data_in_trimmed : 1, : 1;
			} __packed additional_supported;
			uint16_t rsv9[5];
			uint16_t queue_depth : 5, : 11;
			struct
			{
				uint16_t rsv : 1;
				uint16_t gen1_speed : 1;
				uint16_t gen2_speed : 1;
				uint16_t gen3_speed : 1, : 4;
				uint16_t ncq : 1;
				uint16_t host_initiated_power_management_request : 1;
				uint16_t phy_events : 1;
				uint16_t ncq_unload : 1;
				uint16_t ncq_priority : 1;
				uint16_t host_automatic_partial_to_slumber : 1;
				uint16_t device_automatic_partial_to_slumber : 1;
				uint16_t read_log_dma : 1, : 1;
				uint16_t current_speed : 3;
				uint16_t ncq_streaming : 1;
				uint16_t ncq_queue_management : 1;
				uint16_t ncq_receive_send : 1, : 9;
			} __packed sata_capabilities;
			struct
			{
				uint16_t rsv : 1;
				uint16_t non_zero_offsets : 1;
				uint16_t dma_setup_auto_activate : 1;
				uint16_t device_initiated_power_management : 1;
				uint16_t in_order_data : 1;
				uint16_t hardware_feature_control : 1;
				uint16_t software_settings_preservation : 1;
				uint16_t automatic_partial_to_slumber : 1, : 8;
			} __packed sata_features_supported;
			struct
			{
				uint16_t rsv : 1;
				uint16_t non_zero_offsets : 1;
				uint16_t dma_setup_auto_activate : 1;
				uint16_t device_initiated_power_management : 1;
				uint16_t in_order_data : 1;
				uint16_t hardware_feature_control : 1;
				uint16_t software_settings_preservation : 1;
				uint16_t automatic_partial_to_slumber : 1, : 8;
			} __packed sata_features_enabled;
			uint16_t major_revision;
			uint16_t minor_revision;
			struct
			{
				uint16_t smart : 1;
				uint16_t security : 1, : 1;
				uint16_t power_management : 1;
				uint16_t packet : 1;
				uint16_t write_cache : 1;
				uint16_t look_ahead : 1, : 2;
				uint16_t device_reset : 1, : 2;
				uint16_t write_buffer : 1;
				uint16_t read_buffer : 1;
				uint16_t nop : 1, : 1;
				uint16_t download_microcode : 1, : 2;
				uint16_t advanced_power_management : 1, : 1;
				uint16_t power_up_spin_up : 1;
				uint16_t set_features_after_power_up : 1, : 3;
				uint16_t lba48 : 1, : 1;
				uint16_t flush_cache : 1;
				uint16_t flush_cache_ext : 1, : 2;
				uint16_t smart_error_log : 1;
				uint16_t smart_self_test : 1, : 2;
				uint16_t streaming : 1;
				uint16_t gpl : 1;
				uint16_t write_dma_fua : 1, : 1;
				uint16_t word_wide_name : 1, : 4;
				uint16_t idle_immediate_with_unload : 1, : 2;
			} __packed commands_supported;
			struct
			{
				uint16_t smart : 1;
				uint16_t security : 1, : 1;
				uint16_t power_management : 1;
				uint16_t packet : 1;
				uint16_t write_cache : 1;
				uint16_t look_ahead : 1, : 2;
				uint16_t device_reset : 1, : 2;
				uint16_t write_buffer : 1;
				uint16_t read_buffer : 1;
				uint16_t nop : 1, : 1;
				uint16_t download_microcode : 1, : 2;
				uint16_t advanced_power_management : 1, : 1;
				uint16_t power_up_spin_up : 1;
				uint16_t set_features_after_power_up : 1, : 3;
				uint16_t lba48 : 1, : 1;
				uint16_t flush_cache : 1;
				uint16_t flush_cache_ext : 1, : 2;
				uint16_t smart_error_log : 1;
				uint16_t smart_self_test : 1, : 2;
				uint16_t streaming : 1;
				uint16_t gpl : 1;
				uint16_t write_dma_fua : 1, : 1;
				uint16_t word_wide_name : 1, : 4;
				uint16_t idle_immediate_with_unload : 1, : 2;
			} __packed commands_active;
			uint8_t ultra_dma_supported;
			uint8_t ultra_dma_active;
			struct
			{
				uint16_t time_required : 15;
				uint16_t extended_time : 1;
			} __packed normal_security_erase_unit;
			struct
			{
				uint16_t extended_time : 15;
				uint16_t ExtendedTimeReported : 1;
			} __packed enhanced_security_erase_unit;
			uint16_t current_apm_level : 8, : 8;
			uint16_t master_password_id;
			uint16_t hardware_reset_result;
			uint16_t rsv10;
			uint16_t streaming_min_request_size;
			uint16_t streaming_Xfer_time_dma;
			uint16_t streaming_access_latency_dma_pio;
			uint32_t streaming_performance_granularity;
			uint64_t user_addressable_logical_sectors;
			uint16_t streaming_Xfer_time_pio;
			uint16_t data_set_management_cap;
			struct
			{
				uint16_t logical_per_physical_sectors : 4, : 8;
				uint16_t logical_sector_longer_than_512 : 1;
				uint16_t multiple_logical_per_physical : 1, : 2;
			} __packed physical_logical_sector_size;
			uint16_t inter_seek_delay;
			uint16_t world_wide_name[4];
			uint16_t rsv11[5];
			uint32_t words_per_logical_sector;
			struct
			{
				uint16_t rsv1 : 1;
				uint16_t write_read_verify : 1;
				uint16_t write_uncorrectable_ext : 1;
				uint16_t read_write_log_dma : 1;
				uint16_t download_microcode_mode_3 : 1;
				uint16_t free_fall_control : 1;
				uint16_t sense_data_reporting : 1;
				uint16_t extended_power_conditions : 1;
				uint16_t accessible_max_address_config : 1;
				uint16_t dsn : 1, : 6;
			} __packed commands_supported_ext;
			struct
			{
				uint16_t rsv1 : 1;
				uint16_t write_read_verify : 1;
				uint16_t write_uncorrectable_ext : 1;
				uint16_t read_write_log_dma : 1;
				uint16_t download_microcode_mode_3 : 1;
				uint16_t free_fall_control : 1;
				uint16_t sense_data_reporting : 1;
				uint16_t extended_power_conditions : 1;
				uint16_t accessible_max_address_config : 1;
				uint16_t dsn : 1, : 6;
			} __packed commands_active_ext;
			uint16_t rsv12[7];
			struct
			{
				uint16_t security_supported : 1;
				uint16_t security_enabled : 1;
				uint16_t security_locked : 1;
				uint16_t security_frozen : 1;
				uint16_t security_count_expired : 1;
				uint16_t enhanced_security_erase : 1, : 2;
				uint16_t master_password_cap : 1, : 7;
			} __packed security_status;
			uint16_t rsv13[39];
			uint16_t nominal_form_factor : 4, : 12;
			struct
			{
				uint16_t supports_trim : 1, : 15;
			} __packed data_set_management_feature;
			uint16_t additional_product_id[4];
			uint16_t rsv14[2];
			uint16_t current_media_serial_number[30];
			struct
			{
				uint16_t supported : 1, : 1;
				uint16_t write_same_supported : 1;
				uint16_t error_recovery_control_supported : 1;
				uint16_t feature_control_supported : 1;
				uint16_t data_tables_supported : 1, : 10;
			} __packed sct_command_transport;
			uint16_t rsv15[2];
			uint16_t alignment_of_logical_in_physical : 14, : 2;
			uint32_t write_read_verify_sector_mode_3_count;
			uint32_t write_read_verify_sector_mode_2_count;
			uint16_t rsv16[3];
			uint16_t nominal_media_rotation_rate;
			uint16_t rsv17[2];
			uint16_t write_read_verify_current_mode : 8, : 8;
			uint16_t rsv18;
			struct
			{
				uint16_t major_version : 12;
				uint16_t type : 4;
			} __packed transport_major_version;
			uint16_t transport_minor_version;
			uint16_t rsv19[6];
			uint64_t extended_user_addressable_sectors;
			uint16_t min_blocks_per_download_microcode;
			uint16_t max_blocks_per_download_microcode;
			uint16_t rsv20[19];
			struct
			{
				uint16_t checksum_validity_indicator : 8;
				uint16_t checksum  : 8;
			} __packed integrity;
		} __packed ata_identify_block_t;
	}
}
