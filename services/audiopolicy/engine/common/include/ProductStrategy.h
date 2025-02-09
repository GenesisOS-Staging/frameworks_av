/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "VolumeGroup.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <HandleGenerator.h>
#include <media/VolumeGroupAttributes.h>
#include <media/AudioContainers.h>
#include <media/AudioDeviceTypeAddr.h>
#include <media/AudioPolicy.h>
#include <system/audio.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/String8.h>

namespace android {

/**
 * @brief The ProductStrategy class describes for each product_strategy_t identifier the
 * associated audio attributes, the device types to use, the device address to use.
 * The identifier is voluntarily not strongly typed in order to be extensible by OEM.
 */
class ProductStrategy : public virtual RefBase, private HandleGenerator<uint32_t>
{
private:
    using VolumeGroupAttributesVector = std::vector<VolumeGroupAttributes>;

public:
    ProductStrategy(const std::string &name, int id = PRODUCT_STRATEGY_NONE);

    void addAttributes(const VolumeGroupAttributes &volumeGroupAttributes);

    std::vector<android::VolumeGroupAttributes> listVolumeGroupAttributes() const;

    std::string getName() const { return mName; }
    AttributesVector getAudioAttributes() const;
    product_strategy_t getId() const { return mId; }
    StreamTypeVector getSupportedStreams() const;
    VolumeGroupAttributesVector getVolumeGroupAttributes() const { return mAttributesVector; }

    /**
     * @brief matches checks if the given audio attributes shall follow the strategy.
     *        Order of the attributes within a strategy matters.
     *        If only the usage is available, the check is performed on the usages of the given
     *        attributes, otherwise all fields must match.
     * @param attributes to consider
     * @return matching score, negative value if no match.
     */
    int matchesScore(const audio_attributes_t attributes) const;

    bool supportStreamType(const audio_stream_type_t &streamType) const;

    void setDeviceAddress(const std::string &address)
    {
        mDeviceAddress = address;
    }

    std::string getDeviceAddress() const { return mDeviceAddress; }

    void setDeviceTypes(const DeviceTypeSet& devices)
    {
        mApplicableDevices = devices;
    }

    DeviceTypeSet getDeviceTypes() const { return mApplicableDevices; }

    audio_attributes_t getAttributesForStreamType(audio_stream_type_t stream) const;

    volume_group_t getVolumeGroupForStreamType(audio_stream_type_t stream) const;

    volume_group_t getDefaultVolumeGroup() const;

    bool isDefault() const;

    void dump(String8 *dst, int spaces = 0) const;

private:
    std::string mName;

    VolumeGroupAttributesVector mAttributesVector;

    product_strategy_t mId;

    std::string mDeviceAddress; /**< Device address applicable for this strategy, maybe empty */

    /**
     * Applicable device(s) type mask for this strategy.
     */
    DeviceTypeSet mApplicableDevices;
};

class ProductStrategyMap : public std::map<product_strategy_t, sp<ProductStrategy> >
{
public:
    /**
     * @brief initialize: set default product strategy in cache.
     */
    void initialize();
    /**
     * @brief getProductStrategyForAttribute. The order of the vector is dimensionning.
     * @param attr
     * @return applicable product strategy for the given attribute, default if none applicable.
     */
    product_strategy_t getProductStrategyForAttributes(
            const audio_attributes_t &attr, bool fallbackOnDefault = true) const;

    product_strategy_t getProductStrategyForStream(audio_stream_type_t stream) const;

    audio_attributes_t getAttributesForStreamType(audio_stream_type_t stream) const;

    audio_stream_type_t getStreamTypeForAttributes(const audio_attributes_t &attr) const;

    /**
     * @brief getAttributesForProductStrategy can be called from
     *        AudioManager: in this case, the product strategy IS the former routing strategy
     *        CarAudioManager: in this case, the product strategy IS the car usage
     *                      [getAudioAttributesForCarUsage]
     *        OemExtension: in this case, the product strategy IS the Oem usage
     *
     * @param strategy
     * @return audio attributes (or at least one of the attributes) following the given strategy.
     */
    audio_attributes_t getAttributesForProductStrategy(product_strategy_t strategy) const;

    DeviceTypeSet getDeviceTypesForProductStrategy(product_strategy_t strategy) const;

    std::string getDeviceAddressForProductStrategy(product_strategy_t strategy) const;

    volume_group_t getVolumeGroupForAttributes(
            const audio_attributes_t &attr, bool fallbackOnDefault = true) const;

    volume_group_t getVolumeGroupForStreamType(
            audio_stream_type_t stream, bool fallbackOnDefault = true) const;

    volume_group_t getDefaultVolumeGroup() const;

    product_strategy_t getDefault() const;

    void dump(String8 *dst, int spaces = 0) const;

private:
    VolumeGroupAttributes getVolumeGroupAttributesForAttributes(
            const audio_attributes_t &attr, bool fallbackOnDefault = true) const;

    product_strategy_t mDefaultStrategy = PRODUCT_STRATEGY_NONE;
};

using ProductStrategyDevicesRoleMap =
        std::map<std::pair<product_strategy_t, device_role_t>, AudioDeviceTypeAddrVector>;

void dumpProductStrategyDevicesRoleMap(
        const ProductStrategyDevicesRoleMap& productStrategyDeviceRoleMap,
        String8 *dst,
        int spaces);

} // namespace android
