#include "StandardCSIDataProcessing.h"
#include <sstream>
#include <string>



CSIStandardDataProcessingClass::CSIStandardDataProcessingClass() {
    data.timestamp = 0;
    data.csi_length = 0;
    data.channel_number = 0;
    data.buffer_length = 0;
    data.payload_length = 0;
    data.physical_error = 0;
    data.noise_level = 0;
    data.transmission_rate = 0;
    data.channel_bandwidth = 0;
    data.number_of_tones = 0;
    data.receiver_antennas = 0;
    data.transmitter_antennas = 0;
    data.received_signal_strength = 0;
    data.rssi_antenna_0 = 0;
    data.rssi_antenna_1 = 0;
    data.rssi_antenna_2 = 0;

    // initialize the csi matrix
    for (auto & i : data.csi_matrix) {
        for (auto & j : i) {
            for (auto & k : j) {
                k.real = 0;
                k.imag = 0;
            }
        }
    }
}


std::string CSIStandardDataProcessingClass::toCSVString(const std::string& separator) const {

    // use stringstream to construct the string
    std::stringstream ss;

    // write the data to the stringstream
    ss << std::to_string((uint64_t)data.timestamp) << separator;
    ss << std::to_string((int32_t)data.csi_length) << separator;
    ss << std::to_string((int32_t)data.channel_number) << separator;
    ss << std::to_string((int32_t)data.buffer_length) << separator;
    ss << std::to_string((int32_t)data.payload_length) << separator;
    ss << std::to_string((int32_t)data.physical_error) << separator;
    ss << std::to_string((int32_t)data.noise_level) << separator;
    ss << std::to_string((int32_t)data.transmission_rate) << separator;
    ss << std::to_string((int32_t)data.channel_bandwidth) << separator;
    ss << std::to_string((int32_t)data.number_of_tones) << separator;
    ss << std::to_string((int32_t)data.receiver_antennas) << separator;
    ss << std::to_string((int32_t)data.transmitter_antennas) << separator;
    ss << std::to_string((int32_t)data.received_signal_strength) << separator;
    ss << std::to_string((int32_t)data.rssi_antenna_0) << separator;
    ss << std::to_string((int32_t)data.rssi_antenna_1) << separator;
    ss << std::to_string((int32_t)data.rssi_antenna_2) << separator;

    // convert the csi matrix to json string
    ss << "[";
    for (int k = 0; k < data.number_of_tones; k++) {
        ss << "[";
        for (int nc_idx = 0; nc_idx < data.transmitter_antennas; nc_idx++) {
            ss << "[";
            for (int nr_idx = 0; nr_idx < data.receiver_antennas; nr_idx++) {
                ss << "["
                << std::to_string((int32_t)data.csi_matrix[nr_idx][nc_idx][k].real)
                << ","
                << std::to_string(data.csi_matrix[nr_idx][nc_idx][k].imag)
                << "]";

                if (nr_idx < data.receiver_antennas - 1) {
                    ss << ",";
                }
            }
            ss << "]";

            if (nc_idx < data.transmitter_antennas - 1) {
                ss << ",";
            }
        }
        ss << "]";

        if (k < data.number_of_tones - 1) {
            ss << ",";
        }
    }
    ss << "]";

    // return the string
    return ss.str();
}


std::string CSIStandardDataProcessingClass::toCSVHeader(const std::string& separator) {

    // use stringstream to construct the string
    std::stringstream ss;

    // write the data to the stringstream
    ss << "timestamp" << separator;
    ss << "csi_length" << separator;
    ss << "channel_number" << separator;
    ss << "buffer_length" << separator;
    ss << "payload_length" << separator;
    ss << "physical_error" << separator;
    ss << "noise_level" << separator;
    ss << "transmission_rate" << separator;
    ss << "channel_bandwidth" << separator;
    ss << "number_of_tones" << separator;
    ss << "receiver_antennas" << separator;
    ss << "transmitter_antennas" << separator;
    ss << "received_signal_strength" << separator;
    ss << "rssi_antenna_0" << separator;
    ss << "rssi_antenna_1" << separator;
    ss << "rssi_antenna_2" << separator;
    ss << "csi_matrix";

    // return the string
    return ss.str();
}

#include "../csi/OpenWRT_v1.h"

void CSIStandardDataProcessingClass::updateWithOpenWRTv1(uint8_t *buffer, size_t buffer_size) {

    // Get the CSI meta information from buffer
    auto meta_info = get_csi_metadata(buffer, buffer_size);

    // Update the meta information
    data.timestamp = meta_info->timestamp;
    data.csi_length = meta_info->csi_len;
    data.channel_number = meta_info->channel;
    data.buffer_length = meta_info->buf_len;
    data.payload_length = meta_info->payload_len;
    data.physical_error = meta_info->physical_err;
    data.noise_level = meta_info->noise;
    data.transmission_rate = meta_info->rate;
    data.channel_bandwidth = meta_info->bandwidth;
    data.number_of_tones = meta_info->num_tones;
    data.receiver_antennas = meta_info->nr;
    data.transmitter_antennas = meta_info->nc;
    data.received_signal_strength = meta_info->rssi;
    data.rssi_antenna_0 = meta_info->rssi_0;
    data.rssi_antenna_1 = meta_info->rssi_1;
    data.rssi_antenna_2 = meta_info->rssi_2;

    //////// Update the CSI matrix ////////

    // Convert the CSI matrix to an int32_t array with one dimension
    auto* csi_matrix_ptr = reinterpret_cast<int32_t*>(data.csi_matrix);

    // Use the function decode_openwrt_v1 to get the CSI matrix
    decode_openwrt_v1(buffer + CSI_META_LEN + 2, meta_info->csi_len, csi_matrix_ptr);
}

#include <cstring>

void CSIStandardDataProcessingClass::updateWithCSIDataStruct(CSIStandardData* csi_data) {
    
        // Update the meta information
        data.timestamp = csi_data->timestamp;
        data.csi_length = csi_data->csi_length;
        data.channel_number = csi_data->channel_number;
        data.buffer_length = csi_data->buffer_length;
        data.payload_length = csi_data->payload_length;
        data.physical_error = csi_data->physical_error;
        data.noise_level = csi_data->noise_level;
        data.transmission_rate = csi_data->transmission_rate;
        data.channel_bandwidth = csi_data->channel_bandwidth;
        data.number_of_tones = csi_data->number_of_tones;
        data.receiver_antennas = csi_data->receiver_antennas;
        data.transmitter_antennas = csi_data->transmitter_antennas;
        data.received_signal_strength = csi_data->received_signal_strength;
        data.rssi_antenna_0 = csi_data->rssi_antenna_0;
        data.rssi_antenna_1 = csi_data->rssi_antenna_1;
        data.rssi_antenna_2 = csi_data->rssi_antenna_2;
    
        // Convert the CSI matrix to an int32_t array with one dimension
        auto* csi_matrix_ptr = reinterpret_cast<int32_t*>(data.csi_matrix);
    
        // memcpy the data
        memcpy(csi_matrix_ptr, csi_data->csi_matrix, sizeof(data.csi_matrix));
}