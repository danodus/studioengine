//
//  sequenceutils.h
//  MelobaseCoreTests
//
//  Created by Daniel Cliche on 2021-02-04.
//  Copyright (c) 2021 Daniel Cliche. All rights reserved.
//

#include <sequencesdb.h>

#pragma once

void setEvents(MelobaseCore::Sequence* sequence, size_t nbEvents);
void setAnnotations(MelobaseCore::Sequence* sequence, size_t nbAnnotations);

bool compareSequences(MelobaseCore::Sequence* s1, MelobaseCore::Sequence* s2, bool mustHaveEvents);

bool compareDatabases(MelobaseCore::SequencesDB& sequencesDB1, MelobaseCore::SequencesDB& sequencesDB2,
                      bool mustHaveEvents);
