//
//  sequenceeditor.cpp
//  MelobaseCore
//
//  Created by Daniel Cliche on 2014-10-11.
//  Copyright (c) 2014-2021 Daniel Cliche. All rights reserved.
//

#include "sequenceeditor.h"

#include <assert.h>

#include <algorithm>
#include <iostream>

using namespace MelobaseCore;

// ---------------------------------------------------------------------------------------------------------------------
MelobaseCore::SequenceEditor::SequenceEditor(MDStudio::UndoManager* undoManager, MDStudio::Studio* studio,
                                             bool areEventsSortedAutomatically)
    : _undoManager(undoManager), _areEventsSortedAutomatically(areEventsSortedAutomatically) {
    _willModifySequenceFn = nullptr;
    _didModifySequenceFn = nullptr;
    _willRemoveTrackFn = nullptr;
    _sequence = nullptr;

    // Get presets
    if (studio) _presets = studio->presets();
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::sortEvents(std::shared_ptr<Track> track) {
    std::sort(track->clips[0]->events.begin(), track->clips[0]->events.end(),
              [](std::shared_ptr<Event> a, std::shared_ptr<Event> b) {
                  auto channelEventA = std::dynamic_pointer_cast<ChannelEvent>(a);
                  auto channelEventB = std::dynamic_pointer_cast<ChannelEvent>(b);

                  if (channelEventA->tickCount() == channelEventB->tickCount()) {
                      int priA = getEventPriority(channelEventA);
                      int priB = getEventPriority(channelEventB);

                      return priA < priB;
                  } else
                      return channelEventA->tickCount() < channelEventB->tickCount();
              });
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::addEvent(std::shared_ptr<Track> track, std::shared_ptr<Event> eventToAdd,
                                            bool isFirst, bool isLast) {
    // Notify the delegate that the sequence will be modified
    if (isFirst && _willModifySequenceFn) _willModifySequenceFn(this);

    auto events = &track->clips[0]->events;

    std::vector<std::shared_ptr<Event>>::iterator it;
    for (it = events->begin(); it != events->end(); ++it) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(*it);
        auto absChannelEventToAdd = std::dynamic_pointer_cast<ChannelEvent>(eventToAdd);
        if (channelEvent->tickCount() > absChannelEventToAdd->tickCount()) break;
    }

    it = events->insert(it, eventToAdd);

    std::shared_ptr<Event> e = *it;
    if (_undoManager) _undoManager->pushFn([=]() { removeEvent(track, e, isLast, isFirst); });

    // Notify the delegate that the sequence has been modified
    if (isLast && _didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::removeEvent(std::shared_ptr<Track> track, std::shared_ptr<Event> eventToRemove,
                                               bool isFirst, bool isLast) {
    // Notify the delegate that the sequence will be modified
    if (isFirst && _willModifySequenceFn) _willModifySequenceFn(this);

    auto events = &track->clips[0]->events;

    std::vector<std::shared_ptr<Event>>::iterator it;
    for (it = events->begin(); it != events->end(); ++it) {
        if (*it == eventToRemove) break;
    }

    if (it != events->end()) {
        std::shared_ptr<ChannelEvent> e = std::dynamic_pointer_cast<ChannelEvent>(*it);

        // We do not want to delete EOT events
        if (e->type() != CHANNEL_EVENT_TYPE_META_END_OF_TRACK) {
            if (_undoManager) _undoManager->pushFn([=]() { addEvent(track, e, isLast, isFirst); });

            events->erase(it);
        }
    }

    // Notify the delegate that the sequence has been modified
    if (isLast && _didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::updateEvent(std::shared_ptr<Track> track, std::shared_ptr<Event> eventToUpdate,
                                               UInt32 tickCount, UInt32 length, UInt8 channel, SInt32 param1,
                                               SInt32 param2, SInt32 param3, std::vector<UInt8> data) {
    auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(eventToUpdate);

    if (_willModifySequenceFn) _willModifySequenceFn(this);

    UInt32 oldTickCount = channelEvent->tickCount();
    UInt32 oldLength = channelEvent->length();
    UInt8 oldChannel = channelEvent->channel();
    SInt32 oldParam1 = channelEvent->param1();
    SInt32 oldParam2 = channelEvent->param2();
    SInt32 oldParam3 = channelEvent->param3();
    std::vector<UInt8> oldData = channelEvent->data();

    if (_undoManager)
        _undoManager->pushFn([=]() {
            updateEvent(track, eventToUpdate, oldTickCount, oldLength, oldChannel, oldParam1, oldParam2, oldParam3,
                        oldData);
        });

    channelEvent->setTickCount(tickCount);
    channelEvent->setLength(length);
    channelEvent->setChannel(channel);
    channelEvent->setParam1(param1);
    channelEvent->setParam2(param2);
    channelEvent->setParam3(param3);
    channelEvent->setData(data);

    // If the tick count has changed, we need to sort the events
    if (_areEventsSortedAutomatically && (tickCount != oldTickCount)) sortEvents(track);

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequenceEditor::canMoveEvents(std::shared_ptr<Track> track,
                                                 std::vector<std::shared_ptr<Event>> events, SInt32 relativeTick,
                                                 SInt32 relativePitch, SInt32 relativeValue) {
    bool isOverlapped = false;

    for (std::shared_ptr<Event> event : events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);

        if ((relativeTick < 0) && (-relativeTick > channelEvent->tickCount())) {
            isOverlapped = true;
        }

        if (channelEvent->type() == CHANNEL_EVENT_TYPE_NOTE) {
            // We do not allow events with pitch beyond the valid range
            if (channelEvent->param1() + relativePitch < 12) isOverlapped = true;

            if (channelEvent->param1() + relativePitch >= 108) isOverlapped = true;

        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_SUSTAIN) {
            if (channelEvent->param1() + relativeValue > 127) {
                isOverlapped = true;
            } else if (channelEvent->param1() + relativeValue < 0) {
                isOverlapped = true;
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_META_SET_TEMPO) {
            // The relative value is in BPM

            float bpm = 60000000.0f / (float)(channelEvent->param1());
            bpm += relativeValue;

            if (bpm > 300.0f) {
                isOverlapped = true;
            } else if (bpm < 30.0f) {
                isOverlapped = true;
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_PITCH_BEND) {
            if (channelEvent->param1() + relativeValue < 0) {
                isOverlapped = true;
            } else if (channelEvent->param1() + relativeValue > 16383) {
                isOverlapped = true;
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_MODULATION) {
            if (channelEvent->param1() + relativeValue < 0) {
                isOverlapped = true;
            } else if (channelEvent->param1() + relativeValue > 127) {
                isOverlapped = true;
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_MIXER_LEVEL_CHANGE) {
            if (channelEvent->param2() == 0) {
                if (channelEvent->param1() + relativeValue < 0) {
                    isOverlapped = true;
                } else if (channelEvent->param1() + relativeValue > 127) {
                    isOverlapped = true;
                }
            } else {
                if (channelEvent->param1() + relativeValue < 0) {
                    isOverlapped = true;
                } else if (channelEvent->param1() + relativeValue > 100) {
                    isOverlapped = true;
                }
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_MIXER_BALANCE_CHANGE) {
            if (channelEvent->param2() == 0) {
                if (channelEvent->param1() + relativeValue < 0) {
                    isOverlapped = true;
                } else if (channelEvent->param1() + relativeValue > 127) {
                    isOverlapped = true;
                }
            } else {
                if (channelEvent->param1() + relativeValue < -100) {
                    isOverlapped = true;
                } else if (channelEvent->param1() + relativeValue > 100) {
                    isOverlapped = true;
                }
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_CONTROL_CHANGE) {
            if (channelEvent->param2() + relativeValue > 127) {
                isOverlapped = true;
            } else if (channelEvent->param2() + relativeValue < 0) {
                isOverlapped = true;
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_KEY_AFTERTOUCH) {
            if (channelEvent->param2() + relativeValue > 127) {
                isOverlapped = true;
            } else if (channelEvent->param2() + relativeValue < 0) {
                isOverlapped = true;
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_CHANNEL_AFTERTOUCH) {
            if (channelEvent->param1() + relativeValue > 127) {
                isOverlapped = true;
            } else if (channelEvent->param1() + relativeValue < 0) {
                isOverlapped = true;
            }
        }
    }

    return !isOverlapped;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequenceEditor::canAddEvents(std::shared_ptr<Track> track,
                                                std::vector<std::shared_ptr<Event>> events) {
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
SInt32 MelobaseCore::SequenceEditor::moveEvents(std::shared_ptr<Track> track,
                                                std::vector<std::shared_ptr<Event>> events, SInt32 relativeTick,
                                                SInt32 relativePitch, SInt32 relativeValue, bool isFirst, bool isLast) {
    // Notify the delegate that the sequence will be modified
    if (isFirst && _willModifySequenceFn) _willModifySequenceFn(this);

    for (auto event : events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);

        if (relativeTick < 0 && -relativeTick > channelEvent->tickCount()) {
            relativeTick = -channelEvent->tickCount();
        }

        channelEvent->setTickCount(channelEvent->tickCount() + relativeTick);
        if (channelEvent->type() == CHANNEL_EVENT_TYPE_NOTE) {
            channelEvent->setParam1(channelEvent->param1() + relativePitch);
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_SUSTAIN) {
            if (channelEvent->param1() + relativeValue >= 127) {
                channelEvent->setParam1(127);
            } else if (channelEvent->param1() + relativeValue < 0) {
                channelEvent->setParam1(0);
            } else {
                channelEvent->setParam1(channelEvent->param1() + relativeValue);
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_META_SET_TEMPO) {
            // The relative value is in BPM

            float bpm = 60000000.0f / (float)(channelEvent->param1());
            bpm += relativeValue;

            if (bpm > 300.0f) {
                bpm = 300.0f;
            } else if (bpm < 30.0f) {
                bpm = 30.0f;
            }

            channelEvent->setParam1(60000000.0f / bpm);
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_PITCH_BEND) {
            channelEvent->setParam1(channelEvent->param1() + relativeValue);
            if (channelEvent->param1() < 0) {
                channelEvent->setParam1(0);
            } else if (channelEvent->param1() >= 16383) {
                channelEvent->setParam1(16383);
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_MODULATION) {
            channelEvent->setParam1(channelEvent->param1() + relativeValue);
            if (channelEvent->param1() < 0) {
                channelEvent->setParam1(0);
            } else if (channelEvent->param1() >= 127) {
                channelEvent->setParam1(127);
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_MIXER_LEVEL_CHANGE) {
            channelEvent->setParam1(channelEvent->param1() + relativeValue);
            if (channelEvent->param2() == 0) {
                if (channelEvent->param1() < 0) {
                    channelEvent->setParam1(0);
                } else if (channelEvent->param1() >= 127) {
                    channelEvent->setParam1(127);
                }
            } else {
                if (channelEvent->param1() < 0) {
                    channelEvent->setParam1(0);
                } else if (channelEvent->param1() >= 100) {
                    channelEvent->setParam1(100);
                }
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_MIXER_BALANCE_CHANGE) {
            channelEvent->setParam1(channelEvent->param1() + relativeValue);
            if (channelEvent->param2() == 0) {
                if (channelEvent->param1() < 0) {
                    channelEvent->setParam1(0);
                } else if (channelEvent->param1() >= 127) {
                    channelEvent->setParam1(127);
                }
            } else {
                if (channelEvent->param1() < -100) {
                    channelEvent->setParam1(-100);
                } else if (channelEvent->param1() >= 100) {
                    channelEvent->setParam1(100);
                }
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_CONTROL_CHANGE) {
            channelEvent->setParam2(channelEvent->param2() + relativeValue);
            if (channelEvent->param2() < 0) {
                channelEvent->setParam2(0);
            } else if (channelEvent->param2() >= 127) {
                channelEvent->setParam2(127);
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_KEY_AFTERTOUCH) {
            channelEvent->setParam2(channelEvent->param2() + relativeValue);
            if (channelEvent->param2() < 0) {
                channelEvent->setParam2(0);
            } else if (channelEvent->param2() >= 127) {
                channelEvent->setParam2(127);
            }
        } else if (channelEvent->type() == CHANNEL_EVENT_TYPE_CHANNEL_AFTERTOUCH) {
            channelEvent->setParam1(channelEvent->param1() + relativeValue);
            if (channelEvent->param1() < 0) {
                channelEvent->setParam1(0);
            } else if (channelEvent->param1() >= 127) {
                channelEvent->setParam1(127);
            }
        }
    }

    // If last operation, we sort the events
    if (_areEventsSortedAutomatically && isLast) sortEvents(track);

    if (_undoManager)
        _undoManager->pushFn(
            [=]() { moveEvents(track, events, -relativeTick, -relativePitch, -relativeValue, isLast, isFirst); });

    // Notify the delegate that the sequence has been modified
    if (isLast && _didModifySequenceFn) _didModifySequenceFn(this);

    return relativeTick;
}

// ---------------------------------------------------------------------------------------------------------------------
bool MelobaseCore::SequenceEditor::canResizeEvents(std::shared_ptr<Track> track,
                                                   std::vector<std::shared_ptr<Event>> events, SInt32 relativeLength) {
    for (auto event : events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        if ((SInt32)channelEvent->length() + relativeLength < 24) return false;
    }
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::resizeEvents(std::shared_ptr<Track> track,
                                                std::vector<std::shared_ptr<Event>> events, SInt32 relativeLength) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    for (auto event : events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        channelEvent->setLength(channelEvent->length() + relativeLength);
    }

    if (_undoManager) _undoManager->pushFn([=]() { resizeEvents(track, events, -relativeLength); });

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::quantizeEvents(std::shared_ptr<Track> track,
                                                  std::vector<std::shared_ptr<Event>> events, UInt32 resTickCount) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    // Find the ticks of all time signature events
    std::vector<UInt32> timeSignatureTicks;

    for (auto event : _sequence->data.tracks[0]->clips[0]->events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        if (channelEvent->type() == EVENT_TYPE_META_TIME_SIGNATURE)
            timeSignatureTicks.push_back(channelEvent->tickCount());
    }

    std::vector<std::pair<std::vector<std::shared_ptr<Event>>, SInt32>> quantizeOperations;
    // Create a list of operations
    for (std::shared_ptr<Event> event : events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);

        // If this is an event that can be quantized
        if (channelEvent->type() == CHANNEL_EVENT_TYPE_NOTE || channelEvent->type() == CHANNEL_EVENT_TYPE_SUSTAIN ||
            channelEvent->type() == CHANNEL_EVENT_TYPE_PROGRAM_CHANGE ||
            channelEvent->type() == CHANNEL_EVENT_TYPE_PITCH_BEND ||
            channelEvent->type() == CHANNEL_EVENT_TYPE_MODULATION ||
            channelEvent->type() == CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE ||
            channelEvent->type() == CHANNEL_EVENT_TYPE_META_SET_TEMPO ||
            channelEvent->type() == CHANNEL_EVENT_TYPE_META_END_OF_TRACK ||
            channelEvent->type() == CHANNEL_EVENT_TYPE_META_GENERIC ||
            channelEvent->type() == CHANNEL_EVENT_TYPE_MIXER_LEVEL_CHANGE ||
            channelEvent->type() == CHANNEL_EVENT_TYPE_MIXER_BALANCE_CHANGE ||
            channelEvent->type() == CHANNEL_EVENT_TYPE_KEY_AFTERTOUCH ||
            channelEvent->type() == CHANNEL_EVENT_TYPE_CHANNEL_AFTERTOUCH ||
            channelEvent->type() == CHANNEL_EVENT_TYPE_SYSTEM_EXCLUSIVE ||
            channelEvent->type() == CHANNEL_EVENT_TYPE_CONTROL_CHANGE) {
            // Find the time signature tick for the event
            bool isTimeSignatureFound = false;
            UInt32 timeSignatureTick = 0;
            auto rit = timeSignatureTicks.rbegin();
            for (; rit != timeSignatureTicks.rend(); ++rit) {
                timeSignatureTick = *rit;
                if (channelEvent->tickCount() >= *rit) {
                    isTimeSignatureFound = true;
                    ++rit;
                    break;
                }
            }

            // Special case for the time signature event where we want to use the previous time signature tick in order
            // to allow quantization
            if (channelEvent->type() == EVENT_TYPE_META_TIME_SIGNATURE) {
                if (rit != timeSignatureTicks.rend()) {
                    timeSignatureTick = *rit;
                } else {
                    isTimeSignatureFound = false;
                }
            }

            if (isTimeSignatureFound) {
                // Perform the quantization relative to the time signature tick
                UInt32 tickCount = channelEvent->tickCount() - timeSignatureTick;
                UInt32 quantizedTickCount =
                    timeSignatureTick + ((tickCount + resTickCount / 2) / resTickCount) * resTickCount;

                std::vector<std::shared_ptr<Event>> eventsToMove;
                eventsToMove.push_back(event);

                quantizeOperations.push_back(
                    std::make_pair(eventsToMove, quantizedTickCount - channelEvent->tickCount()));
            }
        }  // if this is a note one event
    }      // for each event

    // Perform the work
    for (auto it = quantizeOperations.begin(); it != quantizeOperations.end(); ++it) {
        auto quantizeOperation = *it;

        moveEvents(track, quantizeOperation.first, quantizeOperation.second, 0, 0, it == quantizeOperations.begin(),
                   it == quantizeOperations.end() - 1);
    }

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::setChannelOfEvents(std::shared_ptr<Track> track,
                                                      std::vector<std::shared_ptr<Event>> events, UInt8 channel) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    for (auto event : events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        std::vector<std::shared_ptr<Event>> e;
        e.push_back(event);
        UInt8 c = channelEvent->channel();
        if (_undoManager) _undoManager->pushFn([=]() { setChannelOfEvents(track, e, c); });

        channelEvent->setChannel(channel);
    }

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::setVelocityOfEvents(std::shared_ptr<Track> track,
                                                       std::vector<std::shared_ptr<Event>> events, SInt32 velocity) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    for (auto event : events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        if (channelEvent->type() == CHANNEL_EVENT_TYPE_NOTE) {
            std::vector<std::shared_ptr<Event>> e;
            e.push_back(event);
            SInt32 c = channelEvent->param2();
            if (_undoManager) _undoManager->pushFn([=]() { setVelocityOfEvents(track, e, c); });

            channelEvent->setParam2(velocity);
        }
    }

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::setProgramOfEvents(std::shared_ptr<Track> track,
                                                      std::vector<std::shared_ptr<Event>> events, SInt32 program) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    for (auto event : events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        if (channelEvent->type() == CHANNEL_EVENT_TYPE_PROGRAM_CHANGE) {
            std::vector<std::shared_ptr<Event>> e;
            e.push_back(event);
            SInt32 c = channelEvent->param1();
            if (_undoManager) _undoManager->pushFn([=]() { setProgramOfEvents(track, e, c); });

            // If this is the channel is 10, we force the drum kit
            if (channelEvent->channel() == 9) program = STUDIO_INSTRUMENT_GM_STANDARD_DRUM_KIT;

            channelEvent->setParam1(program);
        }
    }

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::setTimeSignatureOfEvents(std::shared_ptr<Track> track,
                                                            std::vector<std::shared_ptr<Event>> events,
                                                            SInt32 numerator, SInt32 denominator) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    for (auto event : events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        if (channelEvent->type() == CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE) {
            std::vector<std::shared_ptr<Event>> e;
            e.push_back(event);
            SInt32 num = channelEvent->param1();
            SInt32 denum = channelEvent->param2();
            if (_undoManager) _undoManager->pushFn([=]() { setTimeSignatureOfEvents(track, e, num, denum); });

            channelEvent->setParam1(numerator);
            channelEvent->setParam2(denominator);
        }
    }

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::setSysexDataOfEvents(std::shared_ptr<Track> track,
                                                        std::vector<std::shared_ptr<Event>> events,
                                                        std::vector<UInt8> data) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    for (auto event : events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        if (channelEvent->type() == CHANNEL_EVENT_TYPE_SYSTEM_EXCLUSIVE) {
            std::vector<std::shared_ptr<Event>> e;
            e.push_back(event);
            auto d = channelEvent->data();
            if (_undoManager) _undoManager->pushFn([=]() { setSysexDataOfEvents(track, e, d); });

            channelEvent->setData(data);
        }
    }

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::setMetaDataOfEvents(std::shared_ptr<Track> track,
                                                       std::vector<std::shared_ptr<Event>> events,
                                                       std::vector<UInt8> data) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    for (auto event : events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        if (channelEvent->type() == CHANNEL_EVENT_TYPE_META_GENERIC) {
            std::vector<std::shared_ptr<Event>> e;
            e.push_back(event);
            auto d = channelEvent->data();
            if (_undoManager) _undoManager->pushFn([=]() { setMetaDataOfEvents(track, e, d); });

            channelEvent->setData(data);
        }
    }

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::setPitchOfEvents(std::shared_ptr<Track> track,
                                                    std::vector<std::shared_ptr<Event>> events, SInt32 pitch) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    for (auto event : events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        if (channelEvent->type() == CHANNEL_EVENT_TYPE_KEY_AFTERTOUCH) {
            std::vector<std::shared_ptr<Event>> e;
            e.push_back(event);
            auto p = channelEvent->param1();
            if (_undoManager) _undoManager->pushFn([=]() { setPitchOfEvents(track, e, p); });

            channelEvent->setParam1(pitch);
        }
    }

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::addTrack(std::shared_ptr<Track> trackToAdd, int trackIndex) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    if (trackIndex < 0) trackIndex = (int)_sequence->data.tracks.size();

    if (_undoManager) _undoManager->pushFn([=]() { removeTrack(trackToAdd); });

    _sequence->data.tracks.insert(_sequence->data.tracks.begin() + trackIndex, trackToAdd);

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::removeTrack(std::shared_ptr<Track> trackToRemove) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    // Notify the delegate that a track will be removed
    if (_willRemoveTrackFn) _willRemoveTrackFn(this, trackToRemove);

    // Find the index of the track
    size_t trackIndex = std::find(_sequence->data.tracks.begin(), _sequence->data.tracks.end(), trackToRemove) -
                        _sequence->data.tracks.begin();

    if (_undoManager) _undoManager->pushFn([=]() { addTrack(trackToRemove, (int)trackIndex); });

    _sequence->data.tracks.erase(_sequence->data.tracks.begin() + trackIndex);

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::setTrackName(std::shared_ptr<Track> track, std::string name) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    std::string oldName = track->name;
    if (_undoManager) _undoManager->pushFn([=]() { setTrackName(track, oldName); });

    track->name = name;

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::setTrackChannel(std::shared_ptr<Track> track, UInt8 channel) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    UInt8 oldChannel = track->channel;
    if (_undoManager) _undoManager->pushFn([=]() { setTrackChannel(track, oldChannel); });

    track->channel = channel;

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::setTrackEvents(std::shared_ptr<Track> track,
                                                  std::vector<std::shared_ptr<Event>> events) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    std::vector<std::shared_ptr<Event>> oldEvents = track->clips[0]->events;
    if (_undoManager) _undoManager->pushFn([=]() { setTrackEvents(track, oldEvents); });

    track->clips[0]->events = events;

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::setDataFormat(UInt8 dataFormat) {
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    UInt8 oldDataFormat = _sequence->data.format;
    if (_undoManager) _undoManager->pushFn([=]() { setDataFormat(oldDataFormat); });

    _sequence->data.format = dataFormat;

    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::convertToMultiTrack() {
    // Sanity check
    assert(_sequence->data.format == SEQUENCE_DATA_FORMAT_SINGLE_TRACK);

    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    if (_undoManager) _undoManager->beginGroup();

    setDataFormat(SEQUENCE_DATA_FORMAT_SINGLE_TRACK);

    _sequence->data.format = SEQUENCE_DATA_FORMAT_MULTI_TRACK;

    std::shared_ptr<Track> tracks[STUDIO_MAX_CHANNELS];

    std::vector<std::shared_ptr<Event>> eventsToRemove;

    for (auto event : _sequence->data.tracks[0]->clips[0]->events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        if ((channelEvent->type() != CHANNEL_EVENT_TYPE_META_END_OF_TRACK) &&
            (channelEvent->type() != CHANNEL_EVENT_TYPE_META_TIME_SIGNATURE) &&
            (channelEvent->type() != CHANNEL_EVENT_TYPE_META_SET_TEMPO) &&
            (channelEvent->type() != CHANNEL_EVENT_TYPE_META_GENERIC)) {
            // Move the event to the appropriate track
            if (!tracks[channelEvent->channel()]) {
                tracks[channelEvent->channel()] = std::make_shared<Track>();
                tracks[channelEvent->channel()]->channel = channelEvent->channel();
            }
            tracks[channelEvent->channel()]->clips[0]->events.push_back(std::make_shared<ChannelEvent>(
                channelEvent->type(), channelEvent->channel(), channelEvent->tickCount(), channelEvent->length(),
                channelEvent->param1(), channelEvent->param2(), channelEvent->param3(), channelEvent->data()));
            eventsToRemove.push_back(channelEvent);

            if ((channelEvent->type() == CHANNEL_EVENT_TYPE_PROGRAM_CHANGE) &&
                (tracks[channelEvent->channel()]->name.empty())) {
                // Find the preset
                for (auto preset : _presets) {
                    if ((preset._bank == STUDIO_PRESET_BANK_FROM_INSTRUMENT(channelEvent->param1())) &&
                        (preset._number == STUDIO_PRESET_NUMBER_FROM_INSTRUMENT(channelEvent->param1()))) {
                        tracks[channelEvent->channel()]->name = preset._name;
                        break;
                    }
                }
            }
        }
    }

    // Remove the events from first track
    for (auto it = eventsToRemove.begin(); it != eventsToRemove.end(); ++it) {
        removeEvent(_sequence->data.tracks[0], *it, it == eventsToRemove.begin(), it == eventsToRemove.end() - 1);
    }

    for (auto track : tracks) {
        if (track) {
            // Find the maximum tick count for the track
            UInt32 maxTickCount = 0;
            for (auto event : track->clips[0]->events) {
                auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
                if (channelEvent->tickCount() + channelEvent->length() > maxTickCount)
                    maxTickCount = channelEvent->tickCount() + channelEvent->length();
            }

            // Add an end of track event
            addEvent(track,
                     std::make_shared<MelobaseCore::ChannelEvent>(CHANNEL_EVENT_TYPE_META_END_OF_TRACK, 0, maxTickCount,
                                                                  0, -1, -1),
                     true, true);

            // Add the track
            addTrack(track);
        }
    }

    if (_undoManager) _undoManager->endGroup();

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::internalMergeTracks(std::shared_ptr<Track> destinationTrack,
                                                       std::vector<std::shared_ptr<Track>> tracks) {
    UInt32 maxEOTTickCount = 0;

    // Add the events of the tracks to the destination track
    for (auto track : tracks) {
        for (auto itEvent = track->clips[0]->events.begin(); itEvent != track->clips[0]->events.end(); ++itEvent) {
            auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(*itEvent);
            if (channelEvent->type() != CHANNEL_EVENT_TYPE_META_END_OF_TRACK) {
                addEvent(destinationTrack,
                         std::make_shared<ChannelEvent>(channelEvent->type(), channelEvent->channel(),
                                                        channelEvent->tickCount(), channelEvent->length(),
                                                        channelEvent->param1(), channelEvent->param2(),
                                                        channelEvent->param3(), channelEvent->data()),
                         itEvent == track->clips[0]->events.begin(), itEvent == track->clips[0]->events.end() - 1);
            } else {
                // This is an EOT event
                if (channelEvent->tickCount() > maxEOTTickCount) maxEOTTickCount = channelEvent->tickCount();
            }
        }
    }

    // Remove the tracks
    for (auto track : tracks) removeTrack(track);

    // Move the end of track to the end
    for (auto event : destinationTrack->clips[0]->events) {
        auto channelEvent = std::dynamic_pointer_cast<ChannelEvent>(event);
        if (channelEvent->type() == CHANNEL_EVENT_TYPE_META_END_OF_TRACK) {
            if (channelEvent->tickCount() < maxEOTTickCount) {
                // Move the EOT event
                std::vector<std::shared_ptr<Event>> eventsToMove;
                eventsToMove.push_back(event);
                moveEvents(destinationTrack, eventsToMove, maxEOTTickCount - channelEvent->tickCount(), 0, 0, true,
                           true);
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::convertToSingleTrack() {
    // Sanity check
    assert(_sequence->data.format == SEQUENCE_DATA_FORMAT_MULTI_TRACK);

    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    if (_undoManager) _undoManager->beginGroup();

    setDataFormat(SEQUENCE_DATA_FORMAT_MULTI_TRACK);

    _sequence->data.format = SEQUENCE_DATA_FORMAT_SINGLE_TRACK;

    std::vector<std::shared_ptr<Track>> tracks;
    for (size_t trackIndex = 1; trackIndex < _sequence->data.tracks.size(); ++trackIndex)
        tracks.push_back(_sequence->data.tracks[trackIndex]);

    internalMergeTracks(_sequence->data.tracks[0], tracks);

    if (_undoManager) _undoManager->endGroup();

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::mergeTracks(std::shared_ptr<Track> destinationTrack,
                                               std::vector<std::shared_ptr<Track>> tracks) {
    // Notify the delegate that the sequence will be modified
    if (_willModifySequenceFn) _willModifySequenceFn(this);

    if (_undoManager) _undoManager->beginGroup();

    // Before the merge, check if we will have to set the destination track as MULTI
    bool isDestinationToMulti = false;
    if (destinationTrack->channel != SEQUENCE_TRACK_MULTI_CHANNEL) {
        for (auto track : tracks)
            if (track->channel != destinationTrack->channel) {
                isDestinationToMulti = true;
                break;
            }
    }

    // Merge the tracks to destination track
    internalMergeTracks(destinationTrack, tracks);

    // Set the new track name
    auto newName = destinationTrack->name;
    for (auto track : tracks) newName += " " + track->name;
    setTrackName(destinationTrack, newName);

    // Set the destination track to multi if neccessary
    if (isDestinationToMulti) setTrackChannel(destinationTrack, SEQUENCE_TRACK_MULTI_CHANNEL);

    if (_undoManager) _undoManager->endGroup();

    // Notify the delegate that the sequence has been modified
    if (_didModifySequenceFn) _didModifySequenceFn(this);
}

// ---------------------------------------------------------------------------------------------------------------------
void MelobaseCore::SequenceEditor::setSequence(std::shared_ptr<Sequence> sequence) { _sequence = sequence; }
