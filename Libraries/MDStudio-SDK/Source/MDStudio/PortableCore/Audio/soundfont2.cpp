//
//  soundfont2.cpp
//  MDStudio
//
//  Created by Daniel Cliche on 2018-02-24.
//  Copyright © 2018-2020 Daniel Cliche. All rights reserved.
//

#include "soundfont2.h"

#include <cstring>

using namespace MDStudio;

// ---------------------------------------------------------------------------------------------------------------------
SoundFont2::Types SoundFont2::getType(UInt8 id[4]) {
    static const std::vector<std::vector<UInt8>> types = {
        {'R', 'I', 'F', 'F'}, {'L', 'I', 'S', 'T'},

        {'s', 'f', 'b', 'k'}, {'I', 'N', 'F', 'O'}, {'s', 'd', 't', 'a'}, {'p', 'd', 't', 'a'}, {'i', 'f', 'i', 'l'},
        {'i', 's', 'n', 'g'}, {'i', 'r', 'o', 'm'}, {'i', 'v', 'e', 'r'}, {'I', 'N', 'A', 'M'}, {'I', 'C', 'R', 'D'},
        {'I', 'E', 'N', 'G'}, {'I', 'P', 'R', 'D'}, {'I', 'C', 'O', 'P'}, {'I', 'C', 'M', 'T'}, {'I', 'S', 'F', 'T'},
        {'s', 'm', 'p', 'l'}, {'p', 'h', 'd', 'r'}, {'p', 'b', 'a', 'g'}, {'p', 'm', 'o', 'd'}, {'p', 'g', 'e', 'n'},
        {'i', 'n', 's', 't'}, {'i', 'b', 'a', 'g'}, {'i', 'm', 'o', 'd'}, {'i', 'g', 'e', 'n'}, {'s', 'h', 'd', 'r'}};

    int i = 0;
    for (auto t : types) {
        if (id[0] == t[0] && id[1] == t[1] && id[2] == t[2] && id[3] == t[3]) return (Types)i;
        ++i;
    }

    return Unknown;
}

// ---------------------------------------------------------------------------------------------------------------------
void SoundFont2::skipChunk(RIFFChunk& riffChunk, std::ifstream& ifs) { ifs.seekg(riffChunk.size, std::ios_base::cur); }

// ---------------------------------------------------------------------------------------------------------------------
bool SoundFont2::readTop(std::ifstream& ifs) {
    UInt8 id[4];
    ifs.read((char*)id, sizeof(id));
    Types type = getType(id);

    if (type != SFBK) {
        std::cout << "Expected SFBK\n";
        return false;
    }

    return readSFBK(ifs);
}

// ---------------------------------------------------------------------------------------------------------------------
bool SoundFont2::readChunk(std::vector<char>* data, Types expectedType, std::ifstream& ifs) {
    RIFFChunk riffChunk;
    ifs.read((char*)&riffChunk, sizeof(RIFFChunk));
    Types type = getType(riffChunk.id);

    if (type != expectedType) {
        std::cout << "Unexpected chunk type\n";
        return false;
    }

    data->resize(riffChunk.size);
    ifs.read(&(*data)[0], riffChunk.size);

    // Skip the padding
    // std::streampos filePos = ifs.tellg();
    // int paddingSize = filePos % sizeof(UInt32);
    // ifs.seekg(paddingSize, std::ios_base::cur);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool SoundFont2::readChunk(char* data, size_t size, Types expectedType, std::ifstream& ifs) {
    RIFFChunk riffChunk;
    ifs.read((char*)&riffChunk, sizeof(RIFFChunk));
    Types type = getType(riffChunk.id);

    if (type != expectedType) {
        std::cout << "Unexpected chunk type\n";
        return false;
    }

    if (size != riffChunk.size) {
        std::cout << "Size mismatch\n";
        return false;
    }
    ifs.read(data, size);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool SoundFont2::readChunk(std::string* s, Types expectedType, std::ifstream& ifs) {
    RIFFChunk riffChunk;
    ifs.read((char*)&riffChunk, sizeof(RIFFChunk));
    Types type = getType(riffChunk.id);

    if (type != expectedType) {
        std::cout << "Unexpected chunk type\n";
        return false;
    }

    if (riffChunk.size < 1) {
        std::cout << "Unexpected chunk size\n";
        return false;
    }

    s->resize(riffChunk.size - 1);
    ifs.read(&(*s)[0], riffChunk.size - 1);

    // Skip zero
    ifs.seekg(1, std::ios_base::cur);

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
template <class T>
bool SoundFont2::readChunk(std::vector<T>* v, Types expectedType, bool isTerminalSkipped, std::ifstream& ifs) {
    RIFFChunk riffChunk;
    ifs.read((char*)&riffChunk, sizeof(RIFFChunk));
    Types type = getType(riffChunk.id);

    if (type != expectedType) {
        std::cout << "Unexpected chunk type\n";
        return false;
    }

    if ((riffChunk.size == 0) || (riffChunk.size % sizeof(T))) {
        std::cout << "Unexpected chunk size\n";
        return false;
    }

    UInt32 nbElementsToRead = riffChunk.size / sizeof(T);

    if (isTerminalSkipped) {
        if (nbElementsToRead < 1) {
            std::cout << "Terminal element not found\n";
            return false;
        }
        --nbElementsToRead;
    }

    v->resize(nbElementsToRead);
    ifs.read((char*)&(*v)[0], nbElementsToRead * sizeof(T));

    if (isTerminalSkipped) {
        // Skip the terminal element
        ifs.seekg(sizeof(T), std::ios_base::cur);
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool SoundFont2::readSFBK(std::ifstream& ifs) {
    // Read the list chunk

    RIFFChunk riffChunk;
    UInt8 id[4];

    // Info list
    ifs.read((char*)&riffChunk, sizeof(RIFFChunk));
    Types type = getType(riffChunk.id);

    if (type != LIST) {
        std::cout << "Expected list for INFO\n";
        return false;
    }

    // Info
    ifs.read((char*)id, sizeof(id));
    type = getType(id);

    if (type != INFO) {
        std::cout << "Expected INFO\n";
        return false;
    }

    // ifil chunk
    if (!readChunk((char*)&_fileVersion, sizeof(_fileVersion), IFIL, ifs)) return false;

    // isng chunk
    if (!readChunk(&_soundEngine, ISNG, ifs)) return false;

    // inam chunk
    std::vector<char> inamData;
    if (!readChunk(&_name, INAM, ifs)) return false;

    // icrd chunk
    if (!readChunk(&_creationDate, ICRD, ifs)) return false;

    // ieng chunk
    if (!readChunk(&_engineers, IENG, ifs)) return false;

    // iprd chunk
    if (!readChunk(&_product, IPRD, ifs)) return false;

    // icop chunk
    if (!readChunk(&_copyright, ICOP, ifs)) return false;

    // icmt chunk
    if (!readChunk(&_comments, ICMT, ifs)) return false;

    // isft chunk
    if (!readChunk(&_editingSoftware, ISFT, ifs)) return false;

    // SDTA list
    ifs.read((char*)&riffChunk, sizeof(RIFFChunk));
    type = getType(riffChunk.id);

    if (type != LIST) {
        std::cout << "Expected list for SDTA\n";
        return false;
    }

    ifs.read((char*)id, sizeof(id));
    type = getType(id);

    if (type != SDTA) {
        std::cout << "Expected SDTA\n";
        return false;
    }

    ifs.read((char*)&riffChunk, sizeof(RIFFChunk));
    type = getType(riffChunk.id);

    if (type != SMPL) {
        std::cout << "Expected SMPL\n";
        return false;
    }

    _samplesOffset = (UInt32)ifs.tellg();

    skipChunk(riffChunk, ifs);

    // PDTA list
    ifs.read((char*)&riffChunk, sizeof(RIFFChunk));
    type = getType(riffChunk.id);

    if (type != LIST) {
        std::cout << "Expected list for PDTA\n";
        return false;
    }

    // PDTA
    ifs.read((char*)id, sizeof(id));
    type = getType(id);

    if (type != PDTA) {
        std::cout << "Expected PDTA\n";
        return false;
    }

    // phdr chunk
    if (!readChunk(&_presetHeaders, PHDR, false /*true*/, ifs)) return false;

    // pbag chunk
    if (!readChunk(&_presetBags, PBAG, false, ifs)) return false;

    // pmod chunk
    if (!readChunk(&_modLists, PMOD, false /*true*/, ifs)) return false;

    // pgen chunk
    if (!readChunk(&_genLists, PGEN, false /*true*/, ifs)) return false;

    // inst chunk
    if (!readChunk(&_instruments, INST, false /*true*/, ifs)) return false;

    // ibag chunk
    if (!readChunk(&_instBags, IBAG, false, ifs)) return false;

    // imod chunk
    if (!readChunk(&_instModLists, IMOD, false, ifs)) return false;

    // igen chunk
    if (!readChunk(&_instGenLists, IGEN, false, ifs)) return false;

    // shdr chunk
    if (!readChunk(&_samples, SHDR, false, ifs)) return false;

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
bool SoundFont2::load(std::string path) {
    std::ifstream ifs(path.c_str(), std::ios::in | std::ios::binary);
    if (!ifs.is_open()) return false;

    RIFFChunk riffChunk;
    ifs.read((char*)&riffChunk, sizeof(RIFFChunk));

    /*
     std::vector<char> data;
     data.resize(riffChunk.size);
     ifs.read(&data[0], riffChunk.size);

     // Skip the padding
     std::streampos filePos = ifs.tellg();
     int paddingSize = filePos % sizeof(UInt32);
     ifs.seekg(paddingSize, std::ios_base::cur);
     */

    Types chunkType = getType(riffChunk.id);

    if (chunkType != RIFF) {
        std::cout << "Not a RIFF file format\n";
        ifs.close();
        return false;
    }

    if (!readTop(ifs)) {
        std::cout << "Unable to read the file\n";
        ifs.close();
        return false;
    }

    ifs.close();

    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<SoundFont2PresetHeader> SoundFont2::presetHeaders() {
    std::vector<SoundFont2PresetHeader> ret;

    if (!_presetHeaders.empty()) {
        for (auto it = _presetHeaders.begin(); it < _presetHeaders.end() - 1; ++it) ret.push_back(*it);
    }

    return ret;
}

// ---------------------------------------------------------------------------------------------------------------------
void SoundFont2::processGenOperator(SoundFont2Data* datum, SoundFont2Generator oper, SoundFont2GenAmount amount) {
    switch (oper) {
        case 2:
            // Start loop offset
            datum->startLoopOffset = amount.shAmount;
            datum->isStartLoopOffsetSet = true;
            break;
        case 3:
            // End loop offset
            datum->endLoopOffset = amount.shAmount;
            datum->isEndLoopOffsetSet = true;
            break;
        case 8:
            // Initial filter Fc
            datum->initialFilterFc = amount.shAmount;
            datum->isInitialFilterFcSet = true;
            break;
        case 9:
            // Initial filter Q
            datum->initialFilterQ = amount.shAmount;
            datum->isInitialFilterQSet = true;
            break;
        case 17:
            // Pan
            datum->panEffectsSend = amount.shAmount;
            datum->isPanEffectsSendSet = true;
            break;
        case 34:
            // Attack volume env
            datum->attackVolEnv = amount.shAmount;
            datum->isAttackVolEnvSet = true;
            break;
        case 35:
            // Hold volume env
            datum->holdVolEnv = amount.shAmount;
            datum->isHoldVolEnvSet = true;
            break;
        case 36:
            // Decay volume env
            datum->decayVolEnv = amount.shAmount;
            datum->isDecayVolEnvSet = true;
            break;
        case 37:
            // Sustain volume env
            datum->sustainVolEnv = amount.shAmount;
            datum->isSustainVolEnvSet = true;
            break;
        case 38:
            // Release volume env
            datum->releaseVolEnv = amount.shAmount;
            datum->isReleaseVolEnvSet = true;
            break;
        case 43:
            // Key range
            datum->minPitch = amount.ranges.lo;
            datum->maxPitch = amount.ranges.hi;
            datum->isKeyRangeSet = true;
            break;
        case 44:
            // Velocity range
            datum->minVelocity = amount.ranges.lo;
            datum->maxVelocity = amount.ranges.hi;
            datum->isVelocityRangeSet = true;
            break;
        case 51:
            // Coarse tune
            datum->coarseTune = amount.shAmount;
            datum->isCoarseTuneSet = true;
            break;
        case 52:
            // Fine tune
            datum->fineTune = amount.shAmount;
            datum->isFineTuneSet = true;
            break;
        case 53:
            // Sample id
            break;
        case 54:
            // Sample modes
            // 0: no loop
            // 1: loop continuous
            // 2: unused - no loop
            // 3: loop key depressed then proceeds to play the remainder of the sample
            datum->sampleModes = amount.shAmount;
            datum->isSampleModesSet = true;
            break;
        case 58:
            // Overriding root key
            datum->rootKey = amount.shAmount;
            datum->isRootKeySet = true;
            break;
        default:
            // std::cout << "Unhandled generator " << (int)oper << "\n";
            break;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void SoundFont2::combineDatum(SoundFont2Data* destDatum, SoundFont2Data* srcDatum, bool isAdded) {
    if (srcDatum->isInitialFilterFcSet) {
        if (isAdded) {
            destDatum->initialFilterFc += srcDatum->initialFilterFc;
        } else {
            destDatum->initialFilterFc = srcDatum->initialFilterFc;
        }
        destDatum->isInitialFilterFcSet = true;
    }
    if (srcDatum->isInitialFilterQSet) {
        if (isAdded) {
            destDatum->initialFilterQ += srcDatum->initialFilterQ;
        } else {
            destDatum->initialFilterQ = srcDatum->initialFilterQ;
        }
        destDatum->isInitialFilterQSet = true;
    }
    if (srcDatum->isPanEffectsSendSet) {
        if (isAdded) {
            destDatum->panEffectsSend += srcDatum->panEffectsSend;
        } else {
            destDatum->panEffectsSend = srcDatum->panEffectsSend;
        }
        destDatum->isPanEffectsSendSet = true;
    }

    if (srcDatum->isAttackVolEnvSet) {
        if (isAdded) {
            destDatum->attackVolEnv += srcDatum->attackVolEnv;
        } else {
            destDatum->attackVolEnv = srcDatum->attackVolEnv;
        }
        destDatum->isAttackVolEnvSet = true;
    }

    if (srcDatum->isHoldVolEnvSet) {
        if (isAdded) {
            destDatum->holdVolEnv += srcDatum->holdVolEnv;
        } else {
            destDatum->holdVolEnv = srcDatum->holdVolEnv;
        }
        destDatum->isHoldVolEnvSet = true;
    }

    if (srcDatum->isDecayVolEnvSet) {
        if (isAdded) {
            destDatum->decayVolEnv += srcDatum->decayVolEnv;
        } else {
            destDatum->decayVolEnv = srcDatum->decayVolEnv;
        }
        destDatum->isDecayVolEnvSet = true;
    }

    if (srcDatum->isSustainVolEnvSet) {
        if (isAdded) {
            destDatum->sustainVolEnv += srcDatum->sustainVolEnv;
        } else {
            destDatum->sustainVolEnv = srcDatum->sustainVolEnv;
        }
        destDatum->isSustainVolEnvSet = true;
    }

    if (srcDatum->isReleaseVolEnvSet) {
        if (isAdded) {
            destDatum->releaseVolEnv += srcDatum->releaseVolEnv;
        } else {
            destDatum->releaseVolEnv = srcDatum->releaseVolEnv;
        }
        destDatum->isReleaseVolEnvSet = true;
    }

    if (srcDatum->isKeyRangeSet) {
        destDatum->minPitch = srcDatum->minPitch;
        destDatum->maxPitch = srcDatum->maxPitch;
        destDatum->isKeyRangeSet = true;
    }

    if (srcDatum->isVelocityRangeSet) {
        destDatum->minVelocity = srcDatum->minVelocity;
        destDatum->maxVelocity = srcDatum->maxVelocity;
        destDatum->isVelocityRangeSet = true;
    }

    if (srcDatum->isRootKeySet) {
        destDatum->rootKey = srcDatum->rootKey;
        destDatum->isRootKeySet = true;
    }

    if (srcDatum->isCoarseTuneSet) {
        if (isAdded) {
            destDatum->coarseTune += srcDatum->coarseTune;
        } else {
            destDatum->coarseTune = srcDatum->coarseTune;
        }
        destDatum->isCoarseTuneSet = true;
    }

    if (srcDatum->isFineTuneSet) {
        if (isAdded) {
            destDatum->fineTune += srcDatum->fineTune;
        } else {
            destDatum->fineTune = srcDatum->fineTune;
        }
        destDatum->isFineTuneSet = true;
    }

    if (srcDatum->isSampleModesSet) {
        destDatum->sampleModes = srcDatum->sampleModes;
        destDatum->isSampleModesSet = true;
    }

    if (srcDatum->isStartLoopOffsetSet) {
        if (isAdded) {
            destDatum->startLoopOffset += srcDatum->startLoopOffset;
        } else {
            destDatum->startLoopOffset = srcDatum->startLoopOffset;
        }
        destDatum->isStartLoopOffsetSet = true;
    }

    if (srcDatum->isEndLoopOffsetSet) {
        if (isAdded) {
            destDatum->endLoopOffset += srcDatum->endLoopOffset;
        } else {
            destDatum->endLoopOffset = srcDatum->endLoopOffset;
        }
        destDatum->isEndLoopOffsetSet = true;
    }

    // Sample
    destDatum->origKeyAndCorr = srcDatum->origKeyAndCorr;
    destDatum->startLoop = srcDatum->startLoop;
    destDatum->endLoop = srcDatum->endLoop;
    destDatum->start = srcDatum->start;
    destDatum->end = srcDatum->end;
    destDatum->sampleRate = srcDatum->sampleRate;
}

// ---------------------------------------------------------------------------------------------------------------------
std::vector<SoundFont2Data> SoundFont2::dataForPreset(UInt16 bank, UInt16 preset) {
    std::vector<SoundFont2Data> ret;

    UInt16 presetNdx = 0;
    for (auto presetHeader : _presetHeaders) {
        if (presetHeader.bank == bank && presetHeader.preset == preset) {
            //
            // The preset has been found
            //

            auto minPresetBagNdx = presetHeader.presetBagNdx;
            auto maxPresetBagNdx = minPresetBagNdx;

            if (presetNdx < _presetHeaders.size() - 1) maxPresetBagNdx = _presetHeaders[presetNdx + 1].presetBagNdx;

            SoundFont2Data globalGlobalDatum;
            memset(&globalGlobalDatum, 0, sizeof(globalGlobalDatum));

            // For each preset bag
            SoundFont2PresetBag* globalPresetBagPtr = nullptr;
            for (auto presetBagNdx = minPresetBagNdx; presetBagNdx < maxPresetBagNdx; ++presetBagNdx) {
                auto presetBag = _presetBags.at(presetBagNdx);

                auto minGenListNdx = presetBag.genNdx;
                auto maxGenListNdx = _presetBags[presetBagNdx + 1].genNdx;

                if (maxGenListNdx > minGenListNdx) {
                    SoundFont2Data datum;
                    memset(&datum, 0, sizeof(datum));

                    bool isGlobalGlobalDatum = true;

                    // For each gen list
                    for (auto genListNdx = minGenListNdx; genListNdx < maxGenListNdx; ++genListNdx) {
                        auto genList = _genLists.at(genListNdx);

                        // auto modList = _modLists.at(presetBag.modNdx);

                        switch (genList.oper) {
                            case 41: {
                                // Instrument
                                isGlobalGlobalDatum = false;

                                auto instNdx = genList.amount.shAmount;
                                auto inst = _instruments.at(instNdx);

                                auto minInstBagNdx = inst.bagNdx;
                                auto maxInstBagNdx = _instruments.at(instNdx + 1).bagNdx;

                                SoundFont2Data datum2 = datum;

                                SoundFont2Data globalDatum;
                                memset(&globalDatum, 0, sizeof(globalDatum));

                                // For each inst bag
                                for (auto instBagNdx = minInstBagNdx; instBagNdx < maxInstBagNdx; ++instBagNdx) {
                                    SoundFont2Data datum3;
                                    memset(&datum3, 0, sizeof(datum3));

                                    auto instBag = _instBags.at(instBagNdx);

                                    auto minIGenNdx = instBag.genNdx;
                                    auto maxIGenNdx = _instBags.at(instBagNdx + 1).genNdx;

                                    bool isGlobal = true;
                                    for (auto iGenNdx = minIGenNdx; iGenNdx < maxIGenNdx; ++iGenNdx) {
                                        auto iGenList = _instGenLists.at(iGenNdx);
                                        processGenOperator(&datum3, iGenList.oper, iGenList.amount);
                                        // If this is a sample operator, we add
                                        if (iGenList.oper == 53) {
                                            // Must be the last
                                            assert(iGenNdx + 1 == maxIGenNdx);
                                            isGlobal = false;

                                            // Add the sample
                                            auto sampleNdx = iGenList.amount.shAmount;
                                            auto sample = _samples.at(sampleNdx);
                                            datum3.origKeyAndCorr =
                                                ((UInt16)sample.originalKey << 8) | sample.correction;
                                            datum3.startLoop = sample.startLoop;
                                            datum3.endLoop = sample.endLoop;
                                            datum3.start = sample.start;
                                            datum3.end = sample.end;
                                            datum3.sampleRate = sample.sampleRate;
                                        }
                                    }

                                    if (isGlobal) {
                                        globalDatum = datum3;
                                    } else {
                                        SoundFont2Data datum4;
                                        memset(&datum4, 0, sizeof(datum4));

                                        combineDatum(&datum4, &globalGlobalDatum, false);
                                        combineDatum(&datum4, &datum2, false);

                                        SoundFont2Data datum5;
                                        memset(&datum5, 0, sizeof(datum5));

                                        combineDatum(&datum5, &globalDatum, false);
                                        combineDatum(&datum5, &datum3, false);

                                        // Add missing defaults
                                        if (!datum5.isInitialFilterFcSet) {
                                            datum5.initialFilterFc = 13500;
                                            datum5.isInitialFilterFcSet = true;
                                        }
                                        if (!datum5.isInitialFilterQSet) {
                                            datum5.initialFilterQ = 0;
                                            datum5.isInitialFilterQSet = true;
                                        }
                                        if (!datum5.isAttackVolEnvSet) {
                                            datum5.attackVolEnv = -12000;
                                            datum5.isAttackVolEnvSet = true;
                                        }
                                        if (!datum5.isHoldVolEnvSet) {
                                            datum5.holdVolEnv = -12000;
                                            datum5.isHoldVolEnvSet = true;
                                        }
                                        if (!datum5.isDecayVolEnvSet) {
                                            datum5.decayVolEnv = -12000;
                                            datum5.isDecayVolEnvSet = true;
                                        }
                                        if (!datum5.isSustainVolEnvSet) {
                                            datum5.sustainVolEnv = 0;
                                            datum5.isSustainVolEnvSet = true;
                                        }
                                        if (!datum5.isReleaseVolEnvSet) {
                                            datum5.releaseVolEnv = -12000;
                                            datum5.isReleaseVolEnvSet = true;
                                        }

                                        combineDatum(&datum4, &datum5, true);

                                        // Add missing default ranges
                                        if (!datum4.isKeyRangeSet) {
                                            datum4.minPitch = 0;
                                            datum4.maxPitch = 127;
                                            datum4.isKeyRangeSet = true;
                                        }
                                        if (!datum4.isVelocityRangeSet) {
                                            datum4.minVelocity = 0;
                                            datum4.maxVelocity = 127;
                                            datum4.isVelocityRangeSet = true;
                                        }

                                        // Adjust the root key
                                        if (datum4.isRootKeySet) {
                                            datum4.origKeyAndCorr = datum4.rootKey << 8;
                                        }

                                        // Adjust the start loop offset
                                        if (datum4.isStartLoopOffsetSet) datum4.startLoop += datum4.startLoopOffset;

                                        // Adjust the end loop offset
                                        if (datum4.isEndLoopOffsetSet) datum4.endLoop += datum4.endLoopOffset;

                                        SInt16 minPitch = datum2.isKeyRangeSet ? datum2.minPitch : 0;
                                        SInt16 maxPitch = datum2.isKeyRangeSet ? datum2.maxPitch : 127;
                                        SInt16 minVelocity = datum2.isVelocityRangeSet ? datum2.minVelocity : 0;
                                        SInt16 maxVelocity = datum2.isVelocityRangeSet ? datum2.maxVelocity : 127;

                                        if (datum4.minPitch >= minPitch && datum4.maxPitch <= maxPitch &&
                                            datum4.minVelocity >= minVelocity && datum4.maxVelocity <= maxVelocity) {
                                            ret.push_back(datum4);
                                        }
                                    }

                                }  // for each inst bag
                            } break;
                            default:
                                processGenOperator(&datum, genList.oper, genList.amount);
                        }
                    }

                    if (isGlobalGlobalDatum) {
                        globalGlobalDatum = datum;
                        isGlobalGlobalDatum = false;
                    }

                } else {
                    // This is a global preset bag
                    globalPresetBagPtr = &_presetBags.at(presetBagNdx);
                }
            }  // for each preset bag

            break;
        }
        ++presetNdx;
    }

    return ret;
}
