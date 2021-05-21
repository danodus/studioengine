//
//  sequenceparser.h
//  MelobaseCore
//
//  Created by Daniel Cliche on 2021-01-20.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#ifndef SEQUENCEPARSER_H
#define SEQUENCEPARSER_H

#include <expat.h>

#include <map>
#include <memory>

#include "../melobasecore_sequence.h"

namespace MelobaseCore {

class SequenceParser {
    enum class ParserStates {
        Root,
        Sequence,
        SequenceDate,
        SequenceFolderID,
        SequenceName,
        SequenceRating,
        SequenceVersion,
        SequenceDataVersion,
        SequencePlayCount,
        SequenceTickPeriod,
        SequenceAnnotations,
        SequenceData
    };
    ParserStates _parserState;

    std::shared_ptr<Sequence> _newSequence;

    std::string _newSequenceEventsStr, _newSequenceDataStr, _newSequenceAnnotationsStr;

    double _newSequenceVersion = 0.0, _newSequenceDataVersion = 0.0;
    bool _nameSet = false, _sequenceEventsStrSet = false, _sequenceDataStrSet = false,
         _sequenceAnnotationsStrSet = false;

    bool _isNewSequenceFolderIDAvailable = false;
    UInt64 _newSequenceFolderID = 0;

    static void XMLCALL start(void* data, const char* el, const char** attr);
    static void XMLCALL chars(void* data, const char* el, int len);
    static void XMLCALL end(void* data, const char* el);

   public:
    bool parseSequence(std::shared_ptr<Sequence> sequence, std::string data, bool* isRemoteFolderIDAvailable, UInt64* folderID);
};

}  // namespace MelobaseCore

#endif  // SEQUENCEPARSER_H
