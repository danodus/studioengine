--
-- PianoRollMainView.lua
--

require "Common"

if language() == "fr" then

currentVelocityStr = "Vél.:"

controllerProgramChangesStr = "PC"
controllerPitchBendStr = "PB"
controllerKeyAftertouchStr = "KAT"
controllerChannelAftertouchStr = "CAT"
controllerSysexStr = "SYSEX"
controllerControlChangeStr = "CC"
controllerMetaTypeStr = "META"

controllerProgramChangesTooltipStr = "Program Change"
controllerPitchBendTooltipStr = "Pitch Bend"
controllerKeyAftertouchTooltipStr = "Key Aftertouch"
controllerChannelAftertouchTooltipStr = "Channel Aftertouch"
controllerSysexTooltipStr = "System Exclusive"
controllerControlChangeTooltipStr = "Control Change (Sustain, Modulation, etc.)"
controllerMetaTypeTooltipStr = "Meta (Tempo, Time Signature, etc.)"

else

currentVelocityStr = "Vel.:"

controllerProgramChangesStr = "PC"
controllerTempoStr = "T"
controllerPitchBendStr = "PB"
controllerKeyAftertouchStr = "KAT"
controllerChannelAftertouchStr = "CAT"
controllerSysexStr = "SYSEX"
controllerControlChangeStr = "CC"
controllerMetaTypeStr = "META"

controllerProgramChangesTooltipStr = "Program Change"
controllerTempoTooltipStr = "Tempo"
controllerPitchBendTooltipStr = "Pitch Bend"
controllerKeyAftertouchTooltipStr = "Key Aftertouch"
controllerChannelAftertouchTooltipStr = "Channel Aftertouch"
controllerSysexTooltipStr = "System Exclusive"
controllerControlChangeTooltipStr = "Control Change (Sustain, Modulation, etc.)"
controllerMetaTypeTooltipStr = "Meta (Tempo, Time Signature, etc.)"

end
