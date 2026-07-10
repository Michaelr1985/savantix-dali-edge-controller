#include "dali/storage/nvs_metadata_store.h"

#include "nvs.h"
#include "nvs_flash.h"

namespace dali::storage {
namespace {
constexpr const char* kNamespace = "dali_meta";
}

esp_err_t NvsMetadataStore::init() noexcept {
    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        result = nvs_flash_erase();
        if (result != ESP_OK) return result;
        result = nvs_flash_init();
    }
    return result;
}

esp_err_t NvsMetadataStore::saveUint32(const char* key,
                                       std::uint32_t value) noexcept {
    nvs_handle_t handle = 0;
    esp_err_t result = nvs_open(kNamespace, NVS_READWRITE, &handle);
    if (result != ESP_OK) return result;
    result = nvs_set_u32(handle, key, value);
    if (result == ESP_OK) result = nvs_commit(handle);
    nvs_close(handle);
    return result;
}

esp_err_t NvsMetadataStore::loadUint32(const char* key,
                                       std::uint32_t& value) noexcept {
    nvs_handle_t handle = 0;
    esp_err_t result = nvs_open(kNamespace, NVS_READONLY, &handle);
    if (result != ESP_OK) return result;
    result = nvs_get_u32(handle, key, &value);
    nvs_close(handle);
    return result;
}

}  // namespace dali::storage
